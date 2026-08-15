#define SP_HTTP_IMPLEMENTATION
#include "http.h"

#if !defined(SP_WIN32)
#include <signal.h>
#endif

#define FETCH_MAX_STEPS    4
#define FETCH_MAX_SCRIPTS  2
#define FETCH_MAX_HEADERS  4
#define FETCH_MAX_CAPTURED 2
#define FETCH_MAX_COUNTED  4

typedef enum {
  RESOLVE_SYSTEM,
  RESOLVE_LOCAL,
  RESOLVE_REFUSE,
} resolve_kind_t;

typedef struct {
  const c8* send;
  u32       repeat;
  u32       pad;
  u32       delay_ms;
} step_t;

typedef struct {
  const c8* name;
  const c8* value;
} header_t;

typedef struct {
  const c8* text;
  u32       n;
} counted_t;

typedef struct {
  sp_http_error_t err;
  s32            status;
  const c8*      body;
  const c8*      content_type;
  const c8*      location;
} expect_t;

typedef struct {
  const c8*        name;
  bool             tls;
  bool             untrusted;
  bool             no_close_notify; // slam the connection shut after playing the script
  bool             proxy;           // send the request through the mock server as a proxy
  bool             proxy_connect;   // the mock server expects a plaintext CONNECT preamble
  const c8*        connect_reply;   // reply to CONNECT; defaults to 200
  const c8*        url;             // fetch url; defaults to the mock server
  const c8*        path;            // appended to the mock server url; defaults to /
  resolve_kind_t   resolve;
  const c8*        resolve_host;    // fetch http://<resolve_host>:<port>/ instead of the literal
  sp_http_method_t method;
  const c8*        payload;
  const c8*        content_type;
  header_t         headers [FETCH_MAX_HEADERS];
  u32              connect_timeout_ms;
  u32              io_timeout_ms;
  step_t           scripts [FETCH_MAX_SCRIPTS][FETCH_MAX_STEPS];
  expect_t         expect;
  const c8*        captured [FETCH_MAX_CAPTURED]; // substrings that must appear in requests the server received
  counted_t        counted [FETCH_MAX_COUNTED];   // substrings that must appear exactly n times
} test_t;

static const test_t tests [] = {
  {
    .name = "content_length",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello" } }},
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "status_404",
    .scripts = {{ { .send = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nnot found" } }},
    .expect = { .err = SP_HTTP_ERR_STATUS, .status = 404, .body = "not found" },
  },
  {
    .name = "no_body_204",
    .scripts = {{ { .send = "HTTP/1.1 204 No Content\r\n\r\n" } }},
    .expect = { .status = 204, .body = "" },
  },
  {
    .name = "early_hints",
    .scripts = {{ { .send = "HTTP/1.1 103 Early Hints\r\nLink: </s.css>; rel=preload\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }},
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "interim_flood",
    .scripts = {{ { .send = "HTTP/1.1 100 Continue\r\n\r\n", .repeat = 20 } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "upgrade_101",
    .scripts = {{ { .send = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "bad_status_line",
    .scripts = {{ { .send = "ICY 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "conflicting_length",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Length: 9999\r\n\r\nhello" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "te_gzip",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: gzip\r\n\r\nblob" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "chunked",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n" } }},
    .expect = { .status = 200, .body = "hello world" },
  },
  {
    .name = "chunked_bad_separator",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhelloXX\r\n0\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "huge_head",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nX-Pad: ", .pad = 96 * 1024 } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "crlf_location",
    .scripts = {{ { .send = "HTTP/1.1 302 Found\r\nLocation: /a\rSet-Cookie: pwn=1\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_URL },
  },
  {
    .name = "redirect_userinfo",
    .scripts = {{ { .send = "HTTP/1.1 302 Found\r\nLocation: http://evil@127.0.0.1/x\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_URL },
  },
  {
    .name = "redirect_loop",
    .scripts = {{ { .send = "HTTP/1.1 302 Found\r\nLocation: /loop\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_REDIRECTS },
  },
  {
    .name = "redirect_follow",
    .scripts = {
      { { .send = "HTTP/1.1 302 Found\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
  },
  {
    .name = "split_head",
    .scripts = {{
      { .send = "HTTP/1.1 200 OK\r\nContent-Le" },
      { .send = "ngth: 5\r\n\r\nhello", .delay_ms = 30 },
    }},
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "truncated_body",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nhello" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "query_no_path",
    .path = "?a=b",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /?a=b HTTP/1.1" },
  },
  {
    .name = "eof_body",
    .tls = true,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } }},
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "eof_body_no_close_notify",
    .tls = true,
    .no_close_notify = true,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "tls_trusted",
    .tls = true,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }},
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "tls_untrusted",
    .tls = true,
    .untrusted = true,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }},
    .expect = { .err = SP_HTTP_ERR_UNTRUSTED },
  },
  {
    .name = "redirect_query_url",
    .scripts = {
      { { .send = "HTTP/1.1 302 Found\r\nLocation: /next?u=https://example.com/\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /next?u=https://example.com/ HTTP/1.1" },
  },
  {
    .name = "timeout_head",
    .io_timeout_ms = 120,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", .delay_ms = 1000 } }},
    .expect = { .err = SP_HTTP_ERR_TIMEOUT },
  },
  {
    .name = "timeout_body",
    .io_timeout_ms = 120,
    .scripts = {{
      { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhe" },
      { .send = "llo", .delay_ms = 1000 },
    }},
    .expect = { .err = SP_HTTP_ERR_TIMEOUT },
  },
  {
    .name = "timeout_tls",
    .tls = true,
    .io_timeout_ms = 120,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", .delay_ms = 1000 } }},
    .expect = { .err = SP_HTTP_ERR_TIMEOUT },
  },
  {
    .name = "proxy_absolute_form",
    .proxy = true,
    .url = "http://example.test:8080/x",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET http://example.test:8080/x HTTP/1.1", "Host: example.test:8080" },
  },
  {
    .name = "proxy_connect",
    .tls = true,
    .proxy = true,
    .proxy_connect = true,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello" } }},
    .expect = { .status = 200, .body = "hello" },
    .captured = { "CONNECT 127.0.0.1:", "GET / HTTP/1.1" },
  },
  {
    .name = "proxy_connect_refused",
    .tls = true,
    .proxy = true,
    .proxy_connect = true,
    .connect_reply = "HTTP/1.1 403 Forbidden\r\n\r\n",
    .expect = { .err = SP_HTTP_ERR_PROXY },
  },
  {
    .name = "custom_resolver",
    .resolve = RESOLVE_LOCAL,
    .resolve_host = "A",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET / HTTP/1.1", "Host: A:" },
  },
  {
    .name = "resolver_error_propagates",
    .resolve = RESOLVE_REFUSE,
    .resolve_host = "A",
    .expect = { .err = SP_HTTP_ERR_CONNECT },
  },
  {
    .name = "literal_bypasses_resolver",
    .resolve = RESOLVE_REFUSE,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
  },
  {
    .name = "post_payload",
    .method = SP_HTTP_POST,
    .payload = "{\"a\":1}",
    .content_type = "application/json",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "POST / HTTP/1.1", "Content-Type: application/json" },
    .counted = { { "{\"a\":1}", 1 }, { "Content-Length: 7", 1 } },
  },
  {
    .name = "post_empty_payload",
    .method = SP_HTTP_POST,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "POST / HTTP/1.1" },
    .counted = { { "Content-Length: 0", 1 }, { "Content-Type:", 0 } },
  },
  {
    .name = "put_payload",
    .method = SP_HTTP_PUT,
    .payload = "data",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "PUT / HTTP/1.1", "Content-Length: 4" },
  },
  {
    .name = "delete_no_payload",
    .method = SP_HTTP_DELETE,
    .scripts = {{ { .send = "HTTP/1.1 204 No Content\r\n\r\n" } }},
    .expect = { .status = 204, .body = "" },
    .captured = { "DELETE / HTTP/1.1" },
    .counted = { { "Content-Length", 0 } },
  },
  {
    .name = "delete_with_payload",
    .method = SP_HTTP_DELETE,
    .payload = "why",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "DELETE / HTTP/1.1", "Content-Length: 3" },
    .counted = { { "why", 1 } },
  },
  {
    .name = "post_303_rewrites_to_get",
    .method = SP_HTTP_POST,
    .payload = "ping",
    .content_type = "text/plain",
    .scripts = {
      { { .send = "HTTP/1.1 303 See Other\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "POST / HTTP/1.1", "GET /next HTTP/1.1" },
    .counted = { { "ping", 1 }, { "Content-Type:", 1 }, { "Content-Length:", 1 } },
  },
  {
    .name = "post_301_rewrites_to_get",
    .method = SP_HTTP_POST,
    .payload = "ping",
    .scripts = {
      { { .send = "HTTP/1.1 301 Moved Permanently\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /next HTTP/1.1" },
    .counted = { { "ping", 1 } },
  },
  {
    .name = "post_307_preserves_method",
    .method = SP_HTTP_POST,
    .payload = "ping",
    .content_type = "text/plain",
    .scripts = {
      { { .send = "HTTP/1.1 307 Temporary Redirect\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "POST /next HTTP/1.1" },
    .counted = { { "ping", 2 }, { "Content-Type: text/plain", 2 } },
  },
  {
    .name = "post_308_preserves_method",
    .method = SP_HTTP_POST,
    .payload = "ping",
    .scripts = {
      { { .send = "HTTP/1.1 308 Permanent Redirect\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "POST /next HTTP/1.1" },
    .counted = { { "ping", 2 } },
  },
  {
    .name = "custom_headers",
    .headers = {
      { "X-Custom", "abc" },
      { "Authorization", "Bearer tok" },
    },
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "X-Custom: abc", "Authorization: Bearer tok" },
  },
  {
    .name = "header_host_override",
    .headers = { { "Host", "override.test" } },
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "Host: override.test" },
    .counted = { { "Host:", 1 } },
  },
  {
    .name = "header_content_type_override",
    .method = SP_HTTP_POST,
    .payload = "x",
    .content_type = "text/plain",
    .headers = { { "Content-Type", "application/json" } },
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "Content-Type: application/json" },
    .counted = { { "Content-Type:", 1 } },
  },
  {
    .name = "header_crlf_rejected",
    .headers = { { "X-Bad", "a\r\nEvil: 1" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "header_name_crlf_rejected",
    .headers = { { "X-Bad\r\nEvil", "1" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "header_lone_newline_rejected",
    .headers = { { "X-Bad", "a\nb" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "header_reserved_rejected",
    .headers = { { "Content-Length", "5" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "header_connection_rejected",
    .headers = { { "Connection", "keep-alive" } },
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
  },
  {
    .name = "redirect_cross_host_strips_auth",
    .headers = { { "Authorization", "Bearer tok" }, { "Cookie", "a=1" }, { "X-Keep", "yes" } },
    .scripts = {
      { { .send = "HTTP/1.1 302 Found\r\nLocation: http://localhost:@PORT@/next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /next HTTP/1.1" },
    .counted = { { "Bearer tok", 1 }, { "Cookie:", 1 }, { "X-Keep: yes", 2 } },
  },
  {
    .name = "redirect_same_host_keeps_auth",
    .headers = { { "Authorization", "Bearer tok" } },
    .scripts = {
      { { .send = "HTTP/1.1 302 Found\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } },
    },
    .expect = { .status = 200, .body = "ok" },
    .counted = { { "Bearer tok", 2 } },
  },
  {
    .name = "response_metadata",
    .method = SP_HTTP_POST,
    .payload = "x",
    .scripts = {{ { .send = "HTTP/1.1 201 Created\r\nContent-Type: application/json\r\nLocation: /created/1\r\nContent-Length: 4\r\n\r\ndone" } }},
    .expect = { .status = 201, .body = "done", .content_type = "application/json", .location = "/created/1" },
  },
};

static const c8* fetch_cert =
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

static const c8* fetch_key =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgkOe51G2MRpZ8kyVN\n"
  "tWv2/RKrkE9WfLCS4zbMvdlNAUOhRANCAAQ2Hl0cVbbPLuko5otFB3zmPXuP0Lpx\n"
  "11IBhV1NM8Zw6kl46p9Qzc/rljXgguMNSYS3HV1wGDqZ+PON+S5OO/tU\n"
  "-----END PRIVATE KEY-----\n";

typedef struct {
  mbedtls_net_context      listen;
  sp_atomic_s32_t          stop;
  bool                     tls;
  bool                     no_close_notify;
  bool                     proxy_connect;
  const c8*                connect_reply;
  const step_t             (*scripts) [FETCH_MAX_STEPS];
  u32                      script_count;
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
    s32 head_end = sp_str_find(sp_str(buf, len), sp_str_lit("\r\n\r\n"));
    if (head_end != SP_STR_NO_MATCH) {
      u64 need = 0;
      sp_str_t head = sp_str_sub(sp_str(buf, len), 0, head_end);
      s32 at = sp_str_find(head, sp_str_lit("Content-Length: "));
      if (at != SP_STR_NO_MATCH) {
        sp_str_t rest = sp_str_sub(head, at + (s32)(sizeof("Content-Length: ") - 1), (s32)head.len - at - (s32)(sizeof("Content-Length: ") - 1));
        s32 nl = sp_str_find(rest, sp_str_lit("\r\n"));
        sp_str_t num = nl == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, nl);
        sp_parse_u64_ex(num, &need);
      }
      if ((u64)len >= (u64)head_end + 4 + need) {
        ok = true;
        break;
      }
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

static void mock_play(mbedtls_ssl_context* ssl, mbedtls_net_context* net, const step_t* steps) {
  sp_for(it, FETCH_MAX_STEPS) {
    step_t step = steps[it];
    if (!step.send && !step.pad) break;
    u32 repeat = step.repeat ? step.repeat : 1;
    sp_for(r, repeat) {
      if (step.delay_ms) sp_sleep_ms((f64)step.delay_ms);
      if (step.send) {
        if (!mock_send(ssl, net, (const u8*)step.send, sp_cstr_len(step.send))) return;
      }
      u8 chunk [512];
      sp_mem_fill_u8(chunk, sizeof(chunk), 'a');
      u32 pad = step.pad;
      while (pad > 0) {
        u32 take = pad < sizeof(chunk) ? pad : (u32)sizeof(chunk);
        if (!mock_send(ssl, net, chunk, take)) return;
        pad -= take;
      }
    }
  }
}

static s32 server_thread(void* userdata) {
  server_t* server = (server_t*)userdata;
  u32 index = 0;
  for (;;) {
    mbedtls_net_context client;
    mbedtls_net_init(&client);
    if (mbedtls_net_accept(&server->listen, &client, SP_NULLPTR, 0, SP_NULLPTR) != 0) break;
    if (sp_atomic_s32_load(&server->stop, SP_ATOMIC_SEQ_CST)) {
      mbedtls_net_free(&client);
      break;
    }

    bool ok = true;
    if (server->proxy_connect) {
      ok = mock_read_request(server, SP_NULLPTR, &client);
      if (ok) {
        const c8* reply = server->connect_reply ? server->connect_reply : "HTTP/1.1 200 Connection established\r\n\r\n";
        ok = mock_send(SP_NULLPTR, &client, (const u8*)reply, sp_cstr_len(reply));
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

    if (ok) ok = mock_read_request(server, sslp, &client);
    if (ok) {
      u32 which = index < server->script_count ? index : server->script_count - 1;
      mock_play(sslp, &client, server->scripts[which]);
    }
    if (server->tls && ok && !server->no_close_notify) mbedtls_ssl_close_notify(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_net_free(&client);
    index++;
  }
  return 0;
}

static const c8* subst_port(sp_mem_t mem, const c8* send, const c8* port) {
  sp_str_t str = sp_cstr_as_str(send);
  s32 at = sp_str_find(str, sp_str_lit("@PORT@"));
  if (at == SP_STR_NO_MATCH) return send;
  sp_str_t result = sp_fmt(mem, "{}{}{}",
    sp_fmt_str(sp_str_sub(str, 0, at)),
    sp_fmt_cstr(port),
    sp_fmt_str(sp_str_sub(str, at + 6, (s32)str.len - at - 6))).value;
  return sp_str_to_cstr(mem, result);
}

static u32 count_matches(sp_str_t haystack, sp_str_t needle) {
  if (sp_str_empty(needle) || needle.len > haystack.len) return 0;
  u32 count = 0;
  sp_for_range(it, 0, haystack.len - needle.len + 1) {
    if (sp_str_equal(sp_str_sub(haystack, it, (s32)needle.len), needle)) count++;
  }
  return count;
}

static sp_http_error_t resolve_local(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count) {
  (void)user_data; (void)host; (void)timeout_ms; (void)capacity;
  addrs[0] = (sp_http_addr_t) { .kind = SP_HTTP_ADDR_V4, .data = { 127, 0, 0, 1 } };
  *count = 1;
  return SP_HTTP_OK;
}

static sp_http_error_t resolve_refuse(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count) {
  (void)user_data; (void)host; (void)timeout_ms; (void)addrs; (void)capacity;
  *count = 0;
  return SP_HTTP_ERR_CONNECT;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
#if !defined(SP_WIN32)
  signal(SIGPIPE, SIG_IGN);
#endif

  server_t server = sp_zero;
  mbedtls_net_init(&server.listen);
  server.tls = c->tls;
  server.no_close_notify = c->no_close_notify;
  server.proxy_connect = c->proxy_connect;
  server.connect_reply = c->connect_reply;
  server.scripts = c->scripts;
  server.script_count = 0;
  sp_carr_for(c->scripts, it) {
    if (c->scripts[it][0].send || c->scripts[it][0].pad) server.script_count++;
  }
  if (!server.script_count) server.script_count = 1;

  sp_must_eq(t, mbedtls_net_bind(&server.listen, "127.0.0.1", "0", MBEDTLS_NET_PROTO_TCP), 0);
  u16 port_value = 0;
  sp_must_ok(t, sp_sys_socket_local_port((sp_sys_socket_t)server.listen.fd, &port_value));
  const c8* port = sp_str_to_cstr(mem, sp_fmt(mem, "{}", sp_fmt_uint(port_value)).value);

  sp_carr_for(c->scripts, s) {
    sp_carr_for(c->scripts[s], step) {
      if (c->scripts[s][step].send) {
        c->scripts[s][step].send = subst_port(mem, c->scripts[s][step].send, port);
      }
    }
  }

  if (c->tls) {
    mbedtls_x509_crt_init(&server.crt);
    mbedtls_pk_init(&server.pk);
    mbedtls_entropy_init(&server.entropy);
    mbedtls_ctr_drbg_init(&server.drbg);
    mbedtls_ssl_config_init(&server.conf);
    sp_must_eq(t, mbedtls_x509_crt_parse(&server.crt, (const unsigned char*)fetch_cert, sp_cstr_len(fetch_cert) + 1), 0);
    sp_must_eq(t, mbedtls_ctr_drbg_seed(&server.drbg, mbedtls_entropy_func, &server.entropy, SP_NULLPTR, 0), 0);
    sp_must_eq(t, mbedtls_pk_parse_key(&server.pk, (const unsigned char*)fetch_key, sp_cstr_len(fetch_key) + 1, SP_NULLPTR, 0, mbedtls_ctr_drbg_random, &server.drbg), 0);
    sp_must_eq(t, mbedtls_ssl_config_defaults(&server.conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT), 0);
    mbedtls_ssl_conf_rng(&server.conf, mbedtls_ctr_drbg_random, &server.drbg);
    sp_must_eq(t, mbedtls_ssl_conf_own_cert(&server.conf, &server.crt, &server.pk), 0);
  }

  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, server_thread, &server);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  trust.backend = SP_TLS_BACKEND_ANCHORS;
  if (c->tls && !c->untrusted) {
    sp_must_eq(t, mbedtls_x509_crt_parse((mbedtls_x509_crt*)trust.anchors, (const unsigned char*)fetch_cert, sp_cstr_len(fetch_cert) + 1), 0);
  }
  sp_tls_mbedtls_t client = sp_zero;
  sp_tls_mbedtls_init(&client, &trust);

  sp_io_dyn_mem_writer_t body = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &body);

  sp_str_t url;
  if (c->resolve_host) {
    url = sp_fmt(mem, "http://{}:{}/", sp_fmt_cstr(c->resolve_host), sp_fmt_cstr(port)).value;
  }
  else if (c->url) {
    url = sp_cstr_as_str(c->url);
  }
  else {
    url = sp_fmt(mem, "{}://127.0.0.1:{}{}", sp_fmt_cstr(c->tls ? "https" : "http"), sp_fmt_cstr(port), sp_fmt_cstr(c->path ? c->path : "/")).value;
  }

  sp_http_request_t request = {
    .url    = url,
    .tls    = &client.base,
    .sink   = &body.base,
    .method = c->method,
    .proxy  = c->proxy ? sp_fmt(mem, "127.0.0.1:{}", sp_fmt_cstr(port)).value : sp_str_lit(""),
    .no_proxy = !c->proxy, // isolate the suite from proxies in the developer's environment
    .connect_timeout_ms = c->connect_timeout_ms,
    .io_timeout_ms = c->io_timeout_ms,
  };
  if (c->resolve == RESOLVE_LOCAL)  request.resolver = (sp_http_resolver_t) { .resolve = resolve_local };
  if (c->resolve == RESOLVE_REFUSE) request.resolver = (sp_http_resolver_t) { .resolve = resolve_refuse };
  if (c->payload)      request.payload = sp_cstr_as_str(c->payload);
  if (c->content_type) request.content_type = sp_cstr_as_str(c->content_type);
  sp_carr_for(c->headers, it) {
    if (!c->headers[it].name) break;
    request.headers[it].name = sp_cstr_as_str(c->headers[it].name);
    if (c->headers[it].value) request.headers[it].value = sp_cstr_as_str(c->headers[it].value);
  }

  sp_http_response_t response = sp_zero;
  sp_http_error_t err = sp_http_fetch(mem, request, &response);

  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (c->expect.status) sp_expect_eq(t, response.status, c->expect.status);
  if (c->expect.body) sp_expect_str_eq_c(t, sp_io_dyn_mem_writer_as_str(&body), c->expect.body);
  if (c->expect.content_type) sp_expect_str_eq_c(t, response.content_type, c->expect.content_type);
  if (c->expect.location) sp_expect_str_eq_c(t, response.location, c->expect.location);

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
  sp_carr_for(c->counted, it) {
    if (!c->counted[it].text) continue;
    sp_test_kv_c(t, "needle", c->counted[it].text);
    sp_expect_eq(t, count_matches(captured, sp_cstr_as_str(c->counted[it].text)), c->counted[it].n);
  }
  sp_test_kv_clear(t, "needle");

  mbedtls_net_free(&server.listen);
  if (c->tls) {
    mbedtls_ssl_config_free(&server.conf);
    mbedtls_pk_free(&server.pk);
    mbedtls_x509_crt_free(&server.crt);
    mbedtls_ctr_drbg_free(&server.drbg);
    mbedtls_entropy_free(&server.entropy);
  }
  sp_tls_trust_free(&trust);
  return SP_OK;
}

sp_test_each_fn(http, fetch, test_t, tests, run);
