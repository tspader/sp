#include "tls.h"

typedef struct {
  bool      ok;
  const c8* scheme;
  const c8* host;
  const c8* port;
  const c8* path;
  bool      tls;
} expect_t;

typedef struct {
  const c8* name;
  const c8* url;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  { .name = "https_default_port",       .url = "https://example.com",              .expect = { true, "https", "example.com", "443", "/", true } },
  { .name = "http_default_port",        .url = "http://example.com",               .expect = { true, "http", "example.com", "80", "/", false } },
  { .name = "bare_host_defaults_https", .url = "example.com",                      .expect = { true, "https", "example.com", "443", "/", true } },
  { .name = "scheme_case_preserved",    .url = "HTTPS://EXAMPLE.com/x",            .expect = { true, "HTTPS", "EXAMPLE.com", "443", "/x", true } },
  { .name = "port_path_query",          .url = "https://example.com:8443/a/b?q=1", .expect = { true, "https", "example.com", "8443", "/a/b?q=1", true } },
  { .name = "fragment_stripped",        .url = "https://example.com/x#frag",       .expect = { true, "https", "example.com", "443", "/x", true } },
  { .name = "fragment_only",            .url = "https://example.com#frag",         .expect = { true, "https", "example.com", "443", "/", true } },
  { .name = "host_dash_underscore",     .url = "https://a-b.c_d.example.com/",     .expect = { true, "https", "a-b.c_d.example.com", "443", "/", true } },
  { .name = "ipv4_host_port",           .url = "https://127.0.0.1:8080/x",         .expect = { true, "https", "127.0.0.1", "8080", "/x", true } },
  { .name = "query_no_path",            .url = "https://example.com?a=b",          .expect = { true, "https", "example.com", "443", "?a=b", true } },
  { .name = "query_no_path_port",       .url = "https://example.com:8443?a=b",     .expect = { true, "https", "example.com", "8443", "?a=b", true } },
  { .name = "query_no_path_ipv6",       .url = "https://[::1]?a=b",                .expect = { true, "https", "[::1]", "443", "?a=b", true } },
  { .name = "ipv6_loopback",            .url = "https://[::1]/x",                  .expect = { true, "https", "[::1]", "443", "/x", true } },
  { .name = "ipv6_port",                .url = "https://[::1]:8443/x",             .expect = { true, "https", "[::1]", "8443", "/x", true } },
  { .name = "ipv6_full",                .url = "http://[2001:db8::1]/",            .expect = { true, "http", "[2001:db8::1]", "80", "/", false } },
  { .name = "ipv6_v4_mapped",           .url = "https://[::ffff:1.2.3.4]/",        .expect = { true, "https", "[::ffff:1.2.3.4]", "443", "/", true } },
  { .name = "ipv6_unclosed",            .url = "https://[::1" },
  { .name = "ipv6_junk_after_bracket",  .url = "https://[::1]x/" },
  { .name = "ipv6_bad_char",            .url = "https://[bad!]/" },
  { .name = "empty",                    .url = "" },
  { .name = "ftp_scheme",               .url = "ftp://example.com" },
  { .name = "no_host",                  .url = "https://" },
  { .name = "userinfo",                 .url = "https://user@example.com/" },
  { .name = "userinfo_password",        .url = "https://user:pass@example.com/" },
  { .name = "port_empty",               .url = "https://example.com:/" },
  { .name = "port_zero",                .url = "https://example.com:0/" },
  { .name = "port_too_big",             .url = "https://example.com:65536/" },
  { .name = "port_junk",                .url = "https://example.com:4b3/" },
  { .name = "port_double_colon",        .url = "https://example.com:443:80/" },
  { .name = "host_space",               .url = "https://exa mple.com/" },
  { .name = "path_cr",                  .url = "https://example.com/a\rb" },
  { .name = "path_lf",                  .url = "https://example.com/a\nb" },
  { .name = "path_space",               .url = "https://example.com/a b" },
  { .name = "host_crlf",                .url = "https://example.com\r\n/" },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_url_t url = sp_zero;
  bool ok = sp_http_url_parse(sp_cstr_as_str(c->url), &url);
  sp_expect_eq(t, ok, c->expect.ok);
  if (!ok || !c->expect.ok) return SP_OK;

  sp_expect_str_eq_c(t, url.scheme, c->expect.scheme);
  sp_expect_str_eq_c(t, url.host, c->expect.host);
  sp_expect_str_eq_c(t, url.port, c->expect.port);
  sp_expect_str_eq_c(t, url.path, c->expect.path);
  sp_expect_eq(t, url.tls, c->expect.tls);
  return SP_OK;
}

sp_test_each_fn(tls, url, test_t, tests, run);
