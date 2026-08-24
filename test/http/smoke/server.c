#include "client.h"

#define SERVER_TEST_MAX_CONTAINS 4
#define SERVER_TEST_MAX_COUNTED  2
#define SERVER_TEST_MAX_CLIENTS  2
#define SERVER_TEST_PUMP_NS      (5ull * 1000 * 1000)
#define SERVER_TEST_MAX_PUMPS    2000
#define SERVER_TEST_IDLE_MS      10000

typedef struct {
  const c8* contains [SERVER_TEST_MAX_CONTAINS];
  counted_t counted [SERVER_TEST_MAX_COUNTED];
  bool      empty;
} expect_t;

typedef struct {
  const c8* name;
  const c8* send;
  u32       frames;
  u32       clients;
  u32       max_conns;
  u32       idle_ms;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "oneshot",
    .send = "GET /hello HTTP/1.1\r\nConnection: close\r\n\r\n",
    .expect = { .contains = { "HTTP/1.1 200 OK", "hello" } },
  },
  {
    .name = "keep_alive_serves_two_requests_on_one_socket",
    .send = "GET /hello HTTP/1.1\r\n\r\nGET /hello HTTP/1.1\r\nConnection: close\r\n\r\n",
    .expect = { .counted = { { "HTTP/1.1 200 OK", 2 }, { "hello", 2 } } },
  },
  {
    .name = "post_body_reaches_handler",
    .send = "POST /echo HTTP/1.1\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello",
    .expect = { .contains = { "application/json", "hello" } },
  },
  {
    .name = "not_found",
    .send = "GET /nope HTTP/1.1\r\nConnection: close\r\n\r\n",
    .expect = { .contains = { "HTTP/1.1 404 Not Found" } },
  },
  {
    .name = "stream_receives_frames_until_closed",
    .send = "GET /events HTTP/1.1\r\n\r\n",
    .frames = 3,
    .expect = { .contains = { "text/event-stream", "F0\n", "F1\n", "F2\n" } },
  },
  {
    .name = "idle_connection_closed",
    .send = "",
    .idle_ms = 50,
    .expect = { .empty = true },
  },
  {
    .name = "backpressure_serves_beyond_max_conns",
    .send = "GET /hello HTTP/1.1\r\nConnection: close\r\n\r\n",
    .clients = 2,
    .max_conns = 1,
    .expect = { .counted = { { "HTTP/1.1 200 OK", 2 }, { "hello", 2 } } },
  },
};

typedef struct {
  sp_http_stream_t* stream;
} app_t;

static sp_http_reply_t on_hello(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("hello"));
}

static sp_http_reply_t on_echo(sp_http_ctx_t* c) {
  return sp_http_reply_json(200, c->body);
}

static sp_http_reply_t on_events(sp_http_ctx_t* c) {
  app_t* app = sp_cast(app_t*, c->user_data);
  app->stream = sp_http_ctx_stream(c);
  return sp_http_reply_stream(app->stream, sp_str_lit("text/event-stream"));
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET,  "/hello",  on_hello },
  { SP_HTTP_POST, "/echo",   on_echo },
  { SP_HTTP_GET,  "/events", on_events },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_io_uring_t ring = sp_zero;
  sp_err_t err = sp_io_uring_init(&ring, 64);
  if (err == SP_ERR_SYS_UNSUPPORTED || err == SP_ERR_SYS_ACCESS_DENIED) {
    return sp_test_skip(t, "io_uring unavailable");
  }
  sp_must_ok(t, err);

  app_t app = sp_zero;
  sp_http_server_t server = sp_zero;
  sp_http_error_t herr = sp_http_server_init(&server, (sp_http_server_desc_t) {
    .io = sp_io_uring_as_io(&ring),
    .addr = { .octets = { 127, 0, 0, 1 } },
    .router = { .routes = routes, .count = sp_carr_len(routes), .user_data = &app },
    .conn = { .mem = sp_test_arena(t) },
    .max_conns = c->max_conns ? c->max_conns : 4,
    .idle_ms = c->idle_ms ? c->idle_ms : SERVER_TEST_IDLE_MS,
  });
  sp_must_eq(t, (s32)herr, (s32)SP_HTTP_OK);

  u32 num_clients = c->clients ? c->clients : 1;
  client_t clients [SERVER_TEST_MAX_CLIENTS] = sp_zero;
  sp_thread_t threads [SERVER_TEST_MAX_CLIENTS] = sp_zero;
  sp_for(it, num_clients) {
    clients[it] = (client_t) { .port = server.port, .send = c->send };
    sp_thread_init(&threads[it], client_main, &clients[it]);
  }

  u32 written = 0;
  sp_for(it, SERVER_TEST_MAX_PUMPS) {
    bool done = true;
    sp_for(at, num_clients) {
      if (!sp_atomic_s32_load(&clients[at].done, SP_ATOMIC_ACQUIRE)) done = false;
    }
    if (done) break;
    sp_http_server_pump(&server, sp_io_timeout_after(SERVER_TEST_PUMP_NS));
    if (!app.stream) continue;
    if (written < c->frames) {
      sp_fmt_io(&app.stream->base, "F{}\n", sp_fmt_uint(written));
      written++;
    }
    else {
      sp_http_stream_close(app.stream);
      app.stream = SP_NULLPTR;
    }
  }
  sp_for(it, num_clients) {
    sp_thread_join(&threads[it]);
  }
  sp_http_server_deinit(&server);
  sp_io_uring_deinit(&ring);

  c8 combined [SERVER_TEST_MAX_CLIENTS * CLIENT_TEST_CAPTURE];
  u32 combined_len = 0;
  sp_for(it, num_clients) {
    sp_expect_eq(t, sp_atomic_s32_load(&clients[it].done, SP_ATOMIC_ACQUIRE), 1);
    sp_mem_copy(combined + combined_len, clients[it].captured, clients[it].captured_len);
    combined_len += clients[it].captured_len;
  }
  sp_str_t captured = sp_str(combined, combined_len);

  if (c->expect.empty) {
    sp_expect_eq(t, combined_len, 0u);
    sp_expect_eq(t, clients[0].eof, true);
  }
  expect_contains(t, captured, c->expect.contains, SERVER_TEST_MAX_CONTAINS);
  expect_counted(t, captured, c->expect.counted, SERVER_TEST_MAX_COUNTED);
  return SP_OK;
}

sp_test_each_fn(server, pump, test_t, tests, run, .serial = true);

static s32 run_main(void* user_data) {
  sp_http_server_run(sp_cast(sp_http_server_t*, user_data));
  return 0;
}

sp_test(server, run, .serial = true) {
  sp_io_uring_t ring = sp_zero;
  sp_err_t err = sp_io_uring_init(&ring, 64);
  if (err == SP_ERR_SYS_UNSUPPORTED || err == SP_ERR_SYS_ACCESS_DENIED) {
    return sp_test_skip(t, "io_uring unavailable");
  }
  sp_must_ok(t, err);

  sp_http_server_t server = sp_zero;
  sp_http_error_t herr = sp_http_server_init(&server, (sp_http_server_desc_t) {
    .io = sp_io_uring_as_io(&ring),
    .addr = { .octets = { 127, 0, 0, 1 } },
    .router = { .routes = routes, .count = sp_carr_len(routes) },
    .conn = { .mem = sp_test_arena(t) },
    .max_conns = 4,
    .idle_ms = SERVER_TEST_IDLE_MS,
  });
  sp_must_eq(t, (s32)herr, (s32)SP_HTTP_OK);

  sp_thread_t pump = sp_zero;
  sp_thread_init(&pump, run_main, &server);

  client_t client = { .port = server.port, .send = "GET /hello HTTP/1.1\r\nConnection: close\r\n\r\n" };
  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, client_main, &client);
  sp_thread_join(&thread);

  sp_http_server_stop(&server);
  sp_thread_join(&pump);
  sp_http_server_deinit(&server);
  sp_io_uring_deinit(&ring);

  sp_str_t captured = sp_str(client.captured, client.captured_len);
  sp_expect_ne(t, sp_str_find(captured, sp_str_lit("HTTP/1.1 200 OK")), SP_STR_NO_MATCH);
  sp_expect_ne(t, sp_str_find(captured, sp_str_lit("hello")), SP_STR_NO_MATCH);
  return SP_OK;
}

sp_test(server, deinit_with_live_client, .serial = true) {
  sp_io_uring_t ring = sp_zero;
  sp_err_t err = sp_io_uring_init(&ring, 64);
  if (err == SP_ERR_SYS_UNSUPPORTED || err == SP_ERR_SYS_ACCESS_DENIED) {
    return sp_test_skip(t, "io_uring unavailable");
  }
  sp_must_ok(t, err);

  sp_http_server_t server = sp_zero;
  sp_http_error_t herr = sp_http_server_init(&server, (sp_http_server_desc_t) {
    .io = sp_io_uring_as_io(&ring),
    .addr = { .octets = { 127, 0, 0, 1 } },
    .router = { .routes = routes, .count = sp_carr_len(routes) },
    .conn = { .mem = sp_test_arena(t) },
    .max_conns = 4,
    .idle_ms = SERVER_TEST_IDLE_MS,
  });
  sp_must_eq(t, (s32)herr, (s32)SP_HTTP_OK);

  client_t client = { .port = server.port, .send = "GET /partial" };
  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, client_main, &client);

  sp_for(it, 40) {
    sp_http_server_pump(&server, sp_io_timeout_after(SERVER_TEST_PUMP_NS));
  }
  sp_http_server_deinit(&server);
  sp_thread_join(&thread);
  sp_io_uring_deinit(&ring);

  sp_expect_eq(t, client.eof, true);
  sp_expect_eq(t, client.captured_len, 0u);
  return SP_OK;
}
