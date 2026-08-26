#include "../http.h"

typedef struct {
  sp_http_error_t err;
  const c8*       head;
  const c8*       rest;
} expect_t;

typedef struct {
  const c8* name;
  const c8* stream;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "head_with_leftover",
    .stream = "HTTP/1.1 200 OK\r\nX: y\r\n\r\nBODY",
    .expect = { .head = "HTTP/1.1 200 OK\r\nX: y", .rest = "BODY" },
  },
  {
    .name = "terminator_at_end",
    .stream = "GET / HTTP/1.1\r\n\r\n",
    .expect = { .head = "GET / HTTP/1.1", .rest = "" },
  },
  {
    .name = "empty_head",
    .stream = "\r\n\r\nrest",
    .expect = { .head = "", .rest = "rest" },
  },
  {
    .name = "eof_before_terminator",
    .stream = "HTTP/1.1 200",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "empty_stream",
    .stream = "",
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_io_reader_t reader = sp_zero;
  sp_io_reader_from_mem(&reader, c->stream, sp_cstr_len(c->stream));

  sp_str_t head = sp_zero;
  sp_http_error_t err = sp_http_head_read(&reader, &head);
  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  sp_expect_str_eq_c(t, head, c->expect.head);

  c8 rest [64];
  u64 rest_len = 0;
  sp_io_read(&reader, rest, sizeof(rest), &rest_len);
  sp_expect_str_eq_c(t, sp_str(rest, (u32)rest_len), c->expect.rest);
  return SP_OK;
}

sp_test_each_fn(http, stream, test_t, tests, run);
