#include "http.h"

typedef struct {
  const c8* proxy;
} expect_t;

typedef struct {
  const c8* name;
  const c8* url;
  const c8* http_proxy;
  const c8* https_proxy;
  const c8* all_proxy;
  const c8* no_proxy;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "https_picks_https_proxy",
    .url = "https://api.example.com/x",
    .http_proxy = "http://proxy:3128",
    .https_proxy = "http://sproxy:3128",
    .all_proxy = "http://aproxy:1080",
    .expect = { "http://sproxy:3128" },
  },
  {
    .name = "http_picks_http_proxy",
    .url = "http://api.example.com/x",
    .http_proxy = "http://proxy:3128",
    .https_proxy = "http://sproxy:3128",
    .all_proxy = "http://aproxy:1080",
    .expect = { "http://proxy:3128" },
  },
  {
    .name = "https_falls_back_all_proxy",
    .url = "https://api.example.com/x",
    .http_proxy = "http://proxy:3128",
    .all_proxy = "http://aproxy:1080",
    .expect = { "http://aproxy:1080" },
  },
  {
    .name = "no_proxies_configured",
    .url = "https://api.example.com/x",
  },
  {
    .name = "no_proxy_suffix",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "example.com",
  },
  {
    .name = "no_proxy_leading_dot",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = ".example.com",
  },
  {
    .name = "no_proxy_case_insensitive",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "API.EXAMPLE.COM",
  },
  {
    .name = "no_proxy_list",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "other.com, example.com",
  },
  {
    .name = "no_proxy_star",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "*",
  },
  {
    .name = "no_proxy_not_suffix",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "notexample.com",
    .expect = { "http://sproxy:3128" },
  },
  {
    .name = "no_proxy_partial_label",
    .url = "https://api.example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "ample.com",
    .expect = { "http://sproxy:3128" },
  },
  {
    .name = "no_proxy_child_of_host",
    .url = "https://example.com/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "api.example.com",
    .expect = { "http://sproxy:3128" },
  },
  {
    .name = "no_proxy_ipv6_bare",
    .url = "https://[::1]/x",
    .https_proxy = "http://sproxy:3128",
    .no_proxy = "::1",
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_url_t url = sp_zero;
  sp_must(t, sp_http_url_parse(sp_cstr_as_str(c->url), &url));

  sp_str_t proxy = sp_http_proxy_pick(url,
    sp_cstr_as_str(c->http_proxy),
    sp_cstr_as_str(c->https_proxy),
    sp_cstr_as_str(c->all_proxy),
    sp_cstr_as_str(c->no_proxy));
  sp_expect_str_eq(t, proxy, sp_cstr_as_str(c->expect.proxy));
  return SP_OK;
}

sp_test_each_fn(http, proxy, test_t, tests, run);
