#include "sp/sp_tls.h"
#include "test.h"

#include "utest.h"

typedef struct {
  bool      ok;
  const c8* scheme;
  const c8* host;
  const c8* port;
  const c8* path;
  bool      tls;
} url_expect_t;

typedef struct {
  const c8*    url;
  url_expect_t expect;
} url_test_t;

void run_url_tests(s32* utest_result, const url_test_t* tests, u32 count) {
  sp_for(it, count) {
    url_test_t t = tests[it];
    sp_http_url_t url = sp_zero;
    bool ok = sp_http_url_parse(sp_cstr_as_str(t.url), &url);
    EXPECT_EQ_MSG(ok, t.expect.ok, t.url);
    if (!ok || !t.expect.ok) continue;
    EXPECT_TRUE_MSG(sp_str_equal_cstr(url.scheme, t.expect.scheme), t.url);
    EXPECT_TRUE_MSG(sp_str_equal_cstr(url.host, t.expect.host), t.url);
    EXPECT_TRUE_MSG(sp_str_equal_cstr(url.port, t.expect.port), t.url);
    EXPECT_TRUE_MSG(sp_str_equal_cstr(url.path, t.expect.path), t.url);
    EXPECT_EQ_MSG(url.tls, t.expect.tls, t.url);
  }
}

typedef struct {
  sp_tls_error_t err;
  s32            status;
  const c8*      location;
  bool           chunked;
  bool           has_length;
  u64            length;
} head_expect_t;

typedef struct {
  const c8*     head;
  head_expect_t expect;
} head_test_t;

void run_head_tests(s32* utest_result, const head_test_t* tests, u32 count) {
  sp_for(it, count) {
    head_test_t t = tests[it];
    sp_http_head_t head = sp_zero;
    sp_tls_error_t err = sp_http_parse_head(sp_cstr_as_str(t.head), &head);
    EXPECT_EQ_MSG(err, t.expect.err, t.head);
    if (err != SP_TLS_OK || t.expect.err != SP_TLS_OK) continue;
    EXPECT_EQ_MSG(head.status, t.expect.status, t.head);
    EXPECT_EQ_MSG(head.chunked, t.expect.chunked, t.head);
    EXPECT_EQ_MSG(head.has_length, t.expect.has_length, t.head);
    EXPECT_EQ_MSG(head.length, t.expect.length, t.head);
    if (t.expect.location) {
      EXPECT_TRUE_MSG(sp_str_equal_cstr(head.location, t.expect.location), t.head);
    }
    else {
      EXPECT_TRUE_MSG(sp_str_empty(head.location), t.head);
    }
  }
}

typedef struct {
  const c8* base;
  const c8* location;
  const c8* expect;
} resolve_test_t;

void run_resolve_tests(s32* utest_result, sp_mem_t mem, const resolve_test_t* tests, u32 count) {
  sp_for(it, count) {
    resolve_test_t t = tests[it];
    sp_http_url_t base = sp_zero;
    EXPECT_TRUE_MSG(sp_http_url_parse(sp_cstr_as_str(t.base), &base), t.base);
    sp_str_t resolved = sp_http_resolve_url(mem, base, sp_cstr_as_str(t.location));
    EXPECT_TRUE_MSG(sp_str_equal_cstr(resolved, t.expect), t.expect);
  }
}

SP_TEST_MAIN()

struct tls {
  sp_mem_tracking_t tracker;
  sp_mem_arena_t* arena;
  struct { sp_mem_t tracking; sp_mem_t arena; } mem;
};

UTEST_F_SETUP(tls) {
  sp_mem_tracking_init(&ut.tracker);
  ut.mem.tracking = sp_mem_tracking_as_allocator(&ut.tracker);
  ut.arena = sp_mem_arena_new(ut.mem.tracking);
  ut.mem.arena = sp_mem_arena_as_allocator(ut.arena);
}

UTEST_F_TEARDOWN(tls) {
  sp_mem_arena_destroy(ut.arena);
  EXPECT_TRUE(sp_mem_tracking_ok(&ut.tracker));
  sp_mem_tracking_deinit(&ut.tracker);
}

UTEST_F(tls, url_parse_basic) {
  url_test_t tests[] = {
    { "https://example.com",              { true, "https", "example.com", "443", "/", true } },
    { "http://example.com",               { true, "http", "example.com", "80", "/", false } },
    { "example.com",                      { true, "https", "example.com", "443", "/", true } },
    { "HTTPS://EXAMPLE.com/x",            { true, "HTTPS", "EXAMPLE.com", "443", "/x", true } },
    { "https://example.com:8443/a/b?q=1", { true, "https", "example.com", "8443", "/a/b?q=1", true } },
    { "https://example.com/x#frag",       { true, "https", "example.com", "443", "/x", true } },
    { "https://example.com#frag",         { true, "https", "example.com", "443", "/", true } },
    { "https://a-b.c_d.example.com/",     { true, "https", "a-b.c_d.example.com", "443", "/", true } },
    { "https://127.0.0.1:8080/x",         { true, "https", "127.0.0.1", "8080", "/x", true } },
  };
  run_url_tests(utest_result, tests, sp_carr_len(tests));
}

UTEST_F(tls, url_parse_ipv6) {
  url_test_t tests[] = {
    { "https://[::1]/x",           { true, "https", "[::1]", "443", "/x", true } },
    { "https://[::1]:8443/x",      { true, "https", "[::1]", "8443", "/x", true } },
    { "http://[2001:db8::1]/",     { true, "http", "[2001:db8::1]", "80", "/", false } },
    { "https://[::ffff:1.2.3.4]/", { true, "https", "[::ffff:1.2.3.4]", "443", "/", true } },
    { "https://[::1",              { false } },
    { "https://[::1]x/",           { false } },
    { "https://[bad!]/",           { false } },
  };
  run_url_tests(utest_result, tests, sp_carr_len(tests));
}

UTEST_F(tls, url_parse_rejects) {
  url_test_t tests[] = {
    { "",                                { false } },
    { "ftp://example.com",               { false } },
    { "https://",                        { false } },
    { "https://user@example.com/",       { false } },
    { "https://user:pass@example.com/",  { false } },
    { "https://example.com:/",           { false } },
    { "https://example.com:0/",          { false } },
    { "https://example.com:65536/",      { false } },
    { "https://example.com:4b3/",        { false } },
    { "https://example.com:443:80/",     { false } },
    { "https://exa mple.com/",           { false } },
    { "https://example.com/a\rb",        { false } },
    { "https://example.com/a\nb",        { false } },
    { "https://example.com/a b",         { false } },
    { "https://example.com\r\n/",        { false } },
  };
  run_url_tests(utest_result, tests, sp_carr_len(tests));
}

UTEST_F(tls, head_parse) {
  head_test_t tests[] = {
    { "HTTP/1.1 200 OK\r\nContent-Length: 12",
      { SP_TLS_OK, 200, SP_NULLPTR, false, true, 12 } },
    { "HTTP/1.1 200 OK\r\ncontent-length: 12\r\ncontent-length: 12",
      { SP_TLS_OK, 200, SP_NULLPTR, false, true, 12 } },
    { "HTTP/1.1 301 Moved Permanently\r\nLocation: https://example.com/new",
      { SP_TLS_OK, 301, "https://example.com/new" } },
    { "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked",
      { SP_TLS_OK, 200, SP_NULLPTR, true } },
    { "HTTP/1.1 200 OK\r\nTransfer-Encoding: GZIP, Chunked",
      { SP_TLS_OK, 200, SP_NULLPTR, true } },
    { "HTTP/1.1 204 No Content",
      { SP_TLS_OK, 204 } },
    { "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Length: 9",
      { SP_TLS_ERR_PROTOCOL } },
    { "HTTP/1.1 200 OK\r\nContent-Length: abc",
      { SP_TLS_ERR_PROTOCOL } },
    { "HTTP/1.1 200 OK\r\nTransfer-Encoding: gzip",
      { SP_TLS_ERR_PROTOCOL } },
    { "ICY 200 OK",
      { SP_TLS_ERR_PROTOCOL } },
    { "HTTP/1.1",
      { SP_TLS_ERR_PROTOCOL } },
    { "HTTP/1.1 abc OK",
      { SP_TLS_ERR_PROTOCOL } },
    { "",
      { SP_TLS_ERR_PROTOCOL } },
  };
  run_head_tests(utest_result, tests, sp_carr_len(tests));
}

UTEST_F(tls, resolve_url) {
  resolve_test_t tests[] = {
    { "https://example.com/a/b", "https://other.com/x", "https://other.com/x" },
    { "https://example.com/a/b", "/x",                  "https://example.com:443/x" },
    { "https://example.com/a/b", "x",                   "https://example.com:443/a/x" },
    { "https://example.com",     "x",                   "https://example.com:443/x" },
    { "http://example.com:8080/a/", "b",                "http://example.com:8080/a/b" },
    { "https://[::1]:8443/a",    "/b",                  "https://[::1]:8443/b" },
  };
  run_resolve_tests(utest_result, ut.mem.arena, tests, sp_carr_len(tests));
}

UTEST_F(tls, host_bare) {
  EXPECT_TRUE(sp_str_equal_cstr(sp_http_host_bare(sp_str_lit("[::1]")), "::1"));
  EXPECT_TRUE(sp_str_equal_cstr(sp_http_host_bare(sp_str_lit("example.com")), "example.com"));
}
