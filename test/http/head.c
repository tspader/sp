#include "http.h"

typedef struct {
  sp_http_error_t err;
  s32            status;
  const c8*      location;
  const c8*      content_type;
  bool           chunked;
  bool           has_length;
  u64            length;
} expect_t;

typedef struct {
  const c8* name;
  const c8* head;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "content_length",
    .head = "HTTP/1.1 200 OK\r\nContent-Length: 12",
    .expect = { .status = 200, .has_length = true, .length = 12 },
  },
  {
    .name = "duplicate_equal_lengths",
    .head = "HTTP/1.1 200 OK\r\ncontent-length: 12\r\ncontent-length: 12",
    .expect = { .status = 200, .has_length = true, .length = 12 },
  },
  {
    .name = "location",
    .head = "HTTP/1.1 301 Moved Permanently\r\nLocation: https://example.com/new",
    .expect = { .status = 301, .location = "https://example.com/new" },
  },
  {
    .name = "chunked",
    .head = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked",
    .expect = { .status = 200, .chunked = true },
  },
  {
    .name = "te_list_chunked",
    .head = "HTTP/1.1 200 OK\r\nTransfer-Encoding: GZIP, Chunked",
    .expect = { .status = 200, .chunked = true },
  },
  {
    .name = "content_type_params",
    .head = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: 2",
    .expect = { .status = 200, .content_type = "application/json; charset=utf-8", .has_length = true, .length = 2 },
  },
  {
    .name = "no_headers_204",
    .head = "HTTP/1.1 204 No Content",
    .expect = { .status = 204 },
  },
  {
    .name = "conflicting_lengths",
    .head = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Length: 9",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "length_not_numeric",
    .head = "HTTP/1.1 200 OK\r\nContent-Length: abc",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "te_not_chunked",
    .head = "HTTP/1.1 200 OK\r\nTransfer-Encoding: gzip",
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
    .name = "empty",
    .head = "",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_head_t head = sp_zero;
  sp_http_error_t err = sp_http_parse_head(sp_cstr_as_str(c->head), &head);
  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  sp_expect_eq(t, head.status, c->expect.status);
  sp_expect_eq(t, head.chunked, c->expect.chunked);
  sp_expect_eq(t, head.has_length, c->expect.has_length);
  sp_expect_eq(t, head.length, c->expect.length);
  sp_expect_str_eq(t, head.location, sp_cstr_as_str(c->expect.location));
  sp_expect_str_eq(t, head.content_type, sp_cstr_as_str(c->expect.content_type));
  return SP_OK;
}

sp_test_each_fn(http, head, test_t, tests, run);
