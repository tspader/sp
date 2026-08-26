#include "harness.h"

typedef struct {
  sp_http_stream_t* stream;
} app_t;

static sp_http_reply_t on_events(sp_http_ctx_t* c) {
  app_t* app = sp_cast(app_t*, c->user_data);
  app->stream = sp_http_ctx_stream(c);
  return sp_http_reply_stream(app->stream, sp_str_lit("text/event-stream"));
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET, "/events", on_events },
};

sp_test(sim, head_captured_stream_stays_closed) {
  app_t app = sp_zero;
  fixture_t fx = sp_zero;
  sp_http_router_t router = { .routes = routes, .count = sp_carr_len(routes), .user_data = &app };
  server_start(t, &fx, router, (server_opts_t) { .max_conns = 1 });

  client_t a = client_connect(&fx);
  sp_must_eq(t, a.err, SP_OK);
  client_send(&fx, &a, "HEAD /events HTTP/1.1\r\nConnection: close\r\n\r\n");
  client_recv(&fx, &a);

  sp_http_stream_t* stale = app.stream;
  app.stream = SP_NULLPTR;
  sp_must_eq(t, stale != SP_NULLPTR, true);
  sp_expect_eq(t, sp_http_stream_closed(stale), true);

  client_t b = client_connect(&fx);
  sp_must_eq(t, b.err, SP_OK);
  client_send(&fx, &b, "GET /events HTTP/1.1\r\n\r\n");
  sp_expect_eq(t, client_recv(&fx, &b).len, 0u);
  sp_expect_eq(t, app.stream == SP_NULLPTR, true);
  sp_expect_eq(t, sp_http_stream_closed(stale), true);
  if (!sp_http_stream_closed(stale)) {
    sp_io_write_str(&stale->base, sp_str_lit("INJECTED"), SP_NULLPTR);
  }

  sp_http_stream_close(stale);
  server_run(&fx);

  sp_str_t wire = client_recv(&fx, &b);
  sp_expect_ne(t, sp_str_find(wire, sp_str_lit("HTTP/1.1 200 OK")), SP_STR_NO_MATCH);
  sp_expect_eq(t, sp_str_find(wire, sp_str_lit("INJECTED")), SP_STR_NO_MATCH);
  sp_expect_eq(t, app.stream != SP_NULLPTR, true);

  if (app.stream) sp_http_stream_close(app.stream);
  server_run(&fx);
  client_close(&b);
  server_stop(&fx);
  return SP_OK;
}
