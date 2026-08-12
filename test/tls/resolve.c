#include "tls.h"

typedef struct {
  const c8* url;
} expect_t;

typedef struct {
  const c8* name;
  const c8* base;
  const c8* location;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "absolute",
    .base = "https://example.com/a/b",
    .location = "https://other.com/x",
    .expect = { "https://other.com/x" },
  },
  {
    .name = "root_relative",
    .base = "https://example.com/a/b",
    .location = "/x",
    .expect = { "https://example.com:443/x" },
  },
  {
    .name = "sibling_relative",
    .base = "https://example.com/a/b",
    .location = "x",
    .expect = { "https://example.com:443/a/x" },
  },
  {
    .name = "no_path",
    .base = "https://example.com",
    .location = "x",
    .expect = { "https://example.com:443/x" },
  },
  {
    .name = "trailing_slash_dir",
    .base = "http://example.com:8080/a/",
    .location = "b",
    .expect = { "http://example.com:8080/a/b" },
  },
  {
    .name = "ipv6_base",
    .base = "https://[::1]:8443/a",
    .location = "/b",
    .expect = { "https://[::1]:8443/b" },
  },
  {
    .name = "query_holds_absolute_url",
    .base = "https://example.com/a/b",
    .location = "/login?next=https://other.com/x",
    .expect = { "https://example.com:443/login?next=https://other.com/x" },
  },
  {
    .name = "relative_query_url",
    .base = "https://example.com/a/b",
    .location = "x?u=https://y.com/",
    .expect = { "https://example.com:443/a/x?u=https://y.com/" },
  },
  {
    .name = "protocol_relative_https",
    .base = "https://example.com/a/b",
    .location = "//cdn.example.com/x",
    .expect = { "https://cdn.example.com/x" },
  },
  {
    .name = "protocol_relative_http",
    .base = "http://example.com/a/b",
    .location = "//cdn.example.com/x",
    .expect = { "http://cdn.example.com/x" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_url_t base = sp_zero;
  sp_must(t, sp_http_url_parse(sp_cstr_as_str(c->base), &base));

  sp_str_t resolved = sp_http_resolve_url(sp_test_arena(t), base, sp_cstr_as_str(c->location));
  sp_expect_str_eq_c(t, resolved, c->expect.url);
  return SP_OK;
}

sp_test_each_fn(tls, resolve, test_t, tests, run);
