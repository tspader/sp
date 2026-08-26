#include "../http.h"

typedef struct {
  sp_http_error_t err;
  s32             status;
  header_t        headers [HTTP_TEST_MAX_HEADERS];
} expect_t;

typedef struct {
  const c8* name;
  const c8* head;
  u32       pad_headers;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "status_only",
    .head = "HTTP/1.1 204 No Content",
    .expect = { .status = 204 },
  },
  {
    .name = "status_without_reason",
    .head = "HTTP/1.1 200",
    .expect = { .status = 200 },
  },
  {
    .name = "http_1_0_response",
    .head = "HTTP/1.0 200 OK\r\nX: y",
    .expect = { .status = 200, .headers = { { "X", "y" } } },
  },
  {
    .name = "headers_kept_in_order",
    .head = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nX-Custom: abc\r\nServer: A",
    .expect = {
      .status = 200,
      .headers = { { "Content-Type", "text/html" }, { "X-Custom", "abc" }, { "Server", "A" } },
    },
  },
  {
    .name = "duplicate_names_kept",
    .head = "HTTP/1.1 200 OK\r\nX: 1\r\nX: 2",
    .expect = { .status = 200, .headers = { { "X", "1" }, { "X", "2" } } },
  },
  {
    .name = "value_trimmed",
    .head = "HTTP/1.1 200 OK\r\nX:   spaced   ",
    .expect = { .status = 200, .headers = { { "X", "spaced" } } },
  },
  {
    .name = "empty_value",
    .head = "HTTP/1.1 200 OK\r\nX:",
    .expect = { .status = 200, .headers = { { "X", "" } } },
  },
  {
    .name = "content_type_params",
    .head = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8",
    .expect = { .status = 200, .headers = { { "Content-Type", "application/json; charset=utf-8" } } },
  },
  {
    .name = "many_headers_ok",
    .head = "HTTP/1.1 200 OK",
    .pad_headers = 100,
    .expect = { .status = 200 },
  },
  {
    .name = "header_without_colon",
    .head = "HTTP/1.1 200 OK\r\ngarbage",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "header_name_with_space",
    .head = "HTTP/1.1 200 OK\r\nBad Name: x",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "header_name_empty",
    .head = "HTTP/1.1 200 OK\r\n: x",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "header_value_with_bare_cr",
    .head = "HTTP/1.1 200 OK\r\nX: a\rb",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "not_http",
    .head = "ICY 200 OK",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "no_status",
    .head = "HTTP/1.1",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "status_not_numeric",
    .head = "HTTP/1.1 abc OK",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "status_too_large",
    .head = "HTTP/1.1 9999 Huh",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "status_too_small",
    .head = "HTTP/1.1 42 Answer",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "empty",
    .head = "",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t head = sp_cstr_as_str(c->head);
  if (c->pad_headers) {
    sp_io_dyn_mem_writer_t b = sp_zero;
    sp_io_dyn_mem_writer_init(mem, &b);
    sp_io_write_str(&b.base, head, SP_NULLPTR);
    sp_for(it, c->pad_headers) {
      sp_fmt_io(&b.base, "\r\nP{}: v", sp_fmt_uint(it));
    }
    head = sp_io_dyn_mem_writer_as_str(&b);
  }

  sp_http_response_head_t parsed = sp_zero;
  sp_http_error_t err = sp_http_response_head_parse(head, &parsed);
  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  sp_expect_eq(t, parsed.status, c->expect.status);
  if (c->pad_headers) {
    u32 count = 0;
    sp_http_headers_it_t it = sp_http_headers_it(parsed.headers);
    sp_http_header_t header = sp_zero;
    while (sp_http_headers_it_next(&it, &header)) count++;
    sp_expect_eq(t, count, c->pad_headers);
    return SP_OK;
  }

  return expect_headers(t, parsed.headers, c->expect.headers, HTTP_TEST_MAX_HEADERS);
}

sp_test_each_fn(http, head, test_t, tests, run);
