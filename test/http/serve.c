#include "client.h"

#define SERVE_TEST_MAX_CONTAINS 4
#define SERVE_TEST_MAX_COUNTED  2
#define SERVE_TEST_MAX_CLIENTS  2

typedef struct {
  const c8* contains [SERVE_TEST_MAX_CONTAINS];
  counted_t counted [SERVE_TEST_MAX_COUNTED];
} expect_t;

typedef struct {
  const c8* name;
  const c8* send;
  u32       clients;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "oneshot",
    .send = "GET /hello HTTP/1.1\r\nConnection: close\r\n\r\n",
    .expect = { .contains = { "HTTP/1.1 200 OK", "hello" } },
  },
  {
    .name = "keep_alive_serves_two_requests",
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
    .name = "stream_flushes_then_closes",
    .send = "GET /events HTTP/1.1\r\n\r\n",
    .expect = { .contains = { "text/event-stream", "F1", "F2" } },
  },
  {
    .name = "conn_reused_across_sockets",
    .send = "GET /hello HTTP/1.1\r\nConnection: close\r\n\r\n",
    .clients = 2,
    .expect = { .counted = { { "HTTP/1.1 200 OK", 2 }, { "hello", 2 } } },
  },
};

static sp_http_reply_t on_hello(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("hello"));
}

static sp_http_reply_t on_echo(sp_http_ctx_t* c) {
  return sp_http_reply_json(200, c->body);
}

static sp_http_reply_t on_events(sp_http_ctx_t* c) {
  sp_http_stream_t* stream = sp_http_ctx_stream(c);
  sp_io_write_str(&stream->base, sp_str_lit("F1\nF2\n"), SP_NULLPTR);
  return sp_http_reply_stream(stream, sp_str_lit("text/event-stream"));
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET,  "/hello",  on_hello },
  { SP_HTTP_POST, "/echo",   on_echo },
  { SP_HTTP_GET,  "/events", on_events },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  sp_must_ok(t, sp_sys_socket_open(&listener, sp_zero_s(sp_sys_handle_desc_t)));
  sp_must_ok(t, sp_sys_socket_bind(listener, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 } }));
  sp_must_ok(t, sp_sys_socket_listen(listener, 2));

  u16 port = 0;
  sp_must_ok(t, sp_sys_socket_local_port(listener, &port));

  sp_http_router_t router = { .routes = routes, .count = sp_carr_len(routes) };
  sp_http_conn_t conn = sp_zero;
  sp_http_conn_init(&conn, (sp_http_conn_desc_t) {
    .mem = sp_test_arena(t),
    .head_max = 1024,
    .body_max = 1024,
    .stream_max = 64,
  });

  u32 num_clients = c->clients ? c->clients : 1;
  client_t clients [SERVE_TEST_MAX_CLIENTS] = sp_zero;
  sp_for(it, num_clients) {
    clients[it] = (client_t) { .port = port, .send = c->send };
    sp_thread_t thread = sp_zero;
    sp_thread_init(&thread, client_main, &clients[it]);

    sp_sys_socket_t accepted = SP_SYS_INVALID_SOCKET;
    sp_must_ok(t, sp_sys_socket_accept(listener, sp_zero_s(sp_sys_handle_desc_t), &accepted));
    sp_http_conn_reset(&conn);
    sp_http_conn_serve(&conn, accepted, &router);
    sp_thread_join(&thread);
  }
  sp_http_conn_deinit(&conn);
  sp_sys_socket_close(listener);

  c8 combined [SERVE_TEST_MAX_CLIENTS * CLIENT_TEST_CAPTURE];
  u32 combined_len = 0;
  sp_for(it, num_clients) {
    sp_mem_copy(combined + combined_len, clients[it].captured, clients[it].captured_len);
    combined_len += clients[it].captured_len;
  }
  sp_str_t captured = sp_str(combined, combined_len);

  expect_contains(t, captured, c->expect.contains, SERVE_TEST_MAX_CONTAINS);
  expect_counted(t, captured, c->expect.counted, SERVE_TEST_MAX_COUNTED);
  return SP_OK;
}

sp_test_each_fn(conn, serve, test_t, tests, run, .serial = true);
