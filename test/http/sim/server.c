#include "harness.h"

typedef struct {
  const c8* contains;
  bool      eof;
} expect_t;

typedef struct {
  const c8* name;
  const c8* send;
  u64       chunk;
  u32       idle_ms;
  u64       advance_ms;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "get_ok",
    .send = "GET /hello HTTP/1.1\r\n\r\n",
    .expect = { .contains = "HTTP/1.1 200 OK" },
  },
  {
    .name = "not_found",
    .send = "GET /missing HTTP/1.1\r\n\r\n",
    .expect = { .contains = "404" },
  },
  {
    .name = "torn_delivery",
    .send = "GET /hello HTTP/1.1\r\n\r\n",
    .chunk = 1,
    .expect = { .contains = "HTTP/1.1 200 OK" },
  },
  {
    .name = "pipelined_pair",
    .send = "GET /hello HTTP/1.1\r\n\r\nGET /hello HTTP/1.1\r\n\r\n",
    .expect = { .contains = "HTTP/1.1 200 OK" },
  },
  {
    .name = "idle_conn_closed",
    .idle_ms = 10,
    .advance_ms = 1000,
    .expect = { .eof = true },
  },
};

static sp_http_reply_t on_hello(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("A"));
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET, "/hello", on_hello },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  fixture_t fx = sp_zero;
  sp_http_router_t router = { .routes = routes, .count = sp_carr_len(routes) };
  server_start(t, &fx, router, (server_opts_t) { .idle_ms = c->idle_ms });

  client_t client = client_connect(&fx);
  sp_must_eq(t, client.err, SP_OK);

  if (c->chunk) sp_io_sim_chunk(&fx.sim, client.socket, c->chunk);
  if (c->send) client_send(&fx, &client, c->send);
  if (c->advance_ms) {
    sp_io_sim_advance(&fx.sim, c->advance_ms * 1000 * 1000);
    server_run(&fx);
  }

  sp_str_t got = client_recv(&fx, &client);
  if (c->expect.contains) {
    sp_expect_ne(t, sp_str_find(got, sp_cstr_as_str(c->expect.contains)), SP_STR_NO_MATCH);
  }
  sp_expect_eq(t, client.eof, c->expect.eof);

  if (!client.eof) client_close(&client);
  server_stop(&fx);
  return SP_OK;
}

sp_test_each_fn(sim, server, test_t, tests, run);
