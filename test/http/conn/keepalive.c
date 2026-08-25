#include "conn.h"

#define KEEPALIVE_TEST_MAX_REQUESTS 4

typedef struct {
  u32       requests;
  const c8* wire;
  bool      closed;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  u32       chunk;
  bool      eof;
  expect_t  expect;
} test_t;


static const test_t tests [] = {
  {
    .name = "two_requests_in_one_buffer",
    .input = "GET /a HTTP/1.1\r\n\r\nGET /b HTTP/1.1\r\n\r\n",
    .expect = { .requests = 2, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/aHTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/b" },
  },
  {
    .name = "pipelined_bytes_survive_reply",
    .input = "GET /a HTTP/1.1\r\n\r\nGET /b HTTP/1.1\r\n\r\n",
    .chunk = 5,
    .expect = { .requests = 2, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/aHTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/b" },
  },
  {
    .name = "body_consumed_exactly",
    .input = "POST /a HTTP/1.1\r\nContent-Length: 3\r\n\r\nxyzGET /b HTTP/1.1\r\n\r\n",
    .expect = { .requests = 2, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/aHTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/b" },
  },
  {
    .name = "connection_close_ends_after_first",
    .input = "GET /a HTTP/1.1\r\nConnection: close\r\n\r\nGET /b HTTP/1.1\r\n\r\n",
    .expect = { .requests = 1, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\n/a", .closed = true },
  },
  {
    .name = "http_1_0_ends_after_first",
    .input = "GET /a HTTP/1.0\r\n\r\nGET /b HTTP/1.1\r\n\r\n",
    .expect = { .requests = 1, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\n/a", .closed = true },
  },
  {
    .name = "http_1_0_ignores_keep_alive",
    .input = "GET /a HTTP/1.0\r\nConnection: keep-alive\r\n\r\nGET /b HTTP/1.0\r\nConnection: keep-alive\r\n\r\n",
    .expect = { .requests = 1, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\n/a", .closed = true },
  },
  {
    .name = "eof_after_reply_closes",
    .input = "GET /a HTTP/1.1\r\n\r\n",
    .eof = true,
    .expect = { .requests = 1, .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\n/a", .closed = true },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  conn_harness_t h;
  harness_init(&h, harness_desc(sp_test_arena(t), 0, 0, 0), c->input);
  h.chunk = c->chunk;
  h.eof = c->eof;

  u32 requests = 0;
  u32 want = 0;
  sp_for(it, KEEPALIVE_TEST_MAX_REQUESTS) {
    want = harness_drive(&h);
    if (!(want & SP_HTTP_WANT_REQUEST)) break;
    requests++;
    sp_http_conn_reply(&h.conn, sp_http_reply_text(200, h.conn.ctx.path));
  }

  sp_expect_eq(t, requests, c->expect.requests);
  sp_expect_str_eq_c(t, harness_wire(&h), c->expect.wire);
  sp_expect_eq(t, (want & SP_HTTP_WANT_CLOSE) != 0, c->expect.closed);

  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}

sp_test_each_fn(conn, keepalive, test_t, tests, run);
