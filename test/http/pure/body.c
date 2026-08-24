#include "../http.h"

typedef struct {
  sp_http_error_t     err;
  sp_http_body_kind_t kind;
  u64                 length;
} expect_t;

typedef struct {
  const c8* name;
  header_t  headers [HTTP_TEST_MAX_HEADERS];
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "no_framing_headers",
    .expect = { .kind = SP_HTTP_BODY_NONE },
  },
  {
    .name = "content_length",
    .headers = { { "Content-Length", "12" } },
    .expect = { .kind = SP_HTTP_BODY_LENGTH, .length = 12 },
  },
  {
    .name = "content_length_zero",
    .headers = { { "Content-Length", "0" } },
    .expect = { .kind = SP_HTTP_BODY_LENGTH },
  },
  {
    .name = "duplicate_equal_lengths",
    .headers = { { "content-length", "12" }, { "Content-Length", "12" } },
    .expect = { .kind = SP_HTTP_BODY_LENGTH, .length = 12 },
  },
  {
    .name = "chunked",
    .headers = { { "Transfer-Encoding", "chunked" } },
    .expect = { .kind = SP_HTTP_BODY_CHUNKED },
  },
  {
    .name = "chunked_in_list",
    .headers = { { "Transfer-Encoding", "GZIP, Chunked" } },
    .expect = { .kind = SP_HTTP_BODY_CHUNKED },
  },
  {
    .name = "unrelated_headers_ignored",
    .headers = { { "Content-Type", "text/plain" }, { "X", "1" } },
    .expect = { .kind = SP_HTTP_BODY_NONE },
  },
  {
    .name = "conflicting_lengths",
    .headers = { { "Content-Length", "5" }, { "Content-Length", "9" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "length_not_numeric",
    .headers = { { "Content-Length", "abc" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "length_empty",
    .headers = { { "Content-Length", "" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "length_overflows_u64",
    .headers = { { "Content-Length", "99999999999999999999" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "te_not_chunked",
    .headers = { { "Transfer-Encoding", "gzip" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "te_and_length",
    .headers = { { "Transfer-Encoding", "chunked" }, { "Content-Length", "5" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t headers = header_lines(mem, c->headers, HTTP_TEST_MAX_HEADERS);

  sp_http_body_t body = sp_zero;
  sp_http_error_t err = sp_http_body_parse(headers, &body);
  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  sp_expect_eq(t, (s32)body.kind, (s32)c->expect.kind);
  sp_expect_eq(t, body.length, c->expect.length);
  return SP_OK;
}

sp_test_each_fn(http, body, test_t, tests, run);
