#include "conn.h"

typedef struct {
  sp_http_method_t method;
  const c8*        path;
  const c8*        query;
  const c8*        body;
  header_t         headers [HTTP_TEST_MAX_HEADERS];
  const c8*        wire;
  s32              status;
  bool             closed;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  u32       chunk;
  bool      eof;
  u32       head_max;
  u32       body_max;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "get",
    .input = "GET / HTTP/1.1\r\n\r\n",
    .expect = { .path = "/" },
  },
  {
    .name = "query_split_from_path",
    .input = "GET /events?token=abc HTTP/1.1\r\n\r\n",
    .expect = { .path = "/events", .query = "token=abc" },
  },
  {
    .name = "headers_exposed",
    .input = "GET / HTTP/1.1\r\nX-Token: abc\r\nHost: A\r\n\r\n",
    .expect = { .path = "/", .headers = { { "x-token", "abc" }, { "host", "A" } } },
  },
  {
    .name = "post_body",
    .input = "POST /act HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello",
    .expect = { .method = SP_HTTP_POST, .path = "/act", .body = "hello" },
  },
  {
    .name = "byte_at_a_time",
    .input = "POST /act HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello",
    .chunk = 1,
    .expect = { .method = SP_HTTP_POST, .path = "/act", .body = "hello" },
  },
  {
    .name = "head_split_mid_terminator",
    .input = "GET / HTTP/1.1\r\n\r\n",
    .chunk = 15,
    .expect = { .path = "/" },
  },
  {
    .name = "head_method",
    .input = "HEAD / HTTP/1.1\r\n\r\n",
    .expect = { .method = SP_HTTP_HEAD, .path = "/" },
  },
  {
    .name = "http_1_0",
    .input = "GET / HTTP/1.0\r\n\r\n",
    .expect = { .path = "/" },
  },
  {
    .name = "path_percent_decoded",
    .input = "GET /a%20b HTTP/1.1\r\n\r\n",
    .expect = { .path = "/a b" },
  },
  {
    .name = "encoded_traversal_decoded_into_path",
    .input = "GET /%2e%2e/x HTTP/1.1\r\n\r\n",
    .expect = { .path = "/../x" },
  },
  {
    .name = "query_stays_raw",
    .input = "GET /q?name=a%20b HTTP/1.1\r\n\r\n",
    .expect = { .path = "/q", .query = "name=a%20b" },
  },
  {
    .name = "expect_continue_answered_before_body",
    .input = "POST / HTTP/1.1\r\nContent-Length: 2\r\nExpect: 100-continue\r\n\r\nok",
    .expect = { .method = SP_HTTP_POST, .path = "/", .body = "ok", .wire = "HTTP/1.1 100 Continue\r\n\r\n" },
  },
  {
    .name = "body_exactly_body_max",
    .input = "POST / HTTP/1.1\r\nContent-Length: 8\r\n\r\n12345678",
    .body_max = 8,
    .expect = { .method = SP_HTTP_POST, .path = "/", .body = "12345678" },
  },
  {
    .name = "bad_request_line",
    .input = "GARBAGE\r\n\r\n",
    .expect = { .status = 400, .closed = true },
  },
  {
    .name = "bad_header",
    .input = "GET / HTTP/1.1\r\nno colon\r\n\r\n",
    .expect = { .status = 400, .closed = true },
  },
  {
    .name = "chunked_body_rejected",
    .input = "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n",
    .expect = { .status = 411, .closed = true },
  },
  {
    .name = "body_too_large",
    .input = "POST / HTTP/1.1\r\nContent-Length: 9\r\n\r\n123456789",
    .body_max = 8,
    .expect = { .status = 413, .closed = true },
  },
  {
    .name = "head_too_large",
    .input = "GET / HTTP/1.1\r\nX: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n\r\n",
    .head_max = 64,
    .expect = { .status = 431, .closed = true },
  },
  {
    .name = "duplicate_content_length_mismatch",
    .input = "POST / HTTP/1.1\r\nContent-Length: 3\r\nContent-Length: 4\r\n\r\nabcd",
    .expect = { .status = 400, .closed = true },
  },
  {
    .name = "content_length_not_numeric",
    .input = "POST / HTTP/1.1\r\nContent-Length: abc\r\n\r\n",
    .expect = { .status = 400, .closed = true },
  },
  {
    .name = "content_length_overflow",
    .input = "POST / HTTP/1.1\r\nContent-Length: 99999999999999999999\r\n\r\n",
    .expect = { .status = 400, .closed = true },
  },
  {
    .name = "chunked_with_length_rejected",
    .input = "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\nContent-Length: 3\r\n\r\n",
    .expect = { .status = 400, .closed = true },
  },
  {
    .name = "expect_continue_over_body_max_rejected_without_continue",
    .input = "POST / HTTP/1.1\r\nContent-Length: 9\r\nExpect: 100-continue\r\n\r\n",
    .body_max = 8,
    .expect = { .status = 413, .closed = true },
  },
  {
    .name = "unknown_method_answered_after_body",
    .input = "BREW / HTTP/1.1\r\nContent-Length: 3\r\n\r\nabc",
    .expect = { .status = 501 },
  },
  {
    .name = "eof_mid_head_closes_silently",
    .input = "GET / HTT",
    .eof = true,
    .expect = { .closed = true },
  },
  {
    .name = "eof_mid_body_closes_silently",
    .input = "POST / HTTP/1.1\r\nContent-Length: 5\r\n\r\nhel",
    .eof = true,
    .expect = { .closed = true },
  },
  {
    .name = "eof_when_idle_closes",
    .input = "",
    .eof = true,
    .expect = { .closed = true },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  conn_harness_t h;
  harness_init(&h, harness_desc(sp_test_arena(t), c->head_max, c->body_max, 0), c->input);
  h.chunk = c->chunk;
  h.eof = c->eof;

  u32 want = harness_drive(&h);
  sp_expect_eq(t, sp_http_conn_step(&h.conn), want);

  if (c->expect.wire) {
    sp_expect_str_eq_c(t, harness_wire(&h), c->expect.wire);
  }
  if (c->expect.status) {
    sp_expect_eq(t, harness_first_status(&h), c->expect.status);
    sp_expect_eq(t, want & SP_HTTP_WANT_REQUEST, 0u);
  }
  if (!c->expect.status && !c->expect.wire) {
    sp_expect_eq(t, h.wire_len, 0u);
  }
  sp_expect_eq(t, (want & SP_HTTP_WANT_CLOSE) != 0, c->expect.closed);

  if (c->expect.path) {
    sp_must(t, want & SP_HTTP_WANT_REQUEST);
    sp_http_ctx_t* ctx = &h.conn.ctx;
    sp_expect_eq(t, (s32)ctx->method, (s32)c->expect.method);
    sp_expect_str_eq_c(t, ctx->path, c->expect.path);
    sp_expect_str_eq_c(t, ctx->query, c->expect.query ? c->expect.query : "");
    sp_expect_str_eq_c(t, ctx->body, c->expect.body ? c->expect.body : "");
    sp_carr_for(c->expect.headers, it) {
      if (!c->expect.headers[it].name) break;
      sp_test_kv_c(t, "header", c->expect.headers[it].name);
      sp_expect_str_eq_c(t, sp_http_ctx_header(ctx, c->expect.headers[it].name), c->expect.headers[it].value);
    }
    sp_test_kv_clear(t, "header");
  }

  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}

sp_test_each_fn(conn, request, test_t, tests, run);
