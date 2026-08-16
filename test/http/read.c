#include "http.h"

typedef struct {
  sp_http_error_t err;
  const c8*       body;
  const c8*       rest;
} expect_t;

typedef struct {
  const c8*           name;
  const c8*           stream;
  sp_http_body_kind_t kind;
  u64                 length;
  bool                discard; // no sink: body_read drains via sp_io_discard
  u64                 pull;    // read the body this many bytes at a time through sp_http_body_reader_t
  expect_t            expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "length_exact",
    .stream = "helloXX",
    .kind = SP_HTTP_BODY_LENGTH,
    .length = 5,
    .expect = { .body = "hello", .rest = "XX" },
  },
  {
    .name = "length_zero",
    .stream = "XX",
    .kind = SP_HTTP_BODY_LENGTH,
    .expect = { .body = "", .rest = "XX" },
  },
  {
    .name = "length_truncated",
    .stream = "hello",
    .kind = SP_HTTP_BODY_LENGTH,
    .length = 10,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked",
    .stream = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\nREST",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .body = "hello world", .rest = "REST" },
  },
  {
    .name = "chunked_extension",
    .stream = "5;ext=1\r\nhello\r\n0\r\n\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .body = "hello", .rest = "" },
  },
  {
    .name = "chunked_hex_size",
    .stream = "b\r\nhello world\r\n0\r\n\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .body = "hello world", .rest = "" },
  },
  {
    .name = "chunked_hex_size_upper",
    .stream = "B\r\nhello world\r\n0\r\n\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .body = "hello world", .rest = "" },
  },
  {
    .name = "chunked_size_overflows_u64",
    .stream = "ffffffffffffffffff\r\nhello\r\n0\r\n\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked_data_truncated",
    .stream = "5\r\nhel",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked_trailers_consumed",
    .stream = "5\r\nhello\r\n0\r\nX-T: 1\r\nY: 2\r\n\r\nREST",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .body = "hello", .rest = "REST" },
  },
  {
    .name = "chunked_bad_separator",
    .stream = "5\r\nhelloXX\r\n0\r\n\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked_bad_size",
    .stream = "zz\r\nhello\r\n0\r\n\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked_missing_terminator",
    .stream = "5\r\nhello\r\n0\r\n",
    .kind = SP_HTTP_BODY_CHUNKED,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "eof",
    .stream = "hello",
    .kind = SP_HTTP_BODY_EOF,
    .expect = { .body = "hello", .rest = "" },
  },
  {
    .name = "none",
    .stream = "hello",
    .kind = SP_HTTP_BODY_NONE,
    .expect = { .body = "", .rest = "hello" },
  },
  {
    .name = "length_discard",
    .stream = "helloXX",
    .kind = SP_HTTP_BODY_LENGTH,
    .length = 5,
    .discard = true,
    .expect = { .body = "hello", .rest = "XX" },
  },
  {
    .name = "chunked_discard",
    .stream = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\nREST",
    .kind = SP_HTTP_BODY_CHUNKED,
    .discard = true,
    .expect = { .body = "hello world", .rest = "REST" },
  },
  {
    .name = "eof_discard",
    .stream = "hello",
    .kind = SP_HTTP_BODY_EOF,
    .discard = true,
    .expect = { .body = "hello", .rest = "" },
  },
  {
    .name = "length_truncated_discard",
    .stream = "hello",
    .kind = SP_HTTP_BODY_LENGTH,
    .length = 10,
    .discard = true,
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked_pulled",
    .stream = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\nREST",
    .kind = SP_HTTP_BODY_CHUNKED,
    .pull = 3,
    .expect = { .body = "hello world", .rest = "REST" },
  },
  {
    .name = "length_pulled",
    .stream = "helloXX",
    .kind = SP_HTTP_BODY_LENGTH,
    .length = 5,
    .pull = 2,
    .expect = { .body = "hello", .rest = "XX" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);

  sp_io_reader_t reader = sp_zero;
  sp_io_reader_from_mem(&reader, c->stream, sp_cstr_len(c->stream));

  sp_io_dyn_mem_writer_t sink = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &sink);

  u64 len = 0;
  sp_http_body_t body = { .kind = c->kind, .length = c->length };
  sp_http_error_t err = SP_HTTP_OK;
  if (c->pull) {
    sp_http_body_reader_t br = sp_zero;
    sp_http_body_reader_init(&br, &reader, body);
    u8 buf [8];
    for (;;) {
      u64 got = 0;
      sp_err_t rerr = sp_io_read(&br.base, buf, sp_min(c->pull, sizeof(buf)), &got);
      sp_io_write(&sink.base, buf, got, SP_NULLPTR);
      len += got;
      if (rerr == SP_ERR_IO_EOF) break;
      if (rerr != SP_OK) {
        err = br.err != SP_HTTP_OK ? br.err : SP_HTTP_ERR_PROTOCOL;
        break;
      }
    }
  }
  else {
    err = sp_http_body_read(&reader, body, c->discard ? SP_NULLPTR : &sink.base, &len);
  }
  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (err != SP_HTTP_OK || c->expect.err != SP_HTTP_OK) return SP_OK;

  if (!c->discard) sp_expect_str_eq_c(t, sp_io_dyn_mem_writer_as_str(&sink), c->expect.body);
  sp_expect_eq(t, len, sp_cstr_len(c->expect.body));

  c8 rest [64];
  u64 rest_len = 0;
  sp_io_read(&reader, rest, sizeof(rest), &rest_len);
  sp_expect_str_eq_c(t, sp_str(rest, (u32)rest_len), c->expect.rest);
  return SP_OK;
}

sp_test_each_fn(http, read, test_t, tests, run);
