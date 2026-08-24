#include "conn.h"

typedef enum {
  BUILD_RAW,
  BUILD_TEXT,
  BUILD_JSON,
  BUILD_STATUS,
} build_t;

typedef struct {
  const c8* wire;
  bool      keep_alive;
} expect_t;

typedef struct {
  const c8* name;
  const c8* request;
  build_t   build;
  s32       status;
  const c8* content_type;
  const c8* body;
  header_t  headers [HTTP_TEST_MAX_HEADERS];
  u32       send_chunk;
  bool      send_fail;
  bool      peer_eof;
  bool      huge_header;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "text",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hi",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\nhi", .keep_alive = true },
  },
  {
    .name = "json",
    .build = BUILD_JSON,
    .status = 200,
    .body = "{}",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}", .keep_alive = true },
  },
  {
    .name = "status_only",
    .build = BUILD_STATUS,
    .status = 404,
    .expect = { .wire = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", .keep_alive = true },
  },
  {
    .name = "no_content_omits_length",
    .build = BUILD_STATUS,
    .status = 204,
    .expect = { .wire = "HTTP/1.1 204 No Content\r\n\r\n", .keep_alive = true },
  },
  {
    .name = "raw_without_content_type",
    .status = 200,
    .body = "x",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nx", .keep_alive = true },
  },
  {
    .name = "extra_headers_after_content_type",
    .status = 200,
    .content_type = "text/plain",
    .body = "x",
    .headers = { { "Cache-Control", "no-cache" }, { "X-A", "1" } },
    .expect = {
      .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nCache-Control: no-cache\r\nX-A: 1\r\nContent-Length: 1\r\n\r\nx",
      .keep_alive = true,
    },
  },
  {
    .name = "head_elides_body",
    .request = "HEAD / HTTP/1.1\r\n\r\n",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hello",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\n", .keep_alive = true },
  },
  {
    .name = "connection_close_honored",
    .request = "GET / HTTP/1.1\r\nConnection: close\r\n\r\n",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hi",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\nhi" },
  },
  {
    .name = "http_1_0_closes",
    .request = "GET / HTTP/1.0\r\n\r\n",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hi",
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\nhi" },
  },
  {
    .name = "send_fail_discards_and_closes",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hi",
    .send_fail = true,
    .expect = { .wire = "" },
  },
  {
    .name = "peer_eof_still_flushes_reply",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hi",
    .peer_eof = true,
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\nhi" },
  },
  {
    .name = "reply_head_overflow_closes",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hi",
    .huge_header = true,
    .expect = { .wire = "" },
  },
  {
    .name = "partial_sends",
    .build = BUILD_TEXT,
    .status = 200,
    .body = "hello world",
    .send_chunk = 3,
    .expect = { .wire = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nhello world", .keep_alive = true },
  },
};

static sp_http_reply_t build(const test_t* c) {
  sp_str_t body = c->body ? sp_cstr_as_str(c->body) : sp_zero_s(sp_str_t);
  switch (c->build) {
    case BUILD_TEXT:   return sp_http_reply_text(c->status, body);
    case BUILD_JSON:   return sp_http_reply_json(c->status, body);
    case BUILD_STATUS: return sp_http_reply_status(c->status);
    case BUILD_RAW:    break;
  }
  sp_http_reply_t reply = {
    .status = c->status,
    .content_type = c->content_type ? sp_cstr_as_str(c->content_type) : sp_zero_s(sp_str_t),
    .body = body,
  };
  sp_carr_for(c->headers, it) {
    if (!c->headers[it].name) break;
    sp_http_reply_header(&reply, sp_cstr_as_str(c->headers[it].name), sp_cstr_as_str(c->headers[it].value));
  }
  return reply;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  conn_harness_t h;
  harness_init(&h, harness_desc(sp_test_arena(t), 0, 0, 0), c->request ? c->request : "GET / HTTP/1.1\r\n\r\n");
  h.send_chunk = c->send_chunk;

  sp_must(t, harness_drive(&h) & SP_HTTP_WANT_REQUEST);
  sp_http_reply_t reply = build(c);
  if (c->huge_header) {
    u32 huge = 2048;
    c8* big = sp_alloc_n(sp_test_arena(t), c8, huge);
    sp_for(it, huge) {
      big[it] = 'a';
    }
    sp_http_reply_header(&reply, sp_str_lit("X-Huge"), sp_str(big, huge));
  }
  sp_http_conn_reply(&h.conn, reply);
  if (c->send_fail) sp_http_conn_sent(&h.conn, 0);
  if (c->peer_eof) sp_http_conn_received(&h.conn, 0);
  u32 want = harness_drive(&h);
  sp_expect_eq(t, sp_http_conn_step(&h.conn), want);

  sp_expect_str_eq_c(t, harness_wire(&h), c->expect.wire);
  sp_expect_eq(t, (want & SP_HTTP_WANT_CLOSE) != 0, !c->expect.keep_alive);
  sp_expect_eq(t, (want & SP_HTTP_WANT_RECV) != 0, c->expect.keep_alive);

  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}

sp_test_each_fn(conn, reply, test_t, tests, run);
