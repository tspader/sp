#define SP_HTTP_IMPLEMENTATION
#include "sp/sp_http.h"
#include "sp/sp_test.h"

#if !defined(SP_WIN32)
#include <signal.h>
#endif

#define TLS_MAX_STEPS 4

typedef struct {
  const c8* send;
  u32       delay_ms;
} step_t;

typedef struct {
  sp_http_error_t err;
  s32             status;
  const c8*       body;
} expect_t;

typedef struct {
  const c8* name;
  bool      untrusted;       // the client does not anchor the server certificate
  bool      no_close_notify; // slam the connection shut after playing the script
  bool      proxy;           // tunnel through the mock server with a plaintext CONNECT preamble
  const c8* connect_reply;   // reply to CONNECT; defaults to 200
  u32       io_timeout_ms;
  step_t    steps [TLS_MAX_STEPS];
  expect_t  expect;
  const c8* captured [2];    // substrings that must appear in requests the server received
} test_t;

static const test_t tests [] = {
  {
    .name = "trusted",
    .steps = { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } },
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "untrusted",
    .untrusted = true,
    .steps = { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } },
    .expect = { .err = SP_HTTP_ERR_UNTRUSTED },
  },
  {
    .name = "eof_body_close_notify",
    .steps = { { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } },
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "eof_body_no_close_notify",
    .no_close_notify = true,
    .steps = { { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } },
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "timeout",
    .io_timeout_ms = 120,
    .steps = { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", .delay_ms = 1000 } },
    .expect = { .err = SP_HTTP_ERR_TIMEOUT },
  },
  {
    .name = "proxy_connect",
    .proxy = true,
    .steps = { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } },
    .expect = { .status = 200, .body = "hello" },
    .captured = { "CONNECT 127.0.0.1:", "GET / HTTP/1.1" },
  },
  {
    .name = "proxy_connect_refused",
    .proxy = true,
    .connect_reply = "HTTP/1.1 403 Forbidden\r\n\r\n",
    .expect = { .err = SP_HTTP_ERR_PROXY },
  },
};

static const c8* tls_cert =
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

static const c8* tls_key =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgkOe51G2MRpZ8kyVN\n"
  "tWv2/RKrkE9WfLCS4zbMvdlNAUOhRANCAAQ2Hl0cVbbPLuko5otFB3zmPXuP0Lpx\n"
  "11IBhV1NM8Zw6kl46p9Qzc/rljXgguMNSYS3HV1wGDqZ+PON+S5OO/tU\n"
  "-----END PRIVATE KEY-----\n";

typedef struct {
  mbedtls_net_context      listen;
  sp_atomic_s32_t          stop;
  bool                     no_close_notify;
  bool                     proxy;
  const c8*                connect_reply;
  const step_t*            steps;
  c8                       captured [8192];
  u32                      captured_len;
  mbedtls_ssl_config       conf;
  mbedtls_x509_crt         crt;
  mbedtls_pk_context       pk;
  mbedtls_entropy_context  entropy;
  mbedtls_ctr_drbg_context drbg;
} server_t;

static bool mock_send(mbedtls_ssl_context* ssl, mbedtls_net_context* net, const u8* data, u32 len) {
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

static bool mock_read_request(server_t* server, mbedtls_ssl_context* ssl, mbedtls_net_context* net) {
  c8 buf [8192];
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

static void mock_play(mbedtls_ssl_context* ssl, const step_t* steps) {
  sp_for(it, TLS_MAX_STEPS) {
    step_t step = steps[it];
    if (!step.send) break;
    if (step.delay_ms) sp_sleep_ms((f64)step.delay_ms);
    if (!mock_send(ssl, SP_NULLPTR, (const u8*)step.send, sp_cstr_len(step.send))) return;
  }
}

static s32 server_thread(void* userdata) {
  server_t* server = (server_t*)userdata;
  for (;;) {
    mbedtls_net_context client;
    mbedtls_net_init(&client);
    if (mbedtls_net_accept(&server->listen, &client, SP_NULLPTR, 0, SP_NULLPTR) != 0) break;
    if (sp_atomic_s32_load(&server->stop, SP_ATOMIC_SEQ_CST)) {
      mbedtls_net_free(&client);
      break;
    }

    bool ok = true;
    if (server->proxy) {
      ok = mock_read_request(server, SP_NULLPTR, &client);
      if (ok) {
        const c8* reply = server->connect_reply ? server->connect_reply : "HTTP/1.1 200 Connection established\r\n\r\n";
        ok = mock_send(SP_NULLPTR, &client, (const u8*)reply, sp_cstr_len(reply));
      }
    }

    mbedtls_ssl_context ssl;
    mbedtls_ssl_init(&ssl);
    if (ok) {
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

    if (ok) ok = mock_read_request(server, &ssl, &client);
    if (ok) mock_play(&ssl, server->steps);
    if (ok && !server->no_close_notify) mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_net_free(&client);
  }
  return 0;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
#if !defined(SP_WIN32)
  signal(SIGPIPE, SIG_IGN);
#endif

  server_t server = sp_zero;
  mbedtls_net_init(&server.listen);
  server.no_close_notify = c->no_close_notify;
  server.proxy = c->proxy;
  server.connect_reply = c->connect_reply;
  server.steps = c->steps;

  sp_must_eq(t, mbedtls_net_bind(&server.listen, "127.0.0.1", "0", MBEDTLS_NET_PROTO_TCP), 0);
  u16 port_value = 0;
  sp_must_ok(t, sp_sys_socket_local_port((sp_sys_socket_t)server.listen.fd, &port_value));
  const c8* port = sp_str_to_cstr(mem, sp_fmt(mem, "{}", sp_fmt_uint(port_value)).value);

  mbedtls_x509_crt_init(&server.crt);
  mbedtls_pk_init(&server.pk);
  mbedtls_entropy_init(&server.entropy);
  mbedtls_ctr_drbg_init(&server.drbg);
  mbedtls_ssl_config_init(&server.conf);
  sp_must_eq(t, mbedtls_x509_crt_parse(&server.crt, (const unsigned char*)tls_cert, sp_cstr_len(tls_cert) + 1), 0);
  sp_must_eq(t, mbedtls_ctr_drbg_seed(&server.drbg, mbedtls_entropy_func, &server.entropy, SP_NULLPTR, 0), 0);
  sp_must_eq(t, mbedtls_pk_parse_key(&server.pk, (const unsigned char*)tls_key, sp_cstr_len(tls_key) + 1, SP_NULLPTR, 0, mbedtls_ctr_drbg_random, &server.drbg), 0);
  sp_must_eq(t, mbedtls_ssl_config_defaults(&server.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT), 0);
  mbedtls_ssl_conf_rng(&server.conf, mbedtls_ctr_drbg_random, &server.drbg);
  sp_must_eq(t, mbedtls_ssl_conf_own_cert(&server.conf, &server.crt, &server.pk), 0);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  trust.backend = SP_TLS_BACKEND_ANCHORS;
  if (!c->untrusted) {
    sp_must_eq(t, mbedtls_x509_crt_parse(trust.anchors, (const unsigned char*)tls_cert, sp_cstr_len(tls_cert) + 1), 0);
  }
  sp_tls_mbedtls_t client = sp_zero;
  sp_tls_mbedtls_init(&client, &trust);

  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, server_thread, &server);

  sp_io_dyn_mem_writer_t body = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &body);

  sp_http_request_t request = {
    .url    = sp_fmt(mem, "https://127.0.0.1:{}/", sp_fmt_cstr(port)).value,
    .tls    = &client.base,
    .sink   = &body.base,
    .proxy  = c->proxy ? sp_fmt(mem, "127.0.0.1:{}", sp_fmt_cstr(port)).value : sp_str_lit(""),
    .no_proxy = !c->proxy, // isolate the suite from proxies in the developer's environment
    .io_timeout_ms = c->io_timeout_ms,
  };

  sp_http_response_t response = sp_zero;
  sp_http_error_t err = sp_http_fetch(mem, request, &response);

  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (c->expect.status) sp_expect_eq(t, response.status, c->expect.status);
  if (c->expect.body) sp_expect_str_eq_c(t, sp_io_dyn_mem_writer_as_str(&body), c->expect.body);

  sp_atomic_s32_store(&server.stop, 1, SP_ATOMIC_SEQ_CST);
  mbedtls_net_context poke;
  mbedtls_net_init(&poke);
  mbedtls_net_connect(&poke, "127.0.0.1", port, MBEDTLS_NET_PROTO_TCP);
  mbedtls_net_free(&poke);
  sp_thread_join(&thread);

  sp_str_t captured = sp_str(server.captured, server.captured_len);
  sp_carr_for(c->captured, it) {
    if (!c->captured[it]) continue;
    sp_test_kv_c(t, "needle", c->captured[it]);
    sp_expect(t, sp_str_contains(captured, sp_cstr_as_str(c->captured[it])));
  }
  sp_test_kv_clear(t, "needle");

  mbedtls_net_free(&server.listen);
  mbedtls_ssl_config_free(&server.conf);
  mbedtls_pk_free(&server.pk);
  mbedtls_x509_crt_free(&server.crt);
  mbedtls_ctr_drbg_free(&server.drbg);
  mbedtls_entropy_free(&server.entropy);
  sp_tls_trust_free(&trust);
  return SP_OK;
}

sp_test_each_fn(tls, fetch, test_t, tests, run);
