#include "conn.h"

#define STREAM_TEST_MAX_STEPS 6
#define STREAM_TEST_HEAD "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nConnection: close\r\n\r\n"

typedef enum {
  STEP_NONE,
  STEP_WRITE,
  STEP_DRAIN,
  STEP_PEER_EOF,
  STEP_SEND_FAIL,
  STEP_CLOSE,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  const c8*   text;
  sp_err_t    err;
} step_t;

typedef struct {
  const c8* wire;
  bool      dead;
  u32       want;
} expect_t;

typedef struct {
  const c8* name;
  u32       stream_max;
  step_t    steps [STREAM_TEST_MAX_STEPS];
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "head_then_frame",
    .steps = {
      { .kind = STEP_WRITE, .text = "data: a\n\n" },
      { .kind = STEP_DRAIN },
    },
    .expect = { .wire = STREAM_TEST_HEAD "data: a\n\n", .want = SP_HTTP_WANT_RECV },
  },
  {
    .name = "writes_before_drain_follow_head_in_order",
    .steps = {
      { .kind = STEP_WRITE, .text = "1" },
      { .kind = STEP_WRITE, .text = "2" },
      { .kind = STEP_DRAIN },
    },
    .expect = { .wire = STREAM_TEST_HEAD "12", .want = SP_HTTP_WANT_RECV },
  },
  {
    .name = "pending_frame_wants_send",
    .steps = {
      { .kind = STEP_DRAIN },
      { .kind = STEP_WRITE, .text = "x" },
    },
    .expect = { .wire = STREAM_TEST_HEAD, .want = SP_HTTP_WANT_RECV | SP_HTTP_WANT_SEND },
  },
  {
    .name = "ring_wraps",
    .stream_max = 16,
    .steps = {
      { .kind = STEP_WRITE, .text = "0123456789" },
      { .kind = STEP_DRAIN },
      { .kind = STEP_WRITE, .text = "abcdefghij" },
      { .kind = STEP_DRAIN },
    },
    .expect = { .wire = STREAM_TEST_HEAD "0123456789abcdefghij", .want = SP_HTTP_WANT_RECV },
  },
  {
    .name = "overflow_kills_stream",
    .stream_max = 8,
    .steps = {
      { .kind = STEP_WRITE, .text = "0123456789", .err = SP_ERR_IO_NO_SPACE },
    },
    .expect = { .wire = "", .dead = true, .want = SP_HTTP_WANT_CLOSE },
  },
  {
    .name = "peer_eof_kills_stream",
    .steps = {
      { .kind = STEP_DRAIN },
      { .kind = STEP_PEER_EOF },
      { .kind = STEP_WRITE, .text = "x", .err = SP_ERR_IO_EOF },
    },
    .expect = { .wire = STREAM_TEST_HEAD, .dead = true, .want = SP_HTTP_WANT_CLOSE },
  },
  {
    .name = "send_fail_kills_stream",
    .steps = {
      { .kind = STEP_DRAIN },
      { .kind = STEP_WRITE, .text = "x" },
      { .kind = STEP_SEND_FAIL },
    },
    .expect = { .wire = STREAM_TEST_HEAD, .dead = true, .want = SP_HTTP_WANT_CLOSE },
  },
  {
    .name = "close_flushes_pending",
    .steps = {
      { .kind = STEP_WRITE, .text = "x" },
      { .kind = STEP_CLOSE },
      { .kind = STEP_DRAIN },
    },
    .expect = { .wire = STREAM_TEST_HEAD "x", .dead = true, .want = SP_HTTP_WANT_CLOSE },
  },
  {
    .name = "close_releases_conn",
    .steps = {
      { .kind = STEP_DRAIN },
      { .kind = STEP_CLOSE },
    },
    .expect = { .wire = STREAM_TEST_HEAD, .dead = true, .want = SP_HTTP_WANT_CLOSE },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  conn_harness_t h;
  harness_init(&h, harness_desc(sp_test_arena(t), 0, 0, c->stream_max), "GET /events HTTP/1.1\r\n\r\n");

  sp_must(t, harness_drive(&h) & SP_HTTP_WANT_REQUEST);
  sp_http_stream_t* stream = sp_http_ctx_stream(&h.conn.ctx);
  sp_must(t, stream != SP_NULLPTR);
  sp_http_conn_reply(&h.conn, sp_http_reply_stream(stream, sp_str_lit("text/event-stream")));

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));
    switch (step->kind) {
      case STEP_NONE: break;
      case STEP_WRITE: {
        sp_expect_err_eq(t, sp_io_write_str(&stream->base, sp_cstr_as_str(step->text), SP_NULLPTR), step->err);
        break;
      }
      case STEP_DRAIN: {
        while (harness_drain(&h)) {}
        break;
      }
      case STEP_PEER_EOF: {
        sp_http_conn_received(&h.conn, 0);
        break;
      }
      case STEP_SEND_FAIL: {
        sp_http_conn_sent(&h.conn, 0);
        break;
      }
      case STEP_CLOSE: {
        sp_http_stream_close(stream);
        break;
      }
    }
    if (step->kind == STEP_NONE) break;
  }
  sp_test_kv_clear(t, "step");

  sp_expect_str_eq_c(t, harness_wire(&h), c->expect.wire);
  sp_expect_eq(t, sp_http_stream_closed(stream), c->expect.dead);
  sp_expect_eq(t, sp_http_conn_step(&h.conn), c->expect.want);

  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}

sp_test_each_fn(conn, stream, test_t, tests, run);

sp_test(conn, stream_discarded_on_oneshot) {
  conn_harness_t h;
  harness_init(&h, harness_desc(sp_test_arena(t), 0, 0, 0), "GET /a HTTP/1.1\r\n\r\nGET /events HTTP/1.1\r\n\r\n");

  sp_must(t, harness_drive(&h) & SP_HTTP_WANT_REQUEST);
  sp_http_stream_t* stream = sp_http_ctx_stream(&h.conn.ctx);
  sp_io_write_str(&stream->base, sp_str_lit("junk"), SP_NULLPTR);
  sp_http_conn_reply(&h.conn, sp_http_reply_text(200, sp_str_lit("ok")));
  sp_expect_eq(t, h.conn.stream.held, false);

  sp_must(t, harness_drive(&h) & SP_HTTP_WANT_REQUEST);
  sp_http_stream_t* second = sp_http_ctx_stream(&h.conn.ctx);
  sp_http_conn_reply(&h.conn, sp_http_reply_stream(second, sp_str_lit("text/event-stream")));
  sp_io_write_str(&second->base, sp_str_lit("data\n"), SP_NULLPTR);
  while (harness_drain(&h)) {}

  sp_expect_str_eq_c(t, harness_wire(&h),
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\nok"
    STREAM_TEST_HEAD "data\n");
  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}

sp_test(conn, stream_head_request_elides_frames) {
  conn_harness_t h;
  harness_init(&h, harness_desc(sp_test_arena(t), 0, 0, 0), "HEAD /events HTTP/1.1\r\n\r\n");

  sp_must(t, harness_drive(&h) & SP_HTTP_WANT_REQUEST);
  sp_http_stream_t* stream = sp_http_ctx_stream(&h.conn.ctx);
  sp_io_write_str(&stream->base, sp_str_lit("x"), SP_NULLPTR);
  sp_http_conn_reply(&h.conn, sp_http_reply_stream(stream, sp_str_lit("text/event-stream")));
  u32 want = harness_drive(&h);

  sp_expect_str_eq_c(t, harness_wire(&h), STREAM_TEST_HEAD);
  sp_expect_eq(t, want, (u32)SP_HTTP_WANT_CLOSE);
  sp_expect_eq(t, sp_http_stream_closed(stream), true);
  sp_expect_eq(t, h.conn.stream.held, false);
  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}

sp_test(conn, stream_full_in_buffer_stops_recv) {
  conn_harness_t h;
  c8 input [CONN_TEST_HEAD_MAX + CONN_TEST_BODY_MAX + 64];
  sp_str_t request = sp_str_lit("GET /events HTTP/1.1\r\n\r\n");
  sp_mem_copy(input, request.data, request.len);
  sp_for(it, sizeof(input) - request.len - 1) {
    input[request.len + it] = 'x';
  }
  input[sizeof(input) - 1] = 0;
  harness_init(&h, harness_desc(sp_test_arena(t), 0, 0, 0), input);

  sp_must(t, harness_drive(&h) & SP_HTTP_WANT_REQUEST);
  sp_must_eq(t, h.conn.in.len, h.conn.in.cap);
  sp_http_stream_t* stream = sp_http_ctx_stream(&h.conn.ctx);
  sp_http_conn_reply(&h.conn, sp_http_reply_stream(stream, sp_str_lit("text/event-stream")));
  while (harness_drain(&h)) {}

  sp_expect_eq(t, sp_http_conn_step(&h.conn), 0u);
  sp_io_write_str(&stream->base, sp_str_lit("data\n"), SP_NULLPTR);
  sp_expect_eq(t, sp_http_conn_step(&h.conn), (u32)SP_HTTP_WANT_SEND);
  while (harness_drain(&h)) {}
  sp_expect_str_eq_c(t, harness_wire(&h), STREAM_TEST_HEAD "data\n");
  sp_http_conn_deinit(&h.conn);
  return SP_OK;
}
