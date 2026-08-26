#include "../http.h"

typedef struct {
  bool             ok;
  sp_http_method_t method;
  const c8*        wire;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  { .name = "get",       .input = "GET",    .expect = { .ok = true, .method = SP_HTTP_GET, .wire = "GET" } },
  { .name = "post",      .input = "POST",   .expect = { .ok = true, .method = SP_HTTP_POST, .wire = "POST" } },
  { .name = "put",       .input = "PUT",    .expect = { .ok = true, .method = SP_HTTP_PUT, .wire = "PUT" } },
  { .name = "patch",     .input = "PATCH",  .expect = { .ok = true, .method = SP_HTTP_PATCH, .wire = "PATCH" } },
  { .name = "delete",    .input = "DELETE", .expect = { .ok = true, .method = SP_HTTP_DELETE, .wire = "DELETE" } },
  { .name = "head",      .input = "HEAD",   .expect = { .ok = true, .method = SP_HTTP_HEAD, .wire = "HEAD" } },
  // methods are tokens and case-sensitive on the wire
  { .name = "lowercase", .input = "post" },
  { .name = "mixed",     .input = "Head" },
  { .name = "unknown",   .input = "TRACE" },
  { .name = "prefix",    .input = "GE" },
  { .name = "empty",     .input = "" },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_method_t method = c->expect.method == SP_HTTP_GET ? SP_HTTP_POST : SP_HTTP_GET;
  bool ok = sp_http_method_parse(sp_cstr_as_str(c->input), &method);
  sp_expect_eq(t, ok, c->expect.ok);
  if (!ok || !c->expect.ok) return SP_OK;

  sp_expect_eq(t, (s32)method, (s32)c->expect.method);
  sp_expect_str_eq_c(t, sp_http_method_name(method), c->expect.wire);
  return SP_OK;
}

sp_test_each_fn(http, method, test_t, tests, run);
