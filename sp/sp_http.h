#ifndef SP_HTTP_H
#define SP_HTTP_H

#include "sp.h"

#if defined(SP_WIN32) || defined(SP_POSIX)
  #define SP_HTTP_SOCKETS
#endif

struct mbedtls_x509_crt;
struct mbedtls_ssl_config;
struct mbedtls_ssl_context;

typedef enum {
  SP_HTTP_OK = 0,
  SP_HTTP_ERR_NO_STORE,
  SP_HTTP_ERR_PARSE,
  SP_HTTP_ERR_OS,
  SP_HTTP_ERR_UNTRUSTED,
  SP_HTTP_ERR_BAD_CONFIG,
  SP_HTTP_ERR_UNSUPPORTED,
  SP_HTTP_ERR_URL,
  SP_HTTP_ERR_CONNECT,
  SP_HTTP_ERR_HANDSHAKE,
  SP_HTTP_ERR_PROTOCOL,
  SP_HTTP_ERR_STATUS,
  SP_HTTP_ERR_REDIRECTS,
  SP_HTTP_ERR_TIMEOUT,
  SP_HTTP_ERR_PROXY,
} sp_http_error_t;

// @http
#define SP_HTTP_DEFAULT_REDIRECTS          16
#define SP_HTTP_DEFAULT_CONNECT_TIMEOUT_MS 30000
#define SP_HTTP_DEFAULT_IO_TIMEOUT_MS      60000
#define SP_HTTP_TIMEOUT_INFINITE           0xffffffffu
#define SP_HTTP_MAX_ADDRS                  16

#ifndef SP_HTTP_MAX_HEADERS
  #define SP_HTTP_MAX_HEADERS 16
#endif

typedef struct {
  sp_str_t scheme;
  sp_str_t host;
  sp_str_t port;
  sp_str_t path;
  bool     tls;
} sp_http_url_t;

typedef enum {
  SP_HTTP_GET = 0,
  SP_HTTP_POST,
  SP_HTTP_PUT,
  SP_HTTP_PATCH,
  SP_HTTP_DELETE,
} sp_http_method_t;

typedef struct {
  sp_str_t name;
  sp_str_t value;
} sp_http_header_t;

typedef enum {
  SP_HTTP_ADDR_V4,
  SP_HTTP_ADDR_V6,
} sp_http_addr_kind_t;

typedef struct {
  sp_http_addr_kind_t kind;
  u8                  data [16];
} sp_http_addr_t;

// Resolves a bare hostname to addresses; IP literals are handled before the
// resolver is consulted, so implementations only ever see real names.
typedef sp_http_error_t (*sp_http_resolve_fn)(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count);

typedef struct {
  sp_http_resolve_fn resolve;
  void*              user_data;
} sp_http_resolver_t;


// @tls
typedef struct sp_tls sp_tls_t;

struct sp_tls {
  sp_http_error_t (*open)(sp_tls_t* tls, sp_sys_socket_t socket, sp_str_t hostname, u32 io_timeout_ms);
  void            (*close)(sp_tls_t* tls);
  sp_io_reader_t* reader;
  sp_io_writer_t* writer;
};

typedef enum {
  SP_TLS_BACKEND_NONE,
  SP_TLS_BACKEND_ANCHORS,
  SP_TLS_BACKEND_OS_VERIFY,
} sp_tls_backend_t;

typedef struct {
  sp_tls_backend_t         backend;
  struct mbedtls_x509_crt* anchors;
  u32                      loaded;
  u32                      skipped;
  sp_mem_t                 mem;
} sp_tls_trust_t;

typedef struct {
  sp_str_t hostname;
} sp_tls_verify_t;

typedef s32 (*sp_tls_verify_fn)(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags);

SP_API sp_tls_backend_t sp_tls_native_backend(void);


SP_API sp_http_error_t  sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem);
SP_API void             sp_tls_trust_free(sp_tls_trust_t* trust);
SP_API sp_http_error_t  sp_tls_trust_load(sp_tls_trust_t* trust);


SP_API sp_http_error_t  sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped);
SP_API sp_http_error_t  sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped);
SP_API sp_http_error_t  sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped);


SP_API sp_http_error_t  sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf);
SP_API sp_http_error_t  sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname);


SP_API s32              sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags);
SP_API sp_http_error_t  sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname);
SP_API sp_http_error_t  sp_tls_windows_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname);


SP_API sp_http_error_t  sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count);


#if defined(SP_TLS_WITH_MBEDTLS)
#include <mbedtls/x509_crt.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

typedef struct sp_tls_mbedtls sp_tls_mbedtls_t;

typedef struct {
  sp_io_reader_t    base;
  sp_tls_mbedtls_t* tls;
  bool              eof;
} sp_tls_mbedtls_reader_t;

typedef struct {
  sp_io_writer_t    base;
  sp_tls_mbedtls_t* tls;
} sp_tls_mbedtls_writer_t;

struct sp_tls_mbedtls {
  sp_tls_t                 base;
  const sp_tls_trust_t*    trust;
  sp_sys_socket_t          socket;
  u32                      io_timeout_ms;
  mbedtls_net_context      net;
  mbedtls_ssl_context      ssl;
  mbedtls_ssl_config       conf;
  mbedtls_entropy_context  entropy;
  mbedtls_ctr_drbg_context drbg;
  sp_tls_verify_t          verify;
  sp_tls_mbedtls_reader_t  reader;
  sp_tls_mbedtls_writer_t  writer;
};

SP_API void             sp_tls_mbedtls_init(sp_tls_mbedtls_t* tls, const sp_tls_trust_t* trust);
#endif


// @http
typedef struct {
  sp_str_t           url;
  sp_tls_t*          tls;
  sp_http_resolver_t resolver;
  sp_io_writer_t*    sink;
  sp_http_method_t   method;
  sp_str_t           payload;
  sp_str_t           content_type;
  sp_http_header_t   headers[SP_HTTP_MAX_HEADERS];
  u32                max_redirects;
  sp_str_t           proxy;
  bool               no_proxy;
  u32                connect_timeout_ms;
  u32                io_timeout_ms;
} sp_http_request_t;

typedef struct {
  s32      status;
  u64      body_len;
  sp_str_t url;
  sp_str_t content_type;
  sp_str_t location;
} sp_http_response_t;

SP_API bool               sp_http_url_parse(sp_str_t url, sp_http_url_t* out);
SP_API sp_http_resolver_t sp_http_resolver_default(void);
SP_API sp_http_error_t    sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response);

#endif

#if defined SP_IMPLEMENTATION && !defined(SP_HTTP_IMPLEMENTATION)
  #define SP_HTTP_IMPLEMENTATION
#endif

#if !defined(SP_HTTP_IMPL_H)
#if defined(SP_HTTP_EVERYTHING_PUBLIC) || defined(SP_HTTP_IMPLEMENTATION)
#define SP_HTTP_IMPL_H

typedef struct {
  s32      status;
  sp_str_t location;
  sp_str_t content_type;
  bool     chunked;
  bool     has_length;
  u64      length;
} sp_http_head_t;

typedef struct {
  sp_http_url_t           url;
  bool                    absolute_form;
  sp_http_method_t        method;
  sp_str_t                payload;
  sp_str_t                content_type;
  const sp_http_header_t* headers;
  bool                    strip_auth;
} sp_http_wire_t;

// @http
SP_PRIVATE bool            sp_http_ci_equal(sp_str_t a, sp_str_t b);
SP_PRIVATE bool            sp_http_ci_contains(sp_str_t haystack, sp_str_t needle);
SP_PRIVATE sp_str_t        sp_http_str_tail(sp_str_t str, s32 from);
SP_PRIVATE sp_str_t        sp_http_host_bare(sp_str_t host);
SP_PRIVATE bool            sp_http_v6_group(sp_str_t part, u16* value);
SP_PRIVATE bool            sp_http_addr_parse_v4(sp_str_t host, sp_http_addr_t* out);
SP_PRIVATE bool            sp_http_addr_parse_v6(sp_str_t host, sp_http_addr_t* out);
SP_PRIVATE bool            sp_http_addr_parse(sp_str_t host, sp_http_addr_t* out);
SP_PRIVATE bool            sp_http_url_host_ok(sp_str_t host);
SP_PRIVATE bool            sp_http_url_port_ok(sp_str_t port);
SP_PRIVATE bool            sp_http_url_path_ok(sp_str_t path);
SP_PRIVATE sp_http_error_t sp_http_parse_head(sp_str_t head, sp_http_head_t* out);
SP_PRIVATE bool            sp_http_location_is_absolute(sp_str_t location);
SP_PRIVATE sp_str_t        sp_http_resolve_url(sp_mem_t mem, sp_http_url_t base, sp_str_t location);
SP_PRIVATE bool            sp_http_no_proxy_match(sp_str_t host, sp_str_t no_proxy);
SP_PRIVATE sp_str_t        sp_http_proxy_pick(sp_http_url_t url, sp_str_t http_proxy, sp_str_t https_proxy, sp_str_t all_proxy, sp_str_t no_proxy);
SP_PRIVATE sp_str_t        sp_http_env_either(const c8* lower, const c8* upper);
SP_PRIVATE sp_str_t        sp_http_proxy_from_env(sp_http_url_t url);
SP_PRIVATE sp_http_error_t sp_http_map_io(sp_err_t err, sp_http_error_t fallback);
SP_PRIVATE u32             sp_http_timeout_ms(u32 requested, u32 fallback);
SP_PRIVATE sp_http_error_t sp_http_read_head(sp_io_reader_t* reader, sp_str_t* head);
SP_PRIVATE sp_http_error_t sp_http_read_line(sp_io_reader_t* reader, sp_str_t* line);
SP_PRIVATE sp_http_error_t sp_http_copy_n(sp_io_reader_t* reader, sp_io_writer_t* body, u64 n, u64* written);
SP_PRIVATE sp_http_error_t sp_http_read_body(sp_io_reader_t* reader, sp_http_head_t head, sp_io_writer_t* body, u64* written);
SP_PRIVATE sp_str_t        sp_http_method_name(sp_http_method_t method);
SP_PRIVATE bool            sp_http_headers_have(const sp_http_header_t* headers, sp_str_t name);
SP_PRIVATE sp_http_error_t sp_http_headers_check(const sp_http_header_t* headers);
SP_PRIVATE sp_str_t        sp_http_build_request(sp_mem_t mem, sp_http_wire_t wire);
SP_PRIVATE bool            sp_http_proxy_url_parse(sp_mem_t mem, sp_str_t proxy, sp_http_url_t* out);

#if defined(SP_HTTP_SOCKETS)
typedef struct sp_http_conn sp_http_conn_t;

SP_PRIVATE sp_http_error_t sp_http_resolve_system(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count);
SP_PRIVATE sp_http_error_t sp_http_connect_addr(sp_sys_socket_t* out, sp_http_addr_t addr, u16 port, u32 timeout_ms);
SP_PRIVATE sp_http_error_t sp_http_net_connect(sp_sys_socket_t* out, sp_http_resolver_t resolver, sp_str_t host, u16 port, u32 timeout_ms);
SP_PRIVATE sp_http_error_t sp_http_conn_write(sp_http_conn_t* conn, sp_str_t data);
SP_PRIVATE sp_http_error_t sp_http_connect_reply(sp_http_conn_t* conn);
SP_PRIVATE sp_http_error_t sp_http_conn_open(sp_http_conn_t* conn, sp_tls_t* tls, sp_http_resolver_t resolver, sp_http_url_t url, const sp_http_url_t* proxy, u32 connect_timeout_ms, u32 io_timeout_ms);
SP_PRIVATE void            sp_http_conn_close(sp_http_conn_t* conn);
#endif

#if defined(SP_TLS_WITH_MBEDTLS)
// @tls
SP_PRIVATE u32             sp_tls_chain_count(const struct mbedtls_x509_crt* chain);
#if defined(SP_WIN32)
// PCCERT_CONTEXT spelled out; wincrypt.h isn't in scope until the implementation
SP_PRIVATE bool            sp_tls_win32_server_auth(const struct _CERT_CONTEXT* ctx);
#endif

SP_PRIVATE sp_http_error_t sp_tls_mbedtls_open(sp_tls_t* base, sp_sys_socket_t socket, sp_str_t hostname, u32 io_timeout_ms);
SP_PRIVATE void            sp_tls_mbedtls_close(sp_tls_t* base);
SP_PRIVATE sp_http_error_t sp_tls_mbedtls_handshake(sp_tls_mbedtls_t* tls, sp_str_t hostname);
SP_PRIVATE void            sp_tls_mbedtls_drop(sp_tls_mbedtls_t* tls);
SP_PRIVATE sp_err_t        sp_tls_mbedtls_wait(sp_tls_mbedtls_t* tls, s32 rc);
SP_PRIVATE sp_err_t        sp_tls_mbedtls_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read);
SP_PRIVATE sp_err_t        sp_tls_mbedtls_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written);
#endif

#endif // SP_HTTP_EVERYTHING_PUBLIC or SP_HTTP_IMPLEMENTATION
#endif // SP_HTTP_IMPL_H

#ifndef SP_HTTP_C
#if defined(SP_HTTP_IMPLEMENTATION)
#define SP_HTTP_C

sp_tls_backend_t sp_tls_native_backend(void) {
#if defined(SP_WIN32)
  return SP_TLS_BACKEND_OS_VERIFY;
#elif defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
  return SP_TLS_BACKEND_OS_VERIFY;
#elif defined(SP_MACOS)
  // If you didn't compile in SecTrust, fail here instead of at runtime
  return SP_TLS_BACKEND_NONE;
#elif defined(SP_LINUX)
  return SP_TLS_BACKEND_ANCHORS;
#else
  return SP_TLS_BACKEND_NONE;
#endif
}

SP_PRIVATE bool sp_http_ci_equal(sp_str_t a, sp_str_t b) {
  if (a.len != b.len) return false;
  sp_for(it, a.len) {
    c8 ca = a.data[it];
    c8 cb = b.data[it];
    if (ca >= 'A' && ca <= 'Z') ca = (c8)(ca + 32);
    if (cb >= 'A' && cb <= 'Z') cb = (c8)(cb + 32);
    if (ca != cb) return false;
  }
  return true;
}

SP_PRIVATE bool sp_http_ci_contains(sp_str_t haystack, sp_str_t needle) {
  if (needle.len > haystack.len) return false;
  sp_for_range(it, 0, haystack.len - needle.len + 1) {
    if (sp_http_ci_equal(sp_str_sub(haystack, it, (s32)needle.len), needle)) return true;
  }
  return false;
}

SP_PRIVATE sp_str_t sp_http_str_tail(sp_str_t str, s32 from) {
  return sp_str_sub(str, from, (s32)str.len - from);
}

SP_PRIVATE sp_str_t sp_http_host_bare(sp_str_t host) {
  if (host.len >= 2 && host.data[0] == '[' && host.data[host.len - 1] == ']') {
    return sp_str_sub(host, 1, (s32)host.len - 2);
  }
  return host;
}

SP_PRIVATE bool sp_http_url_host_ok(sp_str_t host) {
  if (sp_str_empty(host)) return false;
  if (host.data[0] == '[') {
    if (host.len < 4 || host.data[host.len - 1] != ']') return false;
    sp_for_range(it, 1, host.len - 1) {
      c8 c = host.data[it];
      bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
      if (!hex && c != ':' && c != '.') return false;
    }
    return true;
  }
  sp_for(it, host.len) {
    c8 c = host.data[it];
    bool alnum = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    if (!alnum && c != '-' && c != '.' && c != '_') return false;
  }
  return true;
}

SP_PRIVATE bool sp_http_v6_group(sp_str_t part, u16* value) {
  if (sp_str_empty(part) || part.len > 4) return false;
  u32 group = 0;
  sp_for(it, part.len) {
    c8 c = part.data[it];
    u32 digit;
    if (c >= '0' && c <= '9') digit = (u32)(c - '0');
    else if (c >= 'a' && c <= 'f') digit = (u32)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F') digit = (u32)(c - 'A' + 10);
    else return false;
    group = (group << 4) | digit;
  }
  *value = (u16)group;
  return true;
}

SP_PRIVATE bool sp_http_addr_parse_v4(sp_str_t host, sp_http_addr_t* out) {
  *out = sp_zero_s(sp_http_addr_t);
  sp_str_t rest = host;
  sp_for(it, 4) {
    s32 dot = sp_str_find_c8(rest, '.');
    if ((it < 3) != (dot != SP_STR_NO_MATCH)) return false;
    sp_str_t part = dot == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, dot);
    if (sp_str_empty(part) || part.len > 3) return false;
    if (part.len > 1 && part.data[0] == '0') return false;
    u32 value = 0;
    if (!sp_parse_u32_ex(part, &value)) return false;
    if (value > 255) return false;
    out->data[it] = (u8)value;
    rest = dot == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, dot + 1);
  }
  out->kind = SP_HTTP_ADDR_V4;
  return true;
}

SP_PRIVATE bool sp_http_addr_parse_v6(sp_str_t host, sp_http_addr_t* out) {
  *out = sp_zero_s(sp_http_addr_t);
  u16 groups [8] = sp_zero;
  u32 count = 0;
  s32 run = -1;

  sp_str_t rest = host;
  if (sp_str_starts_with(rest, sp_str_lit("::"))) {
    run = 0;
    rest = sp_http_str_tail(rest, 2);
  }
  else if (!sp_str_empty(rest) && rest.data[0] == ':') {
    return false;
  }

  while (!sp_str_empty(rest)) {
    if (count == 8) return false;
    s32 colon = sp_str_find_c8(rest, ':');
    sp_str_t part = colon == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, colon);

    if (sp_str_find_c8(part, '.') != SP_STR_NO_MATCH) {
      if (colon != SP_STR_NO_MATCH || count > 6) return false;
      sp_http_addr_t v4 = sp_zero;
      if (!sp_http_addr_parse_v4(part, &v4)) return false;
      groups[count++] = (u16)((v4.data[0] << 8) | v4.data[1]);
      groups[count++] = (u16)((v4.data[2] << 8) | v4.data[3]);
      break;
    }

    if (!sp_http_v6_group(part, &groups[count])) return false;
    count++;

    if (colon == SP_STR_NO_MATCH) break;
    rest = sp_http_str_tail(rest, colon + 1);
    if (sp_str_empty(rest)) return false;
    if (rest.data[0] == ':') {
      if (run >= 0) return false;
      run = (s32)count;
      rest = sp_http_str_tail(rest, 1);
    }
  }

  if (run < 0 && count != 8) return false;
  if (run >= 0 && count >= 8) return false;

  u32 front = run < 0 ? count : (u32)run;
  u32 back = count - front;
  sp_for(it, front) {
    out->data[it * 2 + 0] = (u8)(groups[it] >> 8);
    out->data[it * 2 + 1] = (u8)(groups[it] & 0xff);
  }
  sp_for(it, back) {
    u32 at = 8 - back + it;
    out->data[at * 2 + 0] = (u8)(groups[front + it] >> 8);
    out->data[at * 2 + 1] = (u8)(groups[front + it] & 0xff);
  }
  out->kind = SP_HTTP_ADDR_V6;
  return true;
}

SP_PRIVATE bool sp_http_addr_parse(sp_str_t host, sp_http_addr_t* out) {
  if (sp_str_find_c8(host, ':') != SP_STR_NO_MATCH) return sp_http_addr_parse_v6(host, out);
  return sp_http_addr_parse_v4(host, out);
}

SP_PRIVATE bool sp_http_url_port_ok(sp_str_t port) {
  if (sp_str_empty(port) || port.len > 5) return false;
  sp_for(it, port.len) {
    if (port.data[it] < '0' || port.data[it] > '9') return false;
  }
  u32 value = 0;
  if (!sp_parse_u32_ex(port, &value)) return false;
  return value >= 1 && value <= 65535;
}

SP_PRIVATE bool sp_http_url_path_ok(sp_str_t path) {
  sp_for(it, path.len) {
    u8 c = (u8)path.data[it];
    if (c <= 0x20 || c == 0x7f) return false;
  }
  return true;
}

bool sp_http_url_parse(sp_str_t url, sp_http_url_t* out) {
  *out = sp_zero_s(sp_http_url_t);

  s32 hash = sp_str_find_c8(url, '#');
  if (hash != SP_STR_NO_MATCH) url = sp_str_sub(url, 0, hash);

  sp_str_t rest = url;
  s32 sep = sp_str_find(url, sp_str_lit("://"));
  if (sep != SP_STR_NO_MATCH) {
    out->scheme = sp_str_sub(url, 0, sep);
    rest = sp_http_str_tail(url, sep + 3);
  }
  else {
    out->scheme = sp_str_lit("https");
  }

  bool https = sp_http_ci_equal(out->scheme, sp_str_lit("https"));
  bool http  = sp_http_ci_equal(out->scheme, sp_str_lit("http"));
  if (!https && !http) return false;
  out->tls = https;

  s32 slash = sp_str_find_c8(rest, '/');
  s32 qmark = sp_str_find_c8(rest, '?');
  s32 cut = slash;
  if (qmark != SP_STR_NO_MATCH && (slash == SP_STR_NO_MATCH || qmark < slash)) cut = qmark;
  sp_str_t authority;
  if (cut == SP_STR_NO_MATCH) {
    authority = rest;
    out->path = sp_str_lit("/");
  }
  else {
    authority = sp_str_sub(rest, 0, cut);
    out->path = sp_http_str_tail(rest, cut);
  }

  if (sp_str_find_c8(authority, '@') != SP_STR_NO_MATCH) return false;

  if (!sp_str_empty(authority) && authority.data[0] == '[') {
    s32 close = sp_str_find_c8(authority, ']');
    if (close == SP_STR_NO_MATCH) return false;
    out->host = sp_str_sub(authority, 0, close + 1);
    sp_str_t after = sp_http_str_tail(authority, close + 1);
    if (sp_str_empty(after)) {
      out->port = out->tls ? sp_str_lit("443") : sp_str_lit("80");
    }
    else {
      if (after.data[0] != ':') return false;
      out->port = sp_http_str_tail(after, 1);
    }
  }
  else {
    s32 colon = sp_str_find_c8(authority, ':');
    if (colon == SP_STR_NO_MATCH) {
      out->host = authority;
      out->port = out->tls ? sp_str_lit("443") : sp_str_lit("80");
    }
    else {
      out->host = sp_str_sub(authority, 0, colon);
      out->port = sp_http_str_tail(authority, colon + 1);
    }
  }

  return sp_http_url_host_ok(out->host) && sp_http_url_port_ok(out->port) && sp_http_url_path_ok(out->path);
}


SP_PRIVATE sp_http_error_t sp_http_parse_head(sp_str_t head, sp_http_head_t* out) {
  *out = sp_zero_s(sp_http_head_t);

  sp_str_t rest = head;
  bool first = true;
  for (;;) {
    s32 nl = sp_str_find(rest, sp_str_lit("\r\n"));
    sp_str_t line = nl == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, nl);

    if (first) {
      if (!sp_str_starts_with(line, sp_str_lit("HTTP/"))) return SP_HTTP_ERR_PROTOCOL;
      s32 sp1 = sp_str_find_c8(line, ' ');
      if (sp1 == SP_STR_NO_MATCH) return SP_HTTP_ERR_PROTOCOL;
      sp_str_t after = sp_str_trim_left(sp_http_str_tail(line, sp1 + 1));
      s32 sp2 = sp_str_find_c8(after, ' ');
      sp_str_t code = sp2 == SP_STR_NO_MATCH ? after : sp_str_sub(after, 0, sp2);
      u32 status = 0;
      if (!sp_parse_u32_ex(code, &status)) return SP_HTTP_ERR_PROTOCOL;
      out->status = (s32)status;
      first = false;
    }
    else if (!sp_str_empty(line)) {
      s32 colon = sp_str_find_c8(line, ':');
      if (colon != SP_STR_NO_MATCH) {
        sp_str_t name = sp_str_sub(line, 0, colon);
        sp_str_t value = sp_str_trim(sp_http_str_tail(line, colon + 1));
        if (sp_http_ci_equal(name, sp_str_lit("location"))) {
          out->location = value;
        }
        else if (sp_http_ci_equal(name, sp_str_lit("content-type"))) {
          out->content_type = value;
        }
        else if (sp_http_ci_equal(name, sp_str_lit("content-length"))) {
          u64 length = 0;
          if (!sp_parse_u64_ex(value, &length)) return SP_HTTP_ERR_PROTOCOL;
          if (out->has_length && out->length != length) return SP_HTTP_ERR_PROTOCOL;
          out->has_length = true;
          out->length = length;
        }
        else if (sp_http_ci_equal(name, sp_str_lit("transfer-encoding"))) {
          out->chunked = sp_http_ci_contains(value, sp_str_lit("chunked"));
          if (!out->chunked) return SP_HTTP_ERR_PROTOCOL;
        }
      }
    }

    if (nl == SP_STR_NO_MATCH) break;
    rest = sp_http_str_tail(rest, nl + 2);
  }
  return SP_HTTP_OK;
}

SP_PRIVATE bool sp_http_location_is_absolute(sp_str_t location) {
  s32 sep = sp_str_find(location, sp_str_lit("://"));
  if (sep == SP_STR_NO_MATCH || sep == 0) return false;
  sp_for(it, (u32)sep) {
    c8 c = location.data[it];
    bool alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    bool digit = (c >= '0' && c <= '9');
    if (it == 0 && !alpha) return false;
    if (!alpha && !digit && c != '+' && c != '-' && c != '.') return false;
  }
  return true;
}

SP_PRIVATE sp_str_t sp_http_resolve_url(sp_mem_t mem, sp_http_url_t base, sp_str_t location) {
  if (sp_http_location_is_absolute(location)) {
    return sp_str_copy(mem, location);
  }
  sp_str_t scheme = base.tls ? sp_str_lit("https") : sp_str_lit("http");
  if (location.len >= 2 && location.data[0] == '/' && location.data[1] == '/') {
    return sp_fmt(mem, "{}:{}", sp_fmt_str(scheme), sp_fmt_str(location)).value;
  }
  if (!sp_str_empty(location) && location.data[0] == '/') {
    return sp_fmt(mem, "{}://{}:{}{}",
      sp_fmt_str(scheme), sp_fmt_str(base.host), sp_fmt_str(base.port), sp_fmt_str(location)).value;
  }
  s32 slash = sp_str_find_c8_reverse(base.path, '/');
  sp_str_t dir = slash == SP_STR_NO_MATCH ? sp_str_lit("/") : sp_str_sub(base.path, 0, slash + 1);
  return sp_fmt(mem, "{}://{}:{}{}{}",
    sp_fmt_str(scheme), sp_fmt_str(base.host), sp_fmt_str(base.port), sp_fmt_str(dir), sp_fmt_str(location)).value;
}

SP_PRIVATE bool sp_http_no_proxy_match(sp_str_t host, sp_str_t no_proxy) {
  sp_str_t rest = no_proxy;
  while (!sp_str_empty(rest)) {
    s32 comma = sp_str_find_c8(rest, ',');
    sp_str_t entry = comma == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, comma);
    rest = comma == SP_STR_NO_MATCH ? sp_zero_s(sp_str_t) : sp_http_str_tail(rest, comma + 1);

    entry = sp_str_trim(entry);
    if (sp_str_empty(entry)) continue;
    if (sp_str_equal_cstr(entry, "*")) return true;
    if (entry.data[0] == '.') entry = sp_http_str_tail(entry, 1);
    if (sp_str_empty(entry)) continue;
    if (sp_http_ci_equal(host, entry)) return true;
    if (host.len > entry.len &&
        host.data[host.len - entry.len - 1] == '.' &&
        sp_http_ci_equal(sp_http_str_tail(host, (s32)(host.len - entry.len)), entry)) {
      return true;
    }
  }
  return false;
}

SP_PRIVATE sp_str_t sp_http_proxy_pick(sp_http_url_t url, sp_str_t http_proxy, sp_str_t https_proxy, sp_str_t all_proxy, sp_str_t no_proxy) {
  if (sp_http_no_proxy_match(sp_http_host_bare(url.host), no_proxy)) return sp_zero_s(sp_str_t);
  sp_str_t proxy = url.tls ? https_proxy : http_proxy;
  if (sp_str_empty(proxy)) proxy = all_proxy;
  return proxy;
}

SP_PRIVATE sp_str_t sp_http_env_either(const c8* lower, const c8* upper) {
  sp_str_t value = sp_os_env_get(sp_cstr_as_str(lower));
  if (sp_str_empty(value) && upper) value = sp_os_env_get(sp_cstr_as_str(upper));
  return value;
}

SP_PRIVATE sp_str_t sp_http_proxy_from_env(sp_http_url_t url) {
  return sp_http_proxy_pick(url,
    // uppercase HTTP_PROXY is deliberately ignored (CGI can set it from a request header)
    sp_http_env_either("http_proxy", SP_NULLPTR),
    sp_http_env_either("https_proxy", "HTTPS_PROXY"),
    sp_http_env_either("all_proxy", "ALL_PROXY"),
    sp_http_env_either("no_proxy", "NO_PROXY"));
}

SP_PRIVATE sp_http_error_t sp_http_map_io(sp_err_t err, sp_http_error_t fallback) {
  if (err == SP_ERR_IO_TIMEOUT) return SP_HTTP_ERR_TIMEOUT;
  if (err == SP_ERR_IO_NO_SPACE) return SP_HTTP_ERR_OS;
  if (err >= SP_ERR_SYS && err < SP_ERR_SYS + 100) return SP_HTTP_ERR_OS;
  return fallback;
}

// resolved timeouts: 0 means block forever, matching mbedtls_net_recv_timeout
SP_PRIVATE u32 sp_http_timeout_ms(u32 requested, u32 fallback) {
  if (requested == SP_HTTP_TIMEOUT_INFINITE) return 0;
  return requested ? requested : fallback;
}

SP_PRIVATE sp_http_error_t sp_http_read_head(sp_io_reader_t* reader, sp_str_t* head) {
  sp_str_t acc = sp_zero;
  sp_err_t err = sp_io_peek_until(reader, sp_str_lit("\r\n\r\n"), &acc);
  // a head that outgrows the buffer is the peer's protocol violation, not an
  // OS failure
  if (err == SP_ERR_IO_NO_SPACE) return SP_HTTP_ERR_PROTOCOL;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  sp_io_consume(reader, acc.len);
  *head = sp_str_sub(acc, 0, (s32)acc.len - 4);
  return SP_HTTP_OK;
}

SP_PRIVATE sp_http_error_t sp_http_read_line(sp_io_reader_t* reader, sp_str_t* line) {
  sp_str_t acc = sp_zero;
  sp_err_t err = sp_io_peek_until(reader, sp_str_lit("\r\n"), &acc);
  if (err == SP_ERR_IO_NO_SPACE) return SP_HTTP_ERR_PROTOCOL;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  sp_io_consume(reader, acc.len);
  *line = sp_str_sub(acc, 0, (s32)acc.len - 2);
  return SP_HTTP_OK;
}

SP_PRIVATE sp_http_error_t sp_http_copy_n(sp_io_reader_t* reader, sp_io_writer_t* body, u64 n, u64* written) {
  if (!body) {
    sp_err_t err = sp_io_discard(reader, n, SP_NULLPTR);
    if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
    return SP_HTTP_OK;
  }

  sp_io_limit_reader_t limit = sp_zero;
  sp_io_limit_reader_init(&limit, reader, n);
  u64 copied = 0;
  sp_err_t err = sp_io_copy(body, &limit.base, &copied);
  if (written) *written += copied;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  if (limit.remaining) return SP_HTTP_ERR_PROTOCOL;
  return SP_HTTP_OK;
}

SP_PRIVATE sp_http_error_t sp_http_read_body(sp_io_reader_t* reader, sp_http_head_t head, sp_io_writer_t* body, u64* written) {
  if (head.status < 200 || head.status == 204 || head.status == 304) {
    return SP_HTTP_OK;
  }
  if (head.chunked) {
    for (;;) {
      sp_str_t line = sp_zero;
      sp_http_error_t err = sp_http_read_line(reader, &line);
      if (err != SP_HTTP_OK) return err;
      sp_str_t size_str = line;
      s32 semi = sp_str_find_c8(size_str, ';');
      if (semi != SP_STR_NO_MATCH) size_str = sp_str_sub(size_str, 0, semi);
      size_str = sp_str_trim(size_str);

      u64 size = 0;
      if (!sp_parse_hex_ex(size_str, &size)) return SP_HTTP_ERR_PROTOCOL;
      if (size == 0) break;

      sp_http_error_t copy_err = sp_http_copy_n(reader, body, size, written);
      if (copy_err != SP_HTTP_OK) return copy_err;

      sp_str_t crlf = sp_zero;
      copy_err = sp_http_read_line(reader, &crlf);
      if (copy_err != SP_HTTP_OK) return copy_err;
      if (!sp_str_empty(crlf)) return SP_HTTP_ERR_PROTOCOL;
    }
    return SP_HTTP_OK;
  }
  if (head.has_length) {
    return sp_http_copy_n(reader, body, head.length, written);
  }
  if (!body) {
    sp_err_t err = sp_io_discard(reader, SP_LIMIT_U64_MAX, SP_NULLPTR);
    if (err != SP_OK && err != SP_ERR_IO_EOF) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
    return SP_HTTP_OK;
  }
  u64 copied = 0;
  sp_err_t err = sp_io_copy(body, reader, &copied);
  if (written) *written += copied;
  if (err != SP_OK) return sp_http_map_io(err, SP_HTTP_ERR_PROTOCOL);
  return SP_HTTP_OK;
}

SP_PRIVATE sp_str_t sp_http_method_name(sp_http_method_t method) {
  switch (method) {
    case SP_HTTP_GET:    return sp_str_lit("GET");
    case SP_HTTP_POST:   return sp_str_lit("POST");
    case SP_HTTP_PUT:    return sp_str_lit("PUT");
    case SP_HTTP_PATCH:  return sp_str_lit("PATCH");
    case SP_HTTP_DELETE: return sp_str_lit("DELETE");
  }
  return sp_str_lit("GET");
}

SP_PRIVATE bool sp_http_headers_have(const sp_http_header_t* headers, sp_str_t name) {
  sp_for(it, SP_HTTP_MAX_HEADERS) {
    if (sp_str_empty(headers[it].name)) break;
    if (sp_http_ci_equal(headers[it].name, name)) return true;
  }
  return false;
}

SP_PRIVATE sp_http_error_t sp_http_headers_check(const sp_http_header_t* headers) {
  sp_for(it, SP_HTTP_MAX_HEADERS) {
    sp_str_t name = headers[it].name;
    sp_str_t value = headers[it].value;
    if (sp_str_empty(name)) break;
    sp_for(at, name.len) {
      u8 c = (u8)name.data[at];
      if (c <= 0x20 || c >= 0x7f || c == ':') return SP_HTTP_ERR_BAD_CONFIG;
    }
    sp_for(at, value.len) {
      u8 c = (u8)value.data[at];
      if ((c < 0x20 && c != '\t') || c == 0x7f) return SP_HTTP_ERR_BAD_CONFIG;
    }
    if (sp_http_ci_equal(name, sp_str_lit("content-length")) ||
        sp_http_ci_equal(name, sp_str_lit("transfer-encoding")) ||
        sp_http_ci_equal(name, sp_str_lit("connection"))) {
      return SP_HTTP_ERR_BAD_CONFIG;
    }
  }
  return SP_HTTP_OK;
}

SP_PRIVATE sp_str_t sp_http_build_request(sp_mem_t mem, sp_http_wire_t wire) {
  sp_http_url_t url = wire.url;
  bool default_port =
    (url.tls && sp_str_equal_cstr(url.port, "443")) ||
    (!url.tls && sp_str_equal_cstr(url.port, "80"));
  sp_str_t host_header = default_port
    ? url.host
    : sp_fmt(mem, "{}:{}", sp_fmt_str(url.host), sp_fmt_str(url.port)).value;
  sp_str_t path = url.path;
  if (!sp_str_empty(path) && path.data[0] == '?') {
    path = sp_fmt(mem, "/{}", sp_fmt_str(path)).value;
  }
  sp_str_t target = wire.absolute_form
    ? sp_fmt(mem, "http://{}{}", sp_fmt_str(host_header), sp_fmt_str(path)).value
    : path;

  sp_io_dyn_mem_writer_t head = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &head);
  sp_io_write_str(&head.base, sp_fmt(mem, "{} {} HTTP/1.1\r\n", sp_fmt_str(sp_http_method_name(wire.method)), sp_fmt_str(target)).value, SP_NULLPTR);
  if (!sp_http_headers_have(wire.headers, sp_str_lit("host"))) {
    sp_io_write_str(&head.base, sp_fmt(mem, "Host: {}\r\n", sp_fmt_str(host_header)).value, SP_NULLPTR);
  }
  if (!sp_http_headers_have(wire.headers, sp_str_lit("user-agent"))) {
    sp_io_write_str(&head.base, sp_str_lit("User-Agent: sp-http/1.0\r\n"), SP_NULLPTR);
  }
  if (!sp_http_headers_have(wire.headers, sp_str_lit("accept"))) {
    sp_io_write_str(&head.base, sp_str_lit("Accept: */*\r\n"), SP_NULLPTR);
  }
  sp_io_write_str(&head.base, sp_str_lit("Connection: close\r\n"), SP_NULLPTR);
  bool body_expected =
    wire.method == SP_HTTP_POST ||
    wire.method == SP_HTTP_PUT ||
    wire.method == SP_HTTP_PATCH;
  if (body_expected || !sp_str_empty(wire.payload)) {
    sp_io_write_str(&head.base, sp_fmt(mem, "Content-Length: {}\r\n", sp_fmt_uint(wire.payload.len)).value, SP_NULLPTR);
  }
  if (!sp_str_empty(wire.payload) && !sp_str_empty(wire.content_type) && !sp_http_headers_have(wire.headers, sp_str_lit("content-type"))) {
    sp_io_write_str(&head.base, sp_fmt(mem, "Content-Type: {}\r\n", sp_fmt_str(wire.content_type)).value, SP_NULLPTR);
  }
  sp_for(it, SP_HTTP_MAX_HEADERS) {
    if (sp_str_empty(wire.headers[it].name)) break;
    if (wire.strip_auth &&
        (sp_http_ci_equal(wire.headers[it].name, sp_str_lit("authorization")) ||
         sp_http_ci_equal(wire.headers[it].name, sp_str_lit("cookie")))) {
      continue;
    }
    sp_io_write_str(&head.base, sp_fmt(mem, "{}: {}\r\n", sp_fmt_str(wire.headers[it].name), sp_fmt_str(wire.headers[it].value)).value, SP_NULLPTR);
  }
  sp_io_write_str(&head.base, sp_str_lit("\r\n"), SP_NULLPTR);
  return sp_io_dyn_mem_writer_as_str(&head);
}

// a proxy url; scheme defaults to http, and only plain-http proxies are supported
SP_PRIVATE bool sp_http_proxy_url_parse(sp_mem_t mem, sp_str_t proxy, sp_http_url_t* out) {
  if (sp_str_find(proxy, sp_str_lit("://")) == SP_STR_NO_MATCH) {
    proxy = sp_fmt(mem, "http://{}", sp_fmt_str(proxy)).value;
  }
  if (!sp_http_url_parse(proxy, out)) return false;
  return !out->tls;
}

#if defined(SP_HTTP_SOCKETS)

#if defined(SP_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#define SP_HTTP_BUFFER_SIZE 16384
#define SP_HTTP_HEAD_MAX    SP_HTTP_BUFFER_SIZE
#define SP_HTTP_MAX_INTERIM 8

struct sp_http_conn {
  sp_sys_socket_t       socket;
  sp_tls_t*             tls;
  u32                   io_timeout_ms;
  sp_io_socket_reader_t sock_reader;
  sp_io_socket_writer_t sock_writer;
  sp_io_reader_t*       reader;
  sp_io_writer_t*       writer;
  u8                    buffer[SP_HTTP_BUFFER_SIZE];
};

SP_PRIVATE sp_http_error_t sp_http_resolve_system(void* user_data, sp_str_t host, u32 timeout_ms, sp_http_addr_t* addrs, u32 capacity, u32* count) {
  (void)user_data;
  (void)timeout_ms;
  *count = 0;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  c8* name = sp_str_to_cstr(scratch.mem, host);

  struct addrinfo hints = sp_zero;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  struct addrinfo* list = SP_NULLPTR;
  s32 rc = getaddrinfo(name, SP_NULLPTR, &hints, &list);
  sp_mem_end_scratch(scratch);
  if (rc != 0) return SP_HTTP_ERR_CONNECT;

  u32 found = 0;
  for (struct addrinfo* it = list; it && found < capacity; it = it->ai_next) {
    if (it->ai_family == AF_INET) {
      struct sockaddr_in* sa = (struct sockaddr_in*)it->ai_addr;
      addrs[found] = sp_zero_s(sp_http_addr_t);
      addrs[found].kind = SP_HTTP_ADDR_V4;
      sp_mem_copy(addrs[found].data, &sa->sin_addr, 4);
      found++;
    }
    else if (it->ai_family == AF_INET6) {
      struct sockaddr_in6* sa = (struct sockaddr_in6*)it->ai_addr;
      addrs[found] = sp_zero_s(sp_http_addr_t);
      addrs[found].kind = SP_HTTP_ADDR_V6;
      sp_mem_copy(addrs[found].data, &sa->sin6_addr, 16);
      found++;
    }
  }
  freeaddrinfo(list);

  *count = found;
  return found ? SP_HTTP_OK : SP_HTTP_ERR_CONNECT;
}

sp_http_resolver_t sp_http_resolver_default(void) {
  return (sp_http_resolver_t) { .resolve = sp_http_resolve_system };
}

SP_PRIVATE sp_http_error_t sp_http_connect_addr(sp_sys_socket_t* out, sp_http_addr_t addr, u16 port, u32 timeout_ms) {
  struct sockaddr_storage storage = sp_zero;
  s32 family;
  socklen_t addr_len;
  if (addr.kind == SP_HTTP_ADDR_V4) {
    struct sockaddr_in* sa = (struct sockaddr_in*)&storage;
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    sp_mem_copy(&sa->sin_addr, addr.data, 4);
    family = AF_INET;
    addr_len = (socklen_t)sizeof(struct sockaddr_in);
  }
  else {
    struct sockaddr_in6* sa = (struct sockaddr_in6*)&storage;
    sa->sin6_family = AF_INET6;
    sa->sin6_port = htons(port);
    sp_mem_copy(&sa->sin6_addr, addr.data, 16);
    family = AF_INET6;
    addr_len = (socklen_t)sizeof(struct sockaddr_in6);
  }

#if defined(SP_WIN32)
  SOCKET fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == INVALID_SOCKET) return SP_HTTP_ERR_CONNECT;
  sp_sys_socket_set_nonblocking((sp_sys_socket_t)fd);
  bool connected = connect(fd, (struct sockaddr*)&storage, (int)addr_len) == 0;
  bool pending = !connected && WSAGetLastError() == WSAEWOULDBLOCK;
#else
  int fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) return SP_HTTP_ERR_CONNECT;
  sp_sys_socket_set_nonblocking((sp_sys_socket_t)fd);
  bool connected = connect(fd, (struct sockaddr*)&storage, addr_len) == 0;
  bool pending = !connected && errno == EINPROGRESS;
#endif

  sp_http_error_t result = SP_HTTP_ERR_CONNECT;
  if (pending) {
    sp_err_t wait = sp_sys_socket_wait((sp_sys_socket_t)fd, false, timeout_ms);
    if (wait == SP_ERR_SYS_TIMED_OUT) result = SP_HTTP_ERR_TIMEOUT;
    if (wait == SP_OK) connected = sp_sys_socket_error((sp_sys_socket_t)fd) == SP_OK;
  }

  if (connected) {
    // Stays nonblocking: sp_io socket timeouts and the WANT_READ/WANT_WRITE
    // pumps depend on it.
    *out = (sp_sys_socket_t)fd;
    return SP_HTTP_OK;
  }

  sp_sys_socket_close((sp_sys_socket_t)fd);
  return result;
}

SP_PRIVATE sp_http_error_t sp_http_net_connect(sp_sys_socket_t* out, sp_http_resolver_t resolver, sp_str_t host, u16 port, u32 timeout_ms) {
#if defined(SP_WIN32)
  static bool wsa_init = false;
  if (!wsa_init) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return SP_HTTP_ERR_OS;
    wsa_init = true;
  }
#endif

  sp_http_addr_t addrs [SP_HTTP_MAX_ADDRS];
  u32 count = 0;
  if (sp_http_addr_parse(host, &addrs[0])) {
    count = 1;
  }
  else {
    sp_http_error_t err = resolver.resolve(resolver.user_data, host, timeout_ms, addrs, SP_HTTP_MAX_ADDRS, &count);
    if (err != SP_HTTP_OK) return err;
    if (!count) return SP_HTTP_ERR_CONNECT;
  }

  sp_http_error_t result = SP_HTTP_ERR_CONNECT;
  sp_for(it, count) {
    result = sp_http_connect_addr(out, addrs[it], port, timeout_ms);
    if (result == SP_HTTP_OK) break;
  }
  return result;
}

SP_PRIVATE sp_http_error_t sp_http_conn_write(sp_http_conn_t* conn, sp_str_t data) {
  sp_err_t err = sp_io_write_all(conn->writer, data.data, data.len, SP_NULLPTR);
  return err == SP_OK ? SP_HTTP_OK : sp_http_map_io(err, SP_HTTP_ERR_OS);
}

// reads the proxy's reply to CONNECT one byte at a time; the reader is
// unbuffered here, so nothing past the head is consumed and the TLS handshake
// sees a clean stream
SP_PRIVATE sp_http_error_t sp_http_connect_reply(sp_http_conn_t* conn) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  c8* head = sp_alloc_n(scratch.mem, c8, SP_HTTP_HEAD_MAX);
  u64 len = 0;

  sp_http_error_t result = SP_HTTP_ERR_PROXY;
  while (len < SP_HTTP_HEAD_MAX) {
    u64 n = 0;
    sp_err_t err = sp_io_read(conn->reader, head + len, 1, &n);
    if (err != SP_OK) {
      result = sp_http_map_io(err, SP_HTTP_ERR_PROXY);
      break;
    }
    len += n;

    if (sp_str_ends_with(sp_str(head, (u32)len), sp_str_lit("\r\n\r\n"))) {
      sp_http_head_t parsed = sp_zero;
      if (sp_http_parse_head(sp_str(head, (u32)len - 4), &parsed) == SP_HTTP_OK &&
          parsed.status >= 200 && parsed.status <= 299) {
        result = SP_HTTP_OK;
      }
      break;
    }
  }
  sp_mem_end_scratch(scratch);
  return result;
}

SP_PRIVATE sp_http_error_t sp_http_conn_open(sp_http_conn_t* conn, sp_tls_t* tls, sp_http_resolver_t resolver, sp_http_url_t url, const sp_http_url_t* proxy, u32 connect_timeout_ms, u32 io_timeout_ms) {
  conn->socket = SP_SYS_INVALID_SOCKET;
  conn->tls = SP_NULLPTR;
  conn->io_timeout_ms = io_timeout_ms;

  sp_http_url_t target = proxy ? *proxy : url;
  u32 port = 0;
  sp_parse_u32_ex(target.port, &port);

  sp_http_error_t err = sp_http_net_connect(&conn->socket, resolver, sp_http_host_bare(target.host), (u16)port, connect_timeout_ms);
  if (err != SP_HTTP_OK) return err;

  sp_io_socket_reader_init(&conn->sock_reader, conn->socket, io_timeout_ms);
  sp_io_socket_writer_init(&conn->sock_writer, conn->socket, io_timeout_ms);
  conn->reader = &conn->sock_reader.base;
  conn->writer = &conn->sock_writer.base;

  if (proxy && url.tls) {
    sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
    sp_str_t connect_req = sp_fmt(scratch.mem,
      "CONNECT {}:{} HTTP/1.1\r\n"
      "Host: {}:{}\r\n"
      "\r\n",
      sp_fmt_str(url.host), sp_fmt_str(url.port), sp_fmt_str(url.host), sp_fmt_str(url.port)).value;
    err = sp_http_conn_write(conn, connect_req);
    sp_mem_end_scratch(scratch);
    if (err != SP_HTTP_OK) return SP_HTTP_ERR_PROXY;
    err = sp_http_connect_reply(conn);
    if (err != SP_HTTP_OK) return err;
  }

  if (!url.tls) return SP_HTTP_OK;

  err = tls->open(tls, conn->socket, sp_http_host_bare(url.host), io_timeout_ms);
  if (err != SP_HTTP_OK) return err;
  conn->tls = tls;
  conn->reader = tls->reader;
  conn->writer = tls->writer;
  return SP_HTTP_OK;
}

SP_PRIVATE void sp_http_conn_close(sp_http_conn_t* conn) {
  if (conn->tls) conn->tls->close(conn->tls);
  if (conn->socket != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(conn->socket);
  conn->tls = SP_NULLPTR;
  conn->socket = SP_SYS_INVALID_SOCKET;
}

sp_http_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response) {
  sp_http_response_t resp = sp_zero_s(sp_http_response_t);
  u32 max = request.max_redirects ? request.max_redirects : SP_HTTP_DEFAULT_REDIRECTS;
  u32 connect_timeout = sp_http_timeout_ms(request.connect_timeout_ms, SP_HTTP_DEFAULT_CONNECT_TIMEOUT_MS);
  u32 io_timeout = sp_http_timeout_ms(request.io_timeout_ms, SP_HTTP_DEFAULT_IO_TIMEOUT_MS);
  sp_http_resolver_t resolver = request.resolver.resolve ? request.resolver : sp_http_resolver_default();
  sp_str_t current = request.url;
  sp_http_error_t result = sp_http_headers_check(request.headers);
  u32 redirects = 0;

  if (result != SP_HTTP_OK) {
    if (response) *response = resp;
    return result;
  }

  sp_http_method_t method = request.method;
  sp_str_t payload = request.payload;
  sp_str_t content_type = request.content_type;
  sp_str_t origin_host = sp_zero;
  bool origin_tls = false;
  bool origin_set = false;
  result = SP_HTTP_ERR_PROTOCOL;

  for (;;) {
    sp_http_url_t url = sp_zero_s(sp_http_url_t);
    if (!sp_http_url_parse(current, &url)) {
      result = SP_HTTP_ERR_URL;
      break;
    }
    if (url.tls && !request.tls) {
      result = SP_HTTP_ERR_BAD_CONFIG;
      break;
    }
    if (!origin_set) {
      origin_host = url.host;
      origin_tls = url.tls;
      origin_set = true;
    }

    sp_str_t proxy_str = request.proxy;
    if (sp_str_empty(proxy_str) && !request.no_proxy) proxy_str = sp_http_proxy_from_env(url);
    sp_http_url_t proxy_url = sp_zero_s(sp_http_url_t);
    bool use_proxy = !sp_str_empty(proxy_str);
    if (use_proxy && !sp_http_proxy_url_parse(mem, proxy_str, &proxy_url)) {
      result = SP_HTTP_ERR_PROXY;
      break;
    }

    sp_http_conn_t conn = sp_zero_s(sp_http_conn_t);
    result = sp_http_conn_open(&conn, request.tls, resolver, url, use_proxy ? &proxy_url : SP_NULLPTR, connect_timeout, io_timeout);
    if (result != SP_HTTP_OK) {
      sp_http_conn_close(&conn);
      break;
    }

    sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(mem);
    result = sp_http_conn_write(&conn, sp_http_build_request(scratch.mem, (sp_http_wire_t) {
      .url           = url,
      .absolute_form = use_proxy && !url.tls,
      .method        = method,
      .payload       = payload,
      .content_type  = content_type,
      .headers       = request.headers,
      .strip_auth    = !sp_http_ci_equal(url.host, origin_host) || (origin_tls && !url.tls),
    }));
    sp_mem_end_scratch(scratch);
    if (result == SP_HTTP_OK && !sp_str_empty(payload)) {
      result = sp_http_conn_write(&conn, payload);
    }
    if (result != SP_HTTP_OK) {
      sp_http_conn_close(&conn);
      break;
    }

    sp_io_reader_set_buffer(conn.reader, conn.buffer, sizeof(conn.buffer));

    sp_http_head_t parsed = sp_zero;
    u32 interim = 0;
    for (;;) {
      sp_str_t head = sp_zero;
      result = sp_http_read_head(conn.reader, &head);
      if (result != SP_HTTP_OK) break;

      result = sp_http_parse_head(head, &parsed);
      if (result != SP_HTTP_OK) break;

      if (parsed.status >= 100 && parsed.status <= 199) {
        if (parsed.status == 101 || ++interim >= SP_HTTP_MAX_INTERIM) {
          result = SP_HTTP_ERR_PROTOCOL;
          break;
        }
        continue;
      }
      break;
    }
    if (result != SP_HTTP_OK) {
      sp_http_conn_close(&conn);
      break;
    }

    resp.status = parsed.status;
    resp.url = current;

    bool is_redirect =
      parsed.status == 301 || parsed.status == 302 || parsed.status == 303 ||
      parsed.status == 307 || parsed.status == 308;
    if (is_redirect && !sp_str_empty(parsed.location)) {
      if (redirects++ >= max) {
        result = SP_HTTP_ERR_REDIRECTS;
        sp_http_conn_close(&conn);
        break;
      }
      bool rewrite = parsed.status == 303 ||
        ((parsed.status == 301 || parsed.status == 302) && method != SP_HTTP_GET);
      if (rewrite) {
        method = SP_HTTP_GET;
        payload = sp_zero_s(sp_str_t);
        content_type = sp_zero_s(sp_str_t);
      }
      current = sp_http_resolve_url(mem, url, parsed.location);
      sp_http_conn_close(&conn);
      continue;
    }

    resp.content_type = sp_str_copy(mem, parsed.content_type);
    resp.location = sp_str_copy(mem, parsed.location);

    u64 written = 0;
    result = sp_http_read_body(conn.reader, parsed, request.sink, request.sink ? &written : SP_NULLPTR);
    resp.body_len = written;
    sp_http_conn_close(&conn);

    if (result != SP_HTTP_OK) break;
    if (parsed.status >= 400)                          result = SP_HTTP_ERR_STATUS;
    else if (parsed.status >= 200 && parsed.status < 300) result = SP_HTTP_OK;
    else                                               result = SP_HTTP_ERR_PROTOCOL;
    break;
  }

  if (response) *response = resp;
  return result;
}

#else

sp_http_resolver_t sp_http_resolver_default(void) {
  return sp_zero_s(sp_http_resolver_t);
}

sp_http_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response) {
  (void)mem; (void)request;
  if (response) *response = sp_zero_s(sp_http_response_t);
  return SP_HTTP_ERR_UNSUPPORTED;
}

#endif // SP_HTTP_SOCKETS

#if defined(SP_TLS_WITH_MBEDTLS)

#if defined(SP_MACOS) && !defined(SP_TLS_MACOS_SECTRUST) && defined(__has_include)
  #if __has_include(<Security/Security.h>)
    #error "sp_tls on macOS requires SecTrust: define SP_TLS_MACOS_SECTRUST and link -framework Security -framework CoreFoundation"
  #endif
#endif

#if defined(SP_WIN32)
#include <wincrypt.h>
#endif

#if defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

SP_PRIVATE u32 sp_tls_chain_count(const mbedtls_x509_crt* chain) {
  u32 count = 0;
  const mbedtls_x509_crt* it = chain;
  while (it) {
    count++;
    it = it->next;
  }
  return count;
}

sp_http_error_t sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem) {
  *trust = sp_zero_s(sp_tls_trust_t);
  trust->mem = mem;
  trust->backend = sp_tls_native_backend();
  trust->anchors = sp_alloc_type(mem, mbedtls_x509_crt);
  mbedtls_x509_crt_init(trust->anchors);
  return SP_HTTP_OK;
}

void sp_tls_trust_free(sp_tls_trust_t* trust) {
  if (trust->anchors) {
    mbedtls_x509_crt_free(trust->anchors);
    sp_free(trust->mem, trust->anchors, sizeof(mbedtls_x509_crt));
  }
  *trust = sp_zero_s(sp_tls_trust_t);
}

sp_http_error_t sp_tls_trust_load(sp_tls_trust_t* trust) {
  trust->backend = sp_tls_native_backend();
  switch (trust->backend) {
    case SP_TLS_BACKEND_ANCHORS:
#if defined(SP_WIN32)
      return sp_tls_load_windows(trust->anchors, trust->mem, &trust->loaded, &trust->skipped);
#else
      return sp_tls_load_unix(trust->anchors, trust->mem, &trust->loaded, &trust->skipped);
#endif
    case SP_TLS_BACKEND_OS_VERIFY:
      return SP_HTTP_OK;
    case SP_TLS_BACKEND_NONE:
      return SP_HTTP_ERR_UNSUPPORTED;
  }
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_str_t content = sp_zero;
  sp_http_error_t result = SP_HTTP_ERR_NO_STORE;
  if (sp_io_read_file(scratch.mem, path, &content) == SP_OK) {
    c8* pem = sp_str_to_cstr(scratch.mem, content);
    s32 rc = mbedtls_x509_crt_parse((mbedtls_x509_crt*)chain, (const unsigned char*)pem, (size_t)content.len + 1);
    if (rc < 0) {
      result = SP_HTTP_ERR_PARSE;
    }
    else {
      if (loaded)  *loaded = sp_tls_chain_count((mbedtls_x509_crt*)chain);
      if (skipped) *skipped = (u32)rc;
      result = SP_HTTP_OK;
    }
  }
  sp_mem_end_scratch(scratch);
  return result;
}

sp_http_error_t sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)mem;
  sp_str_t env = sp_os_env_get(sp_str_lit("SSL_CERT_FILE"));
  if (!sp_str_empty(env)) {
    return sp_tls_load_pem(chain, env, loaded, skipped);
  }
  const c8* candidates[] = {
    "/etc/ssl/certs/ca-certificates.crt",
    "/etc/pki/tls/certs/ca-bundle.crt",
    "/etc/ssl/cert.pem",
    "/etc/ssl/ca-bundle.pem",
    "/etc/pki/tls/cacert.pem",
  };
  sp_carr_for(candidates, it) {
    sp_str_t path = sp_cstr_as_str(candidates[it]);
    if (sp_fs_exists(path)) {
      return sp_tls_load_pem(chain, path, loaded, skipped);
    }
  }
  return SP_HTTP_ERR_NO_STORE;
}

#if defined(SP_WIN32)
SP_PRIVATE bool sp_tls_win32_server_auth(PCCERT_CONTEXT ctx) {
  DWORD size = 0;
  if (!CertGetEnhancedKeyUsage(ctx, 0, SP_NULLPTR, &size)) return false;
  bool result = false;
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  PCERT_ENHKEY_USAGE usage = (PCERT_ENHKEY_USAGE)sp_alloc(scratch.mem, size);
  if (CertGetEnhancedKeyUsage(ctx, 0, usage, &size)) {
    if (usage->cUsageIdentifier == 0) {
      result = GetLastError() == CRYPT_E_NOT_FOUND;
    }
    else {
      sp_for(it, usage->cUsageIdentifier) {
        if (sp_cstr_equal(usage->rgpszUsageIdentifier[it], "1.3.6.1.5.5.7.3.1")) {
          result = true;
          break;
        }
      }
    }
  }
  sp_mem_end_scratch(scratch);
  return result;
}

sp_http_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)mem;
  mbedtls_x509_crt* certs = (mbedtls_x509_crt*)chain;
  u32 skip = 0;
  HCERTSTORE store = CertOpenSystemStoreA(0, "ROOT");
  if (!store) return SP_HTTP_ERR_NO_STORE;
  PCCERT_CONTEXT ctx = SP_NULLPTR;
  while ((ctx = CertEnumCertificatesInStore(store, ctx)) != SP_NULLPTR) {
    if (!(ctx->dwCertEncodingType & X509_ASN_ENCODING)) continue;
    if (!sp_tls_win32_server_auth(ctx)) {
      skip++;
      continue;
    }
    if (mbedtls_x509_crt_parse_der(certs, ctx->pbCertEncoded, ctx->cbCertEncoded) != 0) skip++;
  }
  CertCloseStore(store, 0);
  if (loaded)  *loaded = sp_tls_chain_count(certs);
  if (skipped) *skipped = skip;
  return sp_tls_chain_count(certs) ? SP_HTTP_OK : SP_HTTP_ERR_NO_STORE;
}
#else
sp_http_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_HTTP_ERR_UNSUPPORTED;
}
#endif

sp_http_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf) {
  mbedtls_ssl_config* cfg = (mbedtls_ssl_config*)conf;
  switch (trust->backend) {
    case SP_TLS_BACKEND_ANCHORS:
      mbedtls_ssl_conf_authmode(cfg, MBEDTLS_SSL_VERIFY_REQUIRED);
      mbedtls_ssl_conf_ca_chain(cfg, (mbedtls_x509_crt*)trust->anchors, SP_NULLPTR);
      return SP_HTTP_OK;
    case SP_TLS_BACKEND_OS_VERIFY:
      mbedtls_ssl_conf_authmode(cfg, MBEDTLS_SSL_VERIFY_REQUIRED);
      mbedtls_ssl_conf_ca_chain(cfg, (mbedtls_x509_crt*)trust->anchors, SP_NULLPTR);
      return SP_HTTP_OK;
    case SP_TLS_BACKEND_NONE:
      return SP_HTTP_ERR_UNSUPPORTED;
  }
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname) {
  mbedtls_ssl_context* context = (mbedtls_ssl_context*)ssl;
  if (sp_str_empty(hostname) || hostname.len >= SP_PATH_MAX) return SP_HTTP_ERR_URL;
  c8 host[SP_PATH_MAX];
  sp_cstr_copy_to_n(hostname.data, hostname.len, host, sizeof(host));
  if (mbedtls_ssl_set_hostname(context, host) != 0) return SP_HTTP_ERR_OS;
  if (trust->backend == SP_TLS_BACKEND_OS_VERIFY) {
#if !defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
    return SP_HTTP_ERR_BAD_CONFIG;
#else
    if (verify) verify->hostname = hostname;
    mbedtls_ssl_set_verify(context, sp_tls_verify_cb, verify);
#endif
  }
  return SP_HTTP_OK;
}

s32 sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags) {
  *flags = 0;
  if (depth != 0) return 0;
  sp_tls_verify_t* verify = (sp_tls_verify_t*)user_data;
  sp_str_t hostname = verify ? verify->hostname : sp_zero_s(sp_str_t);
#if defined(SP_WIN32)
  sp_http_error_t err = sp_tls_windows_eval(crt, hostname);
#else
  sp_http_error_t err = sp_tls_macos_eval(crt, hostname);
#endif
  if (err != SP_HTTP_OK) {
    *flags = MBEDTLS_X509_BADCERT_NOT_TRUSTED;
  }
  return 0;
}

sp_http_error_t sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
#if defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
  if (sp_str_empty(hostname) || hostname.len >= SP_PATH_MAX) return SP_HTTP_ERR_UNTRUSTED;

  CFMutableArrayRef certs = CFArrayCreateMutable(SP_NULLPTR, 0, &kCFTypeArrayCallBacks);
  if (!certs) return SP_HTTP_ERR_OS;

  bool converted = true;
  const mbedtls_x509_crt* it = (const mbedtls_x509_crt*)chain;
  while (it) {
    CFDataRef der = CFDataCreate(SP_NULLPTR, it->raw.p, (CFIndex)it->raw.len);
    SecCertificateRef cert = der ? SecCertificateCreateWithData(SP_NULLPTR, der) : SP_NULLPTR;
    if (der) CFRelease(der);
    if (!cert) {
      converted = false;
      break;
    }
    CFArrayAppendValue(certs, cert);
    CFRelease(cert);
    it = it->next;
  }

  c8 host[SP_PATH_MAX];
  sp_cstr_copy_to_n(hostname.data, hostname.len, host, sizeof(host));
  CFStringRef cfhost = CFStringCreateWithCString(SP_NULLPTR, host, kCFStringEncodingUTF8);
  SecPolicyRef policy = cfhost ? SecPolicyCreateSSL(true, cfhost) : SP_NULLPTR;

  SecTrustRef trust = SP_NULLPTR;
  sp_http_error_t result = SP_HTTP_ERR_UNTRUSTED;
  if (converted && CFArrayGetCount(certs) > 0 && policy) {
    if (SecTrustCreateWithCertificates(certs, policy, &trust) == errSecSuccess) {
      CFErrorRef error = SP_NULLPTR;
      if (SecTrustEvaluateWithError(trust, &error)) result = SP_HTTP_OK;
      if (error) CFRelease(error);
    }
  }

  if (trust) CFRelease(trust);
  if (policy) CFRelease(policy);
  if (cfhost) CFRelease(cfhost);
  CFRelease(certs);
  return result;
#else
  (void)chain; (void)hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
#endif
}

sp_http_error_t sp_tls_windows_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
#if defined(SP_WIN32)
  const mbedtls_x509_crt* leaf = (const mbedtls_x509_crt*)chain;
  if (!leaf) return SP_HTTP_ERR_UNTRUSTED;
  if (sp_str_empty(hostname) || hostname.len >= SP_PATH_MAX) return SP_HTTP_ERR_UNTRUSTED;

  WCHAR wide[SP_PATH_MAX];
  s32 wide_len = MultiByteToWideChar(CP_UTF8, 0, hostname.data, (int)hostname.len, wide, SP_PATH_MAX - 1);
  if (wide_len <= 0) return SP_HTTP_ERR_UNTRUSTED;
  wide[wide_len] = 0;

  HCERTSTORE extra = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, SP_NULLPTR);
  if (!extra) return SP_HTTP_ERR_OS;

  sp_http_error_t result = SP_HTTP_ERR_OS;
  PCCERT_CHAIN_CONTEXT verdict = SP_NULLPTR;
  PCCERT_CONTEXT ctx = CertCreateCertificateContext(X509_ASN_ENCODING, leaf->raw.p, (DWORD)leaf->raw.len);

  bool converted = ctx != SP_NULLPTR;
  const mbedtls_x509_crt* it = leaf->next;
  while (converted && it) {
    converted = CertAddEncodedCertificateToStore(extra, X509_ASN_ENCODING, it->raw.p, (DWORD)it->raw.len, CERT_STORE_ADD_REPLACE_EXISTING, SP_NULLPTR) != 0;
    it = it->next;
  }

  LPSTR usages[] = { (LPSTR)szOID_PKIX_KP_SERVER_AUTH };
  CERT_CHAIN_PARA para = sp_zero;
  para.cbSize = sizeof(para);
  para.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
  para.RequestedUsage.Usage.cUsageIdentifier = 1;
  para.RequestedUsage.Usage.rgpszUsageIdentifier = usages;

  if (converted && CertGetCertificateChain(SP_NULLPTR, ctx, SP_NULLPTR, extra, &para, CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT, SP_NULLPTR, &verdict)) {
    SSL_EXTRA_CERT_CHAIN_POLICY_PARA ssl = sp_zero;
    ssl.cbSize = sizeof(ssl);
    ssl.dwAuthType = AUTHTYPE_SERVER;
    ssl.pwszServerName = wide;
    CERT_CHAIN_POLICY_PARA policy = sp_zero;
    policy.cbSize = sizeof(policy);
    policy.dwFlags = CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS;
    policy.pvExtraPolicyPara = &ssl;
    CERT_CHAIN_POLICY_STATUS status = sp_zero;
    status.cbSize = sizeof(status);
    if (CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, verdict, &policy, &status)) {
      result = status.dwError == 0 ? SP_HTTP_OK : SP_HTTP_ERR_UNTRUSTED;
    }
  }

  if (verdict) CertFreeCertificateChain(verdict);
  if (ctx) CertFreeCertificateContext(ctx);
  CertCloseStore(extra, 0);
  return result;
#else
  (void)chain; (void)hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
#endif
}

sp_http_error_t sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count) {
  u32 total = sp_tls_chain_count((const mbedtls_x509_crt*)chain);
  sp_mem_slice_t* out = sp_alloc_n(mem, sp_mem_slice_t, total);
  const mbedtls_x509_crt* it = (const mbedtls_x509_crt*)chain;
  u32 index = 0;
  while (it) {
    out[index].data = it->raw.p;
    out[index].len = it->raw.len;
    it = it->next;
    index++;
  }
  if (ders)  *ders = out;
  if (count) *count = total;
  return SP_HTTP_OK;
}

// mbedtls returns WANT_READ/WANT_WRITE when the nonblocking socket has no
// data/space; park on the socket until it's ready or the io timeout expires.
SP_PRIVATE sp_err_t sp_tls_mbedtls_wait(sp_tls_mbedtls_t* tls, s32 rc) {
  bool readable = rc == MBEDTLS_ERR_SSL_WANT_READ;
  sp_err_t wait = sp_sys_socket_wait(tls->socket, readable, tls->io_timeout_ms);
  if (wait == SP_ERR_SYS_TIMED_OUT) return SP_ERR_IO_TIMEOUT;
  return wait;
}

SP_PRIVATE sp_err_t sp_tls_mbedtls_read(sp_io_reader_t* reader, void* ptr, u64 size, u64* bytes_read) {
  sp_tls_mbedtls_reader_t* wrapper = (sp_tls_mbedtls_reader_t*)reader;
  sp_tls_mbedtls_t* tls = wrapper->tls;
  if (bytes_read) *bytes_read = 0;
  if (wrapper->eof) return SP_ERR_IO_EOF;
  for (;;) {
    s32 n = mbedtls_ssl_read(&tls->ssl, (unsigned char*)ptr, (size_t)sp_min(size, (u64)INT32_MAX));
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) {
      sp_err_t err = sp_tls_mbedtls_wait(tls, n);
      if (err != SP_OK) return err;
      continue;
    }
    if (n == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
      // latched: mbedtls reports close_notify only once, then raw EOF
      wrapper->eof = true;
      return SP_ERR_IO_EOF;
    }
    if (n == MBEDTLS_ERR_SSL_TIMEOUT) return SP_ERR_IO_TIMEOUT;
    if (n == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) continue;
    // a raw EOF without close_notify could be a truncation attack, so it is
    // an error, not end-of-stream
    if (n <= 0) return SP_ERR_IO;
    if (bytes_read) *bytes_read = (u64)n;
    return SP_OK;
  }
}

SP_PRIVATE sp_err_t sp_tls_mbedtls_write(sp_io_writer_t* writer, const void* ptr, u64 size, u64* bytes_written) {
  sp_tls_mbedtls_t* tls = ((sp_tls_mbedtls_writer_t*)writer)->tls;
  if (bytes_written) *bytes_written = 0;
  for (;;) {
    s32 n = mbedtls_ssl_write(&tls->ssl, (const unsigned char*)ptr, (size_t)sp_min(size, (u64)INT32_MAX));
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) {
      sp_err_t err = sp_tls_mbedtls_wait(tls, n);
      if (err != SP_OK) return err;
      continue;
    }
    if (n <= 0) return SP_ERR_IO;
    if (bytes_written) *bytes_written = (u64)n;
    return SP_OK;
  }
}

SP_PRIVATE sp_http_error_t sp_tls_mbedtls_handshake(sp_tls_mbedtls_t* tls, sp_str_t hostname) {
  if (mbedtls_ctr_drbg_seed(&tls->drbg, mbedtls_entropy_func, &tls->entropy, SP_NULLPTR, 0) != 0) return SP_HTTP_ERR_OS;
  if (mbedtls_ssl_config_defaults(&tls->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) return SP_HTTP_ERR_OS;
  mbedtls_ssl_conf_min_tls_version(&tls->conf, MBEDTLS_SSL_VERSION_TLS1_2);
  mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->drbg);
  mbedtls_ssl_conf_read_timeout(&tls->conf, tls->io_timeout_ms);
  if (sp_tls_conf_apply(tls->trust, &tls->conf) != SP_HTTP_OK) return SP_HTTP_ERR_BAD_CONFIG;
  if (mbedtls_ssl_setup(&tls->ssl, &tls->conf) != 0) return SP_HTTP_ERR_OS;
  if (sp_tls_ssl_attach(tls->trust, &tls->ssl, &tls->verify, hostname) != SP_HTTP_OK) return SP_HTTP_ERR_BAD_CONFIG;
  mbedtls_ssl_set_bio(&tls->ssl, &tls->net, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

  s32 rc;
  while ((rc = mbedtls_ssl_handshake(&tls->ssl)) != 0) {
    if (rc == MBEDTLS_ERR_SSL_TIMEOUT) return SP_HTTP_ERR_TIMEOUT;
    if (rc == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) return SP_HTTP_ERR_UNTRUSTED;
    if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) return SP_HTTP_ERR_HANDSHAKE;
    sp_err_t err = sp_tls_mbedtls_wait(tls, rc);
    if (err == SP_ERR_IO_TIMEOUT) return SP_HTTP_ERR_TIMEOUT;
    if (err != SP_OK) return SP_HTTP_ERR_OS;
  }
  return SP_HTTP_OK;
}

SP_PRIVATE void sp_tls_mbedtls_drop(sp_tls_mbedtls_t* tls) {
  tls->net.fd = -1;
  mbedtls_ssl_free(&tls->ssl);
  mbedtls_ssl_config_free(&tls->conf);
  mbedtls_ctr_drbg_free(&tls->drbg);
  mbedtls_entropy_free(&tls->entropy);
  mbedtls_net_free(&tls->net);
}

SP_PRIVATE sp_http_error_t sp_tls_mbedtls_open(sp_tls_t* base, sp_sys_socket_t socket, sp_str_t hostname, u32 io_timeout_ms) {
  sp_tls_mbedtls_t* tls = (sp_tls_mbedtls_t*)base;
  mbedtls_net_init(&tls->net);
  mbedtls_ssl_init(&tls->ssl);
  mbedtls_ssl_config_init(&tls->conf);
  mbedtls_entropy_init(&tls->entropy);
  mbedtls_ctr_drbg_init(&tls->drbg);
  tls->verify = sp_zero_s(sp_tls_verify_t);
  tls->socket = socket;
  tls->io_timeout_ms = io_timeout_ms;
  tls->net.fd = (int)socket;
  tls->reader.eof = false;

  sp_http_error_t err = sp_tls_mbedtls_handshake(tls, hostname);
  if (err != SP_HTTP_OK) {
    sp_tls_mbedtls_drop(tls);
    return err;
  }
  tls->base.reader = &tls->reader.base;
  tls->base.writer = &tls->writer.base;
  return SP_HTTP_OK;
}

SP_PRIVATE void sp_tls_mbedtls_close(sp_tls_t* base) {
  sp_tls_mbedtls_t* tls = (sp_tls_mbedtls_t*)base;
  mbedtls_ssl_close_notify(&tls->ssl);
  sp_tls_mbedtls_drop(tls);
}

void sp_tls_mbedtls_init(sp_tls_mbedtls_t* tls, const sp_tls_trust_t* trust) {
  *tls = sp_zero_s(sp_tls_mbedtls_t);
  tls->base.open = sp_tls_mbedtls_open;
  tls->base.close = sp_tls_mbedtls_close;
  tls->trust = trust;
  tls->reader.base.read = sp_tls_mbedtls_read;
  tls->reader.tls = tls;
  tls->writer.base.write = sp_tls_mbedtls_write;
  tls->writer.tls = tls;
}

#else

sp_http_error_t sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem) {
  *trust = sp_zero_s(sp_tls_trust_t);
  trust->mem = mem;
  trust->backend = sp_tls_native_backend();
  return SP_HTTP_OK;
}

void sp_tls_trust_free(sp_tls_trust_t* trust) {
  *trust = sp_zero_s(sp_tls_trust_t);
}

sp_http_error_t sp_tls_trust_load(sp_tls_trust_t* trust) {
  trust->backend = sp_tls_native_backend();
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped) {
  (void)chain; (void)path;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf) {
  (void)trust; (void)conf;
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname) {
  (void)trust; (void)ssl;
  if (verify) verify->hostname = hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
}

s32 sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags) {
  (void)user_data; (void)crt; (void)depth; (void)flags;
  return 0;
}

sp_http_error_t sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
  (void)chain; (void)hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_windows_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
  (void)chain; (void)hostname;
  return SP_HTTP_ERR_UNSUPPORTED;
}

sp_http_error_t sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count) {
  (void)chain; (void)mem;
  if (ders)  *ders = SP_NULLPTR;
  if (count) *count = 0;
  return SP_HTTP_ERR_UNSUPPORTED;
}

#endif // SP_TLS_WITH_MBEDTLS

#endif // SP_HTTP_IMPLEMENTATION
#endif // SP_HTTP_C
