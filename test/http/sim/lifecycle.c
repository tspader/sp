#include "harness.h"

static sp_http_reply_t on_hello(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("A"));
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET, "/hello", on_hello },
};

static sp_http_router_t router() {
  return (sp_http_router_t) { .routes = routes, .count = sp_carr_len(routes) };
}

sp_test(sim, reset_frees_slot) {
  fixture_t fx = sp_zero;
  server_start(t, &fx, router(), (server_opts_t) { .max_conns = 1 });

  client_t a = client_connect(&fx);
  sp_must_eq(t, a.err, SP_OK);
  client_send(&fx, &a, "GET /hello HTTP/1.1\r\n\r\n");
  sp_expect_ne(t, sp_str_find(client_recv(&fx, &a), sp_str_lit("200 OK")), SP_STR_NO_MATCH);

  sp_io_sim_kill(&fx.sim, a.socket);
  server_run(&fx);

  client_t b = client_connect(&fx);
  sp_must_eq(t, b.err, SP_OK);
  client_send(&fx, &b, "GET /hello HTTP/1.1\r\n\r\n");
  sp_expect_ne(t, sp_str_find(client_recv(&fx, &b), sp_str_lit("200 OK")), SP_STR_NO_MATCH);

  client_close(&b);
  server_stop(&fx);
  return SP_OK;
}

sp_test(sim, full_slots_defer_accept) {
  fixture_t fx = sp_zero;
  server_start(t, &fx, router(), (server_opts_t) { .max_conns = 1 });

  client_t a = client_connect(&fx);
  sp_must_eq(t, a.err, SP_OK);
  client_send(&fx, &a, "GET /hello HTTP/1.1\r\n\r\n");
  sp_expect_ne(t, sp_str_find(client_recv(&fx, &a), sp_str_lit("200 OK")), SP_STR_NO_MATCH);

  client_t b = client_connect(&fx);
  sp_must_eq(t, b.err, SP_OK);
  client_send(&fx, &b, "GET /hello HTTP/1.1\r\n\r\n");
  sp_expect_eq(t, client_recv(&fx, &b).len, 0u);
  sp_expect_eq(t, b.eof, false);

  client_close(&a);
  server_run(&fx);

  sp_expect_ne(t, sp_str_find(client_recv(&fx, &b), sp_str_lit("200 OK")), SP_STR_NO_MATCH);

  client_close(&b);
  server_stop(&fx);
  return SP_OK;
}
