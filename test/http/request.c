#include "http.h"

typedef struct {
  sp_http_error_t err;
  const c8*       method;
  const c8*       target;
  header_t        headers [HTTP_TEST_MAX_HEADERS];
} expect_t;

typedef struct {
  const c8* name;
  const c8* head;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "get_root",
    .head = "GET / HTTP/1.1",
    .expect = { .method = "GET", .target = "/" },
  },
  {
    .name = "headers_kept",
    .head = "POST /shot HTTP/1.1\r\nHost: A\r\nContent-Length: 4",
    .expect = {
      .method = "POST",
      .target = "/shot",
      .headers = { { "Host", "A" }, { "Content-Length", "4" } },
    },
  },
  {
    .name = "http_1_0",
    .head = "GET / HTTP/1.0",
    .expect = { .method = "GET", .target = "/" },
  },
  {
    .name = "unknown_method_kept",
    .head = "BREW /pot HTTP/1.1",
    .expect = { .method = "BREW", .target = "/pot" },
  },
  {
    .name = "method_case_preserved",
    .head = "get / HTTP/1.1",
    .expect = { .method = "get", .target = "/" },
  },
  {
    .name = "query_target",
    .head = "GET /a?b=c HTTP/1.1",
    .expect = { .method = "GET", .target = "/a?b=c" },
  },
  {
    .name = "missing_version",
    .head = "GET /",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "bad_version",
    .head = "GET / HTTP/2",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "double_space",
    .head = "GET  / HTTP/1.1",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "empty_method",
    .head = " / HTTP/1.1",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "target_with_control_byte",
    .head = "GET /\x01 HTTP/1.1",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "header_without_colon",
    .head = "GET / HTTP/1.1\r\ngarbage",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "empty",
    .head = "",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_request_head_t parsed = sp_zero;
  sp_http_error_t err = sp_http_request_head_parse(sp_cstr_as_str(c->head), &parsed);
  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  sp_expect_str_eq_c(t, parsed.method, c->expect.method);
  sp_expect_str_eq_c(t, parsed.target, c->expect.target);
  return expect_headers(t, parsed.headers, c->expect.headers, HTTP_TEST_MAX_HEADERS);
}

sp_test_each_fn(http, request, test_t, tests, run);
