#include "sp/sp_tls.h"
#include "test.h"

#include "utest.h"

#if defined(SP_TLS_WITH_MBEDTLS) && !defined(SP_WIN32)
#include <signal.h>
#endif

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
    { "https://example.com?a=b",          { true, "https", "example.com", "443", "?a=b", true } },
    { "https://example.com:8443?a=b",     { true, "https", "example.com", "8443", "?a=b", true } },
    { "https://[::1]?a=b",                { true, "https", "[::1]", "443", "?a=b", true } },
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
    { "https://example.com/a/b", "/login?next=https://other.com/x", "https://example.com:443/login?next=https://other.com/x" },
    { "https://example.com/a/b", "x?u=https://y.com/",  "https://example.com:443/a/x?u=https://y.com/" },
    { "https://example.com/a/b", "//cdn.example.com/x", "https://cdn.example.com/x" },
    { "http://example.com/a/b",  "//cdn.example.com/x", "http://cdn.example.com/x" },
  };
  run_resolve_tests(utest_result, ut.mem.arena, tests, sp_carr_len(tests));
}

UTEST_F(tls, no_proxy_match) {
  EXPECT_TRUE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("example.com")));
  EXPECT_TRUE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit(".example.com")));
  EXPECT_TRUE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("API.EXAMPLE.COM")));
  EXPECT_TRUE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("other.com, example.com")));
  EXPECT_TRUE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("*")));
  EXPECT_FALSE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("")));
  EXPECT_FALSE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("notexample.com")));
  EXPECT_FALSE(sp_http_no_proxy_match(sp_str_lit("api.example.com"), sp_str_lit("ample.com")));
  EXPECT_FALSE(sp_http_no_proxy_match(sp_str_lit("example.com"), sp_str_lit("api.example.com")));
}

UTEST_F(tls, proxy_pick) {
  sp_http_url_t https_url = sp_zero;
  sp_http_url_t http_url = sp_zero;
  EXPECT_TRUE(sp_http_url_parse(sp_str_lit("https://api.example.com/x"), &https_url));
  EXPECT_TRUE(sp_http_url_parse(sp_str_lit("http://api.example.com/x"), &http_url));

  sp_str_t http_p  = sp_str_lit("http://proxy:3128");
  sp_str_t https_p = sp_str_lit("http://sproxy:3128");
  sp_str_t all_p   = sp_str_lit("http://aproxy:1080");
  sp_str_t none    = sp_str_lit("");

  EXPECT_TRUE(sp_str_equal(sp_http_proxy_pick(https_url, http_p, https_p, all_p, none), https_p));
  EXPECT_TRUE(sp_str_equal(sp_http_proxy_pick(http_url, http_p, https_p, all_p, none), http_p));
  EXPECT_TRUE(sp_str_equal(sp_http_proxy_pick(https_url, http_p, none, all_p, none), all_p));
  EXPECT_TRUE(sp_str_empty(sp_http_proxy_pick(https_url, none, none, none, none)));
  EXPECT_TRUE(sp_str_empty(sp_http_proxy_pick(https_url, http_p, https_p, all_p, sp_str_lit("example.com"))));
}

UTEST_F(tls, host_bare) {
  EXPECT_TRUE(sp_str_equal_cstr(sp_http_host_bare(sp_str_lit("[::1]")), "::1"));
  EXPECT_TRUE(sp_str_equal_cstr(sp_http_host_bare(sp_str_lit("example.com")), "example.com"));
}

#if defined(SP_TLS_WITH_MBEDTLS)

static const c8* tls_test_cert =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIBfjCCASWgAwIBAgIUDPglN4zQDNG5UTi2PtR5HoWKNbkwCgYIKoZIzj0EAwIw\n"
  "FDESMBAGA1UEAwwJMTI3LjAuMC4xMCAXDTI2MDcwMTIyNTA1NloYDzIxMjYwNjA3\n"
  "MjI1MDU2WjAUMRIwEAYDVQQDDAkxMjcuMC4wLjEwWTATBgcqhkjOPQIBBggqhkjO\n"
  "PQMBBwNCAAQ2Hl0cVbbPLuko5otFB3zmPXuP0Lpx11IBhV1NM8Zw6kl46p9Qzc/r\n"
  "ljXgguMNSYS3HV1wGDqZ+PON+S5OO/tUo1MwUTAdBgNVHQ4EFgQUZs7KCxZ9MnDz\n"
  "rNhwgCN4rZrqHjgwHwYDVR0jBBgwFoAUZs7KCxZ9MnDzrNhwgCN4rZrqHjgwDwYD\n"
  "VR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNHADBEAiA2wjJs70BXB/E2UgJFteWi\n"
  "KHJg0TfhR8GmnwycFLKQxwIgfuDjz1eFI6NFseCI92HdSOohKe9uTWjgfYyOw/lH\n"
  "7uk=\n"
  "-----END CERTIFICATE-----\n";

static const c8* tls_test_key =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgkOe51G2MRpZ8kyVN\n"
  "tWv2/RKrkE9WfLCS4zbMvdlNAUOhRANCAAQ2Hl0cVbbPLuko5otFB3zmPXuP0Lpx\n"
  "11IBhV1NM8Zw6kl46p9Qzc/rljXgguMNSYS3HV1wGDqZ+PON+S5OO/tU\n"
  "-----END PRIVATE KEY-----\n";

#define TLS_MOCK_STEPS 4
#define TLS_MOCK_SCRIPTS 2

typedef struct {
  const c8* send;
  u32       repeat;
  u32       pad;
  u32       delay_ms;
} tls_mock_step_t;

typedef struct {
  tls_mock_step_t steps[TLS_MOCK_STEPS];
} tls_mock_script_t;

typedef struct {
  sp_tls_error_t err;
  s32            status;
  const c8*      body;
} fetch_expect_t;

typedef struct {
  bool              tls;
  bool              untrusted;
  bool              no_close_notify; // slam the connection shut after playing the script
  bool              proxy;         // send the request through the mock server as a proxy
  bool              proxy_connect; // the mock server expects a plaintext CONNECT preamble
  const c8*         connect_reply; // reply to CONNECT; defaults to 200
  const c8*         url;           // fetch url; defaults to the mock server
  const c8*         path;          // appended to the mock server url; defaults to /
  u32               connect_timeout_ms;
  u32               io_timeout_ms;
  tls_mock_script_t scripts[TLS_MOCK_SCRIPTS];
  fetch_expect_t    expect;
  const c8*         captured[2];   // substrings that must appear in requests the server received
} fetch_test_t;

typedef struct {
  mbedtls_net_context      listen;
  sp_atomic_s32_t          stop;
  bool                     tls;
  bool                     no_close_notify;
  bool                     proxy_connect;
  const c8*                connect_reply;
  const tls_mock_script_t* scripts;
  u32                      script_count;
  c8                       captured[8192];
  u32                      captured_len;
  mbedtls_ssl_config       conf;
  mbedtls_x509_crt         crt;
  mbedtls_pk_context       pk;
  mbedtls_entropy_context  entropy;
  mbedtls_ctr_drbg_context drbg;
} tls_mock_server_t;

static bool tls_mock_send(mbedtls_ssl_context* ssl, mbedtls_net_context* net, const u8* data, u32 len) {
  u32 sent = 0;
  while (sent < len) {
    s32 n = ssl
      ? mbedtls_ssl_write(ssl, data + sent, len - sent)
      : mbedtls_net_send(net, data + sent, len - sent);
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
    if (n <= 0) return false;
    sent += (u32)n;
  }
  return true;
}

static bool tls_mock_read_request(tls_mock_server_t* server, mbedtls_ssl_context* ssl, mbedtls_net_context* net) {
  c8 buf[8192];
  u32 len = 0;
  bool ok = false;
  for (;;) {
    if (sp_str_find(sp_str(buf, len), sp_str_lit("\r\n\r\n")) != SP_STR_NO_MATCH) {
      ok = true;
      break;
    }
    if (len == sizeof(buf)) break;
    s32 n = ssl
      ? mbedtls_ssl_read(ssl, (u8*)buf + len, sizeof(buf) - len)
      : mbedtls_net_recv(net, (u8*)buf + len, sizeof(buf) - len);
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
    if (n <= 0) break;
    len += (u32)n;
  }
  u32 space = (u32)sizeof(server->captured) - server->captured_len;
  u32 take = len < space ? len : space;
  sp_mem_copy(server->captured + server->captured_len, buf, take);
  server->captured_len += take;
  return ok;
}

static void tls_mock_play(mbedtls_ssl_context* ssl, mbedtls_net_context* net, const tls_mock_script_t* script) {
  sp_carr_for(script->steps, it) {
    tls_mock_step_t step = script->steps[it];
    if (!step.send && !step.pad) break;
    u32 repeat = step.repeat ? step.repeat : 1;
    sp_for(r, repeat) {
      if (step.delay_ms) sp_sleep_ms((f64)step.delay_ms);
      if (step.send) {
        if (!tls_mock_send(ssl, net, (const u8*)step.send, sp_cstr_len(step.send))) return;
      }
      u8 chunk[512];
      sp_mem_fill_u8(chunk, sizeof(chunk), 'a');
      u32 pad = step.pad;
      while (pad > 0) {
        u32 take = pad < sizeof(chunk) ? pad : (u32)sizeof(chunk);
        if (!tls_mock_send(ssl, net, chunk, take)) return;
        pad -= take;
      }
    }
  }
}

static s32 tls_mock_server_thread(void* userdata) {
  tls_mock_server_t* server = (tls_mock_server_t*)userdata;
  u32 index = 0;
  for (;;) {
    mbedtls_net_context client;
    mbedtls_net_init(&client);
    if (mbedtls_net_accept(&server->listen, &client, SP_NULLPTR, 0, SP_NULLPTR) != 0) break;
    if (sp_atomic_s32_get(&server->stop)) {
      mbedtls_net_free(&client);
      break;
    }

    bool ok = true;
    if (server->proxy_connect) {
      ok = tls_mock_read_request(server, SP_NULLPTR, &client);
      if (ok) {
        const c8* reply = server->connect_reply ? server->connect_reply : "HTTP/1.1 200 Connection established\r\n\r\n";
        ok = tls_mock_send(SP_NULLPTR, &client, (const u8*)reply, sp_cstr_len(reply));
      }
    }

    mbedtls_ssl_context ssl;
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_context* sslp = server->tls ? &ssl : SP_NULLPTR;
    if (ok && server->tls) {
      ok = mbedtls_ssl_setup(&ssl, &server->conf) == 0;
      if (ok) {
        mbedtls_ssl_set_bio(&ssl, &client, mbedtls_net_send, mbedtls_net_recv, SP_NULLPTR);
        s32 rc;
        while ((rc = mbedtls_ssl_handshake(&ssl)) != 0) {
          if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ok = false;
            break;
          }
        }
      }
    }

    if (ok) ok = tls_mock_read_request(server, sslp, &client);
    if (ok) {
      u32 which = index < server->script_count ? index : server->script_count - 1;
      tls_mock_play(sslp, &client, &server->scripts[which]);
    }
    if (server->tls && ok && !server->no_close_notify) mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_net_free(&client);
    index++;
  }
  return 0;
}

void run_fetch_test(s32* utest_result, sp_mem_t mem, fetch_test_t t) {
#if !defined(SP_WIN32)
  signal(SIGPIPE, SIG_IGN);
#endif

  tls_mock_server_t server = sp_zero;
  mbedtls_net_init(&server.listen);
  server.tls = t.tls;
  server.no_close_notify = t.no_close_notify;
  server.proxy_connect = t.proxy_connect;
  server.connect_reply = t.connect_reply;
  server.scripts = t.scripts;
  server.script_count = 0;
  sp_carr_for(t.scripts, it) {
    if (t.scripts[it].steps[0].send || t.scripts[it].steps[0].pad) server.script_count++;
  }
  if (!server.script_count) server.script_count = 1;

  c8 port[8] = sp_zero;
  bool bound = false;
  sp_for(attempt, 64) {
    sp_str_t formatted = sp_fmt(mem, "{}", sp_fmt_uint(42600 + attempt)).value;
    sp_cstr_copy_to_n(formatted.data, formatted.len, port, sizeof(port));
    if (mbedtls_net_bind(&server.listen, "127.0.0.1", port, MBEDTLS_NET_PROTO_TCP) == 0) {
      bound = true;
      break;
    }
  }
  EXPECT_TRUE(bound);
  if (!bound) return;

  if (t.tls) {
    mbedtls_x509_crt_init(&server.crt);
    mbedtls_pk_init(&server.pk);
    mbedtls_entropy_init(&server.entropy);
    mbedtls_ctr_drbg_init(&server.drbg);
    mbedtls_ssl_config_init(&server.conf);
    EXPECT_EQ(mbedtls_x509_crt_parse(&server.crt, (const unsigned char*)tls_test_cert, sp_cstr_len(tls_test_cert) + 1), 0);
    EXPECT_EQ(mbedtls_ctr_drbg_seed(&server.drbg, mbedtls_entropy_func, &server.entropy, SP_NULLPTR, 0), 0);
    EXPECT_EQ(mbedtls_pk_parse_key(&server.pk, (const unsigned char*)tls_test_key, sp_cstr_len(tls_test_key) + 1, SP_NULLPTR, 0, mbedtls_ctr_drbg_random, &server.drbg), 0);
    EXPECT_EQ(mbedtls_ssl_config_defaults(&server.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT), 0);
    mbedtls_ssl_conf_rng(&server.conf, mbedtls_ctr_drbg_random, &server.drbg);
    EXPECT_EQ(mbedtls_ssl_conf_own_cert(&server.conf, &server.crt, &server.pk), 0);
  }

  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, tls_mock_server_thread, &server);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  trust.backend = SP_TLS_BACKEND_ANCHORS;
  if (t.tls && !t.untrusted) {
    EXPECT_EQ(mbedtls_x509_crt_parse((mbedtls_x509_crt*)trust.anchors, (const unsigned char*)tls_test_cert, sp_cstr_len(tls_test_cert) + 1), 0);
  }

  sp_io_dyn_mem_writer_t body = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &body);

  sp_str_t url = t.url
    ? sp_cstr_as_str(t.url)
    : sp_fmt(mem, "{}://127.0.0.1:{}{}", sp_fmt_cstr(t.tls ? "https" : "http"), sp_fmt_cstr(port), sp_fmt_cstr(t.path ? t.path : "/")).value;
  sp_http_response_t response = sp_zero;
  sp_tls_error_t err = sp_http_fetch(mem, (sp_http_request_t) {
    .url   = url,
    .trust = &trust,
    .body  = &body.base,
    .proxy = t.proxy ? sp_fmt(mem, "127.0.0.1:{}", sp_fmt_cstr(port)).value : sp_str_lit(""),
    .no_proxy = !t.proxy, // isolate the suite from proxies in the developer's environment
    .connect_timeout_ms = t.connect_timeout_ms,
    .io_timeout_ms = t.io_timeout_ms,
  }, &response);

  EXPECT_EQ(err, t.expect.err);
  if (t.expect.status) EXPECT_EQ(response.status, t.expect.status);
  if (t.expect.body) EXPECT_TRUE(sp_str_equal_cstr(sp_io_dyn_mem_writer_as_str(&body), t.expect.body));

  sp_atomic_s32_set(&server.stop, 1);
  mbedtls_net_context poke;
  mbedtls_net_init(&poke);
  mbedtls_net_connect(&poke, "127.0.0.1", port, MBEDTLS_NET_PROTO_TCP);
  mbedtls_net_free(&poke);
  sp_thread_join(&thread);

  sp_carr_for(t.captured, it) {
    if (!t.captured[it]) continue;
    EXPECT_TRUE_MSG(sp_str_contains(sp_str(server.captured, server.captured_len), sp_cstr_as_str(t.captured[it])), t.captured[it]);
  }

  mbedtls_net_free(&server.listen);
  if (t.tls) {
    mbedtls_ssl_config_free(&server.conf);
    mbedtls_pk_free(&server.pk);
    mbedtls_x509_crt_free(&server.crt);
    mbedtls_ctr_drbg_free(&server.drbg);
    mbedtls_entropy_free(&server.entropy);
  }
  sp_tls_trust_free(&trust);
}

UTEST_F(tls, fetch_content_length) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello" } }}},
    .expect = { .status = 200, .body = "hello" },
  });
}

UTEST_F(tls, fetch_status_404) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nnot found" } }}},
    .expect = { .err = SP_TLS_ERR_STATUS, .status = 404, .body = "" },
  });
}

UTEST_F(tls, fetch_204_no_body) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 204 No Content\r\n\r\n" } }}},
    .expect = { .status = 204, .body = "" },
  });
}

UTEST_F(tls, fetch_early_hints) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 103 Early Hints\r\nLink: </s.css>; rel=preload\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }}},
    .expect = { .status = 200, .body = "hello" },
  });
}

UTEST_F(tls, fetch_interim_flood) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 100 Continue\r\n\r\n", .repeat = 20 } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_upgrade_101) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n\r\n" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_bad_status_line) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "ICY 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_conflicting_length) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Length: 9999\r\n\r\nhello" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_te_gzip) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: gzip\r\n\r\nblob" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_chunked) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n" } }}},
    .expect = { .status = 200, .body = "hello world" },
  });
}

UTEST_F(tls, fetch_chunked_bad_separator) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhelloXX\r\n0\r\n\r\n" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_huge_head) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nX-Pad: ", .pad = 96 * 1024 } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_crlf_location) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 302 Found\r\nLocation: /a\rSet-Cookie: pwn=1\r\n\r\n" } }}},
    .expect = { .err = SP_TLS_ERR_URL },
  });
}

UTEST_F(tls, fetch_redirect_userinfo) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 302 Found\r\nLocation: http://evil@127.0.0.1/x\r\n\r\n" } }}},
    .expect = { .err = SP_TLS_ERR_URL },
  });
}

UTEST_F(tls, fetch_redirect_loop) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 302 Found\r\nLocation: /loop\r\n\r\n" } }}},
    .expect = { .err = SP_TLS_ERR_REDIRECTS },
  });
}

UTEST_F(tls, fetch_redirect_follow) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {
      {{ { .send = "HTTP/1.1 302 Found\r\nLocation: /next\r\n\r\n" } }},
      {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    },
    .expect = { .status = 200, .body = "ok" },
  });
}

UTEST_F(tls, fetch_split_head) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{
      { .send = "HTTP/1.1 200 OK\r\nContent-Le" },
      { .send = "ngth: 5\r\n\r\nhello", .delay_ms = 30 },
    }}},
    .expect = { .status = 200, .body = "hello" },
  });
}

UTEST_F(tls, fetch_truncated_body) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nhello" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_query_no_path) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .path = "?a=b",
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }}},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /?a=b HTTP/1.1" },
  });
}

UTEST_F(tls, fetch_eof_body) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } }}},
    .expect = { .status = 200, .body = "hello" },
  });
}

UTEST_F(tls, fetch_eof_body_no_close_notify) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .no_close_notify = true,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } }}},
    .expect = { .err = SP_TLS_ERR_PROTOCOL },
  });
}

UTEST_F(tls, fetch_tls_trusted) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }}},
    .expect = { .status = 200, .body = "hello" },
  });
}

UTEST_F(tls, fetch_tls_untrusted) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .untrusted = true,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }}},
    .expect = { .err = SP_TLS_ERR_UNTRUSTED },
  });
}

UTEST_F(tls, fetch_redirect_query_url) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .scripts = {
      {{ { .send = "HTTP/1.1 302 Found\r\nLocation: /next?u=https://example.com/\r\n\r\n" } }},
      {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /next?u=https://example.com/ HTTP/1.1" },
  });
}

UTEST_F(tls, fetch_timeout_head) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .io_timeout_ms = 120,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", .delay_ms = 1000 } }}},
    .expect = { .err = SP_TLS_ERR_TIMEOUT },
  });
}

UTEST_F(tls, fetch_timeout_body) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .io_timeout_ms = 120,
    .scripts = {{{
      { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhe" },
      { .send = "llo", .delay_ms = 1000 },
    }}},
    .expect = { .err = SP_TLS_ERR_TIMEOUT },
  });
}

UTEST_F(tls, fetch_timeout_tls) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .io_timeout_ms = 120,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", .delay_ms = 1000 } }}},
    .expect = { .err = SP_TLS_ERR_TIMEOUT },
  });
}

UTEST_F(tls, fetch_proxy_absolute_form) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .proxy = true,
    .url = "http://example.test:8080/x",
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }}},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET http://example.test:8080/x HTTP/1.1", "Host: example.test:8080" },
  });
}

UTEST_F(tls, fetch_proxy_connect) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .proxy = true,
    .proxy_connect = true,
    .scripts = {{{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }}},
    .expect = { .status = 200, .body = "hello" },
    .captured = { "CONNECT 127.0.0.1:", "GET / HTTP/1.1" },
  });
}

UTEST_F(tls, fetch_proxy_connect_refused) {
  run_fetch_test(utest_result, ut.mem.arena, (fetch_test_t) {
    .tls = true,
    .proxy = true,
    .proxy_connect = true,
    .connect_reply = "HTTP/1.1 403 Forbidden\r\n\r\n",
    .expect = { .err = SP_TLS_ERR_PROXY },
  });
}

#endif
