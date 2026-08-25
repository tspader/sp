#include "http.h"

typedef struct {
  s32       status;
  const c8* body;
  const c8* allow;
} expect_t;

typedef struct {
  const c8*        name;
  sp_http_method_t method;
  const c8*        path;
  const c8*        query;
  expect_t         expect;
} test_t;

static sp_http_reply_t on_root(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("R"));
}

static sp_http_reply_t on_act(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(201, sp_str_lit("A"));
}

static sp_http_reply_t on_echo(sp_http_ctx_t* c) {
  return sp_http_reply_text(200, c->path);
}

static sp_http_reply_t on_user(sp_http_ctx_t* c) {
  return sp_http_reply_text(200, sp_cstr_as_str(sp_cast(const c8*, c->user_data)));
}

static sp_http_reply_t on_q(sp_http_ctx_t* c) {
  return sp_http_reply_text(200, sp_http_ctx_query(c, "name"));
}

static sp_http_reply_t on_shadowed(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("S"));
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET,  "/",          on_root },
  { SP_HTTP_POST, "/act",       on_act },
  { SP_HTTP_GET,  "/user",      on_user },
  { SP_HTTP_GET,  "/q",         on_q },
  { SP_HTTP_GET,  "/static/*",  on_echo },
  { SP_HTTP_GET,  "/static/x",  on_shadowed },
};

static const test_t tests [] = {
  {
    .name = "exact_match",
    .path = "/",
    .expect = { .status = 200, .body = "R" },
  },
  {
    .name = "exact_match_with_method",
    .method = SP_HTTP_POST,
    .path = "/act",
    .expect = { .status = 201, .body = "A" },
  },
  {
    .name = "prefix_match",
    .path = "/static/a/b.js",
    .expect = { .status = 200, .body = "/static/a/b.js" },
  },
  {
    .name = "prefix_matches_bare_prefix",
    .path = "/static/",
    .expect = { .status = 200, .body = "/static/" },
  },
  {
    .name = "prefix_does_not_match_without_slash",
    .path = "/staticx",
    .expect = { .status = 404 },
  },
  {
    .name = "prefix_requires_slash",
    .path = "/static",
    .expect = { .status = 404 },
  },
  {
    .name = "head_falls_back_to_get",
    .method = SP_HTTP_HEAD,
    .path = "/",
    .expect = { .status = 200, .body = "R" },
  },
  {
    .name = "query_value_decoded",
    .path = "/q",
    .query = "name=a%20b&x=1",
    .expect = { .status = 200, .body = "a b" },
  },
  {
    .name = "first_route_wins",
    .path = "/static/x",
    .expect = { .status = 200, .body = "/static/x" },
  },
  {
    .name = "no_route",
    .path = "/nope",
    .expect = { .status = 404 },
  },
  {
    .name = "wrong_method",
    .path = "/act",
    .expect = { .status = 405, .allow = "POST" },
  },
  {
    .name = "user_data_reaches_handler",
    .path = "/user",
    .expect = { .status = 200, .body = "U" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_router_t router = {
    .routes = routes,
    .count = sp_carr_len(routes),
    .user_data = (void*)"U",
  };
  sp_http_ctx_t ctx = {
    .mem = sp_test_arena(t),
    .method = c->method,
    .path = sp_cstr_as_str(c->path),
    .query = c->query ? sp_cstr_as_str(c->query) : sp_zero_s(sp_str_t),
  };
  sp_http_reply_t reply = sp_http_route(&router, &ctx);
  sp_expect_eq(t, reply.status, c->expect.status);
  if (c->expect.body) sp_expect_str_eq_c(t, reply.body, c->expect.body);
  if (c->expect.allow) {
    sp_must_eq(t, reply.num_headers, 1u);
    sp_expect_str_eq_c(t, reply.headers[0].name, "Allow");
    sp_expect_str_eq_c(t, reply.headers[0].value, c->expect.allow);
  }
  return SP_OK;
}

sp_test_each_fn(http, route, test_t, tests, run);
