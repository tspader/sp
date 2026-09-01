#include "../http.h"

#if !defined(SP_WIN32)
#include <signal.h>
#endif

#define FETCH_MAX_STEPS    4
#define FETCH_MAX_SCRIPTS  2
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
  const c8* text;
  u32       n;
} counted_t;

typedef struct {
  sp_http_error_t err;
  s32             status;
  const c8*       body;
  header_t        headers [HTTP_TEST_MAX_HEADERS];
} expect_t;

typedef struct {
  const c8*        name;
  bool             proxy;           // send the request through the mock server as a proxy
  const c8*        url;             // fetch url; defaults to the mock server
  const c8*        path;            // appended to the mock server url; defaults to /
  resolve_kind_t   resolve;
  const c8*        resolve_host;    // fetch http://<resolve_host>:<port>/ instead of the literal
  sp_http_method_t method;
  const c8*        payload;
  const c8*        content_type;
  header_t         headers [HTTP_TEST_MAX_HEADERS];
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
    .name = "no_body_304_ignores_length",
    .scripts = {{ { .send = "HTTP/1.1 304 Not Modified\r\nContent-Length: 20\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_STATUS, .status = 304, .body = "" },
  },
  {
    .name = "head_no_body",
    .method = SP_HTTP_HEAD,
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\n" } }},
    .expect = { .status = 200, .body = "" },
    .captured = { "HEAD / HTTP/1.1" },
  },
  {
    .name = "head_redirect_stays_head",
    .method = SP_HTTP_HEAD,
    .scripts = {
      { { .send = "HTTP/1.1 302 Found\r\nLocation: /next\r\n\r\n" } },
      { { .send = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n" } },
    },
    .expect = { .status = 200, .body = "" },
    .captured = { "HEAD /next HTTP/1.1" },
  },
  {
    .name = "https_needs_tls",
    .url = "https://127.0.0.1:1/",
    .expect = { .err = SP_HTTP_ERR_BAD_CONFIG },
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
    .name = "chunked",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n" } }},
    .expect = { .status = 200, .body = "hello world" },
  },
  {
    .name = "huge_head",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nX-Pad: ", .pad = 96 * 1024 } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "crlf_location",
    .scripts = {{ { .send = "HTTP/1.1 302 Found\r\nLocation: /a\rSet-Cookie: pwn=1\r\n\r\n" } }},
    .expect = { .err = SP_HTTP_ERR_PROTOCOL },
  },
  {
    .name = "redirect_without_location",
    .scripts = {{ { .send = "HTTP/1.1 302 Found\r\nContent-Length: 1\r\n\r\nx" } }},
    .expect = { .err = SP_HTTP_ERR_STATUS, .status = 302, .body = "x" },
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
    .name = "eof_body",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nhello" } }},
    .expect = { .status = 200, .body = "hello" },
  },
  {
    .name = "query_no_path",
    .path = "?a=b",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET /?a=b HTTP/1.1" },
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
    .name = "proxy_absolute_form",
    .proxy = true,
    .url = "http://example.test:8080/x",
    .scripts = {{ { .send = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok" } }},
    .expect = { .status = 200, .body = "ok" },
    .captured = { "GET http://example.test:8080/x HTTP/1.1", "Host: example.test:8080" },
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
    .resolve = RESOLVE_LOCAL,
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
    .name = "response_headers_kept",
    .method = SP_HTTP_POST,
    .payload = "x",
    .scripts = {{ { .send = "HTTP/1.1 201 Created\r\nContent-Type: application/json\r\nLocation: /created/1\r\nContent-Length: 4\r\n\r\ndone" } }},
    .expect = {
      .status = 201,
      .body = "done",
      .headers = { { "content-type", "application/json" }, { "location", "/created/1" } },
    },
  },
};

typedef struct {
  sp_sys_socket_t listener;
  sp_atomic_s32_t stop;
  const step_t    (*scripts) [FETCH_MAX_STEPS];
  u32             script_count;
  c8              captured [8192];
  u32             captured_len;
} server_t;

static bool mock_send(sp_sys_socket_t socket, const u8* data, u32 len) {
  u32 sent = 0;
  while (sent < len) {
    u64 n = 0;
    if (sp_sys_socket_send(socket, data + sent, len - sent, &n) != SP_OK) return false;
    sent += (u32)n;
  }
  return true;
}

static bool mock_read_request(server_t* server, sp_sys_socket_t socket) {
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
    u64 n = 0;
    if (sp_sys_socket_recv(socket, (u8*)buf + len, sizeof(buf) - len, &n) != SP_OK) break;
    if (n == 0) break;
    len += (u32)n;
  }
  u32 space = (u32)sizeof(server->captured) - server->captured_len;
  u32 take = len < space ? len : space;
  sp_mem_copy(server->captured + server->captured_len, buf, take);
  server->captured_len += take;
  return ok;
}

static void mock_play(sp_sys_socket_t socket, const step_t* steps) {
  sp_for(it, FETCH_MAX_STEPS) {
    step_t step = steps[it];
    if (!step.send && !step.pad) break;
    u32 repeat = step.repeat ? step.repeat : 1;
    sp_for(r, repeat) {
      if (step.delay_ms) sp_sleep_ms((f64)step.delay_ms);
      if (step.send) {
        if (!mock_send(socket, (const u8*)step.send, sp_cstr_len(step.send))) return;
      }
      u8 chunk [512];
      sp_mem_fill_u8(chunk, sizeof(chunk), 'a');
      u32 pad = step.pad;
      while (pad > 0) {
        u32 take = pad < sizeof(chunk) ? pad : (u32)sizeof(chunk);
        if (!mock_send(socket, chunk, take)) return;
        pad -= take;
      }
    }
  }
}

static s32 server_thread(void* userdata) {
  server_t* server = (server_t*)userdata;
  u32 index = 0;
  for (;;) {
    sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
    if (sp_sys_socket_accept(server->listener, (sp_sys_handle_desc_t) { SP_SYS_BLOCKING }, &client) != SP_OK) break;
    if (sp_atomic_s32_load(&server->stop, SP_ATOMIC_SEQ_CST)) {
      sp_sys_socket_close(client);
      break;
    }

    if (mock_read_request(server, client)) {
      u32 which = index < server->script_count ? index : server->script_count - 1;
      mock_play(client, server->scripts[which]);
    }
    sp_sys_socket_close(client);
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
  server.listener = SP_SYS_INVALID_SOCKET;
  server.scripts = c->scripts;
  sp_carr_for(c->scripts, it) {
    if (c->scripts[it][0].send || c->scripts[it][0].pad) server.script_count++;
  }
  if (!server.script_count) server.script_count = 1;

  sp_must_ok(t, sp_sys_socket_open(&server.listener, SP_SYS_SOCKET_STREAM, (sp_sys_handle_desc_t) { SP_SYS_BLOCKING }));
  sp_must_ok(t, sp_sys_socket_bind(server.listener, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 } }));
  sp_must_ok(t, sp_sys_socket_listen(server.listener, 4));
  sp_sys_ipv4_t server_addr = sp_zero;
  sp_must_ok(t, sp_sys_socket_local_addr(server.listener, &server_addr));
  const c8* port = sp_str_to_cstr(mem, sp_fmt(mem, "{}", sp_fmt_uint(server_addr.port)).value);

  sp_carr_for(c->scripts, s) {
    sp_carr_for(c->scripts[s], step) {
      if (c->scripts[s][step].send) {
        c->scripts[s][step].send = subst_port(mem, c->scripts[s][step].send, port);
      }
    }
  }

  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, server_thread, &server);

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
    url = sp_fmt(mem, "http://127.0.0.1:{}{}", sp_fmt_cstr(port), sp_fmt_cstr(c->path ? c->path : "/")).value;
  }

  sp_http_request_t request = {
    .url    = url,
    .sink   = &body.base,
    .method = c->method,
    .proxy  = c->proxy ? sp_fmt(mem, "127.0.0.1:{}", sp_fmt_cstr(port)).value : sp_str_lit(""),
    .no_proxy = !c->proxy, // isolate the suite from proxies in the developer's environment
    .io_timeout_ms = c->io_timeout_ms,
  };
  if (c->resolve == RESOLVE_LOCAL)  request.resolver = (sp_http_resolver_t) { .resolve = resolve_local };
  if (c->resolve == RESOLVE_REFUSE) request.resolver = (sp_http_resolver_t) { .resolve = resolve_refuse };
  if (c->payload)      request.payload = sp_cstr_as_str(c->payload);
  if (c->content_type) request.content_type = sp_cstr_as_str(c->content_type);
  sp_http_header_t headers [HTTP_TEST_MAX_HEADERS];
  u32 num_headers = 0;
  sp_carr_for(c->headers, it) {
    if (!c->headers[it].name) break;
    headers[num_headers++] = (sp_http_header_t) {
      .name = sp_cstr_as_str(c->headers[it].name),
      .value = c->headers[it].value ? sp_cstr_as_str(c->headers[it].value) : sp_str_lit(""),
    };
  }
  request.headers = headers;
  request.num_headers = num_headers;

  sp_http_response_t response = sp_zero;
  sp_http_error_t err = sp_http_fetch(mem, request, &response);

  sp_expect_eq(t, (s32)err, (s32)c->expect.err);
  if (c->expect.status) sp_expect_eq(t, response.status, c->expect.status);
  if (c->expect.body) sp_expect_str_eq_c(t, sp_io_dyn_mem_writer_as_str(&body), c->expect.body);
  sp_carr_for(c->expect.headers, it) {
    if (!c->expect.headers[it].name) break;
    sp_test_kv_c(t, "header", c->expect.headers[it].name);
    sp_expect_str_eq_c(t, sp_http_headers_find(response.headers, sp_cstr_as_str(c->expect.headers[it].name)), c->expect.headers[it].value);
  }
  sp_test_kv_clear(t, "header");

  sp_atomic_s32_store(&server.stop, 1, SP_ATOMIC_SEQ_CST);
  sp_sys_socket_t poke = SP_SYS_INVALID_SOCKET;
  if (sp_sys_socket_open(&poke, SP_SYS_SOCKET_STREAM, (sp_sys_handle_desc_t) { SP_SYS_BLOCKING }) == SP_OK) {
    sp_sys_socket_connect(poke, server_addr);
    sp_sys_socket_close(poke);
  }
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

  sp_sys_socket_close(server.listener);
  return SP_OK;
}

sp_test_each_fn(http, fetch, test_t, tests, run);
