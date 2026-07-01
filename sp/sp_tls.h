
#if defined SP_IMPLEMENTATION && !defined(SP_TLS_IMPLEMENTATION)
  #define SP_TLS_IMPLEMENTATION
#endif

#ifndef SP_TLS_H
#define SP_TLS_H

#include "sp.h"

struct mbedtls_x509_crt;
struct mbedtls_ssl_config;
struct mbedtls_ssl_context;

typedef enum {
  SP_TLS_OK = 0,
  SP_TLS_ERR_NO_STORE,
  SP_TLS_ERR_PARSE,
  SP_TLS_ERR_OS,
  SP_TLS_ERR_UNTRUSTED,
  SP_TLS_ERR_BAD_CONFIG,
  SP_TLS_ERR_UNSUPPORTED,
  SP_TLS_ERR_URL,
  SP_TLS_ERR_CONNECT,
  SP_TLS_ERR_HANDSHAKE,
  SP_TLS_ERR_PROTOCOL,
  SP_TLS_ERR_STATUS,
  SP_TLS_ERR_REDIRECTS,
} sp_tls_error_t;

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


SP_API sp_tls_error_t   sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem);
SP_API void             sp_tls_trust_free(sp_tls_trust_t* trust);
SP_API sp_tls_error_t   sp_tls_trust_load(sp_tls_trust_t* trust);


SP_API sp_tls_error_t   sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped);
SP_API sp_tls_error_t   sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped);
SP_API sp_tls_error_t   sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped);


SP_API sp_tls_error_t   sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf);
SP_API sp_tls_error_t   sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname);


SP_API s32              sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags);
SP_API sp_tls_error_t   sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname);


SP_API sp_tls_error_t   sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count);


// @http
#define SP_HTTP_DEFAULT_REDIRECTS 16

typedef struct {
  sp_str_t scheme;
  sp_str_t host;
  sp_str_t port;
  sp_str_t path;
  bool     tls;
} sp_http_url_t;

typedef struct {
  sp_str_t        url;
  sp_tls_trust_t* trust;
  sp_io_writer_t* body;
  u32             max_redirects;
} sp_http_request_t;

typedef struct {
  s32      status;
  u64      body_len;
  sp_str_t url;
} sp_http_response_t;

SP_API bool           sp_http_url_parse(sp_str_t url, sp_http_url_t* out);
SP_API sp_tls_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response);

#endif


#if defined(SP_TLS_IMPLEMENTATION)

sp_tls_backend_t sp_tls_native_backend(void) {
#if defined(SP_WIN32)
  return SP_TLS_BACKEND_ANCHORS;
#elif defined(SP_MACOS)
  return SP_TLS_BACKEND_OS_VERIFY;
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
  sp_for_range(it, 0, (s32)(haystack.len - needle.len) + 1) {
    if (sp_http_ci_equal(sp_str_sub(haystack, it, (s32)needle.len), needle)) return true;
  }
  return false;
}

SP_PRIVATE sp_str_t sp_http_str_tail(sp_str_t str, s32 from) {
  return sp_str_sub(str, from, (s32)str.len - from);
}

bool sp_http_url_parse(sp_str_t url, sp_http_url_t* out) {
  *out = sp_zero_s(sp_http_url_t);

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
  sp_str_t authority;
  if (slash == SP_STR_NO_MATCH) {
    authority = rest;
    out->path = sp_str_lit("/");
  }
  else {
    authority = sp_str_sub(rest, 0, slash);
    out->path = sp_http_str_tail(rest, slash);
  }

  s32 colon = sp_str_find_c8(authority, ':');
  if (colon == SP_STR_NO_MATCH) {
    out->host = authority;
    out->port = out->tls ? sp_str_lit("443") : sp_str_lit("80");
  }
  else {
    out->host = sp_str_sub(authority, 0, colon);
    out->port = sp_http_str_tail(authority, colon + 1);
  }

  return !sp_str_empty(out->host) && !sp_str_empty(out->port);
}

#if defined(SP_TLS_WITH_MBEDTLS)

#include <mbedtls/x509_crt.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

#if defined(SP_WIN32)
#include <windows.h>
#include <wincrypt.h>
#elif defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
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

sp_tls_error_t sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem) {
  *trust = sp_zero_s(sp_tls_trust_t);
  trust->mem = mem;
  trust->backend = sp_tls_native_backend();
  if (trust->backend != SP_TLS_BACKEND_NONE) {
    trust->anchors = sp_alloc_type(mem, mbedtls_x509_crt);
    mbedtls_x509_crt_init(trust->anchors);
  }
  return SP_TLS_OK;
}

void sp_tls_trust_free(sp_tls_trust_t* trust) {
  if (trust->anchors) {
    mbedtls_x509_crt_free(trust->anchors);
    sp_free(trust->mem, trust->anchors, sizeof(mbedtls_x509_crt));
  }
  *trust = sp_zero_s(sp_tls_trust_t);
}

sp_tls_error_t sp_tls_trust_load(sp_tls_trust_t* trust) {
  trust->backend = sp_tls_native_backend();
  switch (trust->backend) {
    case SP_TLS_BACKEND_ANCHORS:
#if defined(SP_WIN32)
      return sp_tls_load_windows(trust->anchors, trust->mem, &trust->loaded, &trust->skipped);
#else
      return sp_tls_load_unix(trust->anchors, trust->mem, &trust->loaded, &trust->skipped);
#endif
    case SP_TLS_BACKEND_OS_VERIFY:
      return SP_TLS_OK;
    case SP_TLS_BACKEND_NONE:
      return SP_TLS_ERR_UNSUPPORTED;
  }
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_str_t content = sp_zero;
  sp_tls_error_t result = SP_TLS_ERR_NO_STORE;
  if (sp_io_read_file(scratch.mem, path, &content) == SP_OK) {
    c8* pem = sp_str_to_cstr(scratch.mem, content);
    s32 rc = mbedtls_x509_crt_parse((mbedtls_x509_crt*)chain, (const unsigned char*)pem, (size_t)content.len + 1);
    if (rc < 0) {
      result = SP_TLS_ERR_PARSE;
    }
    else {
      if (loaded)  *loaded = sp_tls_chain_count((mbedtls_x509_crt*)chain);
      if (skipped) *skipped = (u32)rc;
      result = SP_TLS_OK;
    }
  }
  sp_mem_end_scratch(scratch);
  return result;
}

sp_tls_error_t sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)mem;
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
  return SP_TLS_ERR_NO_STORE;
}

#if defined(SP_WIN32)
sp_tls_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)mem;
  mbedtls_x509_crt* certs = (mbedtls_x509_crt*)chain;
  const c8* stores[] = { "ROOT", "CA" };
  u32 skip = 0;
  sp_carr_for(stores, si) {
    HCERTSTORE store = CertOpenSystemStoreA(0, stores[si]);
    if (!store) continue;
    PCCERT_CONTEXT ctx = SP_NULLPTR;
    while ((ctx = CertEnumCertificatesInStore(store, ctx)) != SP_NULLPTR) {
      if (ctx->dwCertEncodingType & X509_ASN_ENCODING) {
        if (mbedtls_x509_crt_parse_der(certs, ctx->pbCertEncoded, ctx->cbCertEncoded) != 0) skip++;
      }
    }
    CertCloseStore(store, 0);
  }
  if (loaded)  *loaded = sp_tls_chain_count(certs);
  if (skipped) *skipped = skip;
  return sp_tls_chain_count(certs) ? SP_TLS_OK : SP_TLS_ERR_NO_STORE;
}
#else
sp_tls_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_ERR_UNSUPPORTED;
}
#endif

sp_tls_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf) {
  mbedtls_ssl_config* cfg = (mbedtls_ssl_config*)conf;
  switch (trust->backend) {
    case SP_TLS_BACKEND_ANCHORS:
      mbedtls_ssl_conf_authmode(cfg, MBEDTLS_SSL_VERIFY_REQUIRED);
      mbedtls_ssl_conf_ca_chain(cfg, (mbedtls_x509_crt*)trust->anchors, SP_NULLPTR);
      return SP_TLS_OK;
    case SP_TLS_BACKEND_OS_VERIFY:
      mbedtls_ssl_conf_authmode(cfg, MBEDTLS_SSL_VERIFY_REQUIRED);
      mbedtls_ssl_conf_ca_chain(cfg, (mbedtls_x509_crt*)trust->anchors, SP_NULLPTR);
      return SP_TLS_OK;
    case SP_TLS_BACKEND_NONE:
      return SP_TLS_ERR_UNSUPPORTED;
  }
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname) {
  mbedtls_ssl_context* context = (mbedtls_ssl_context*)ssl;
  c8 host[SP_PATH_MAX];
  sp_cstr_copy_to_n(hostname.data, hostname.len, host, sizeof(host));
  if (mbedtls_ssl_set_hostname(context, host) != 0) return SP_TLS_ERR_OS;
  if (trust->backend == SP_TLS_BACKEND_OS_VERIFY) {
#if !defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
    return SP_TLS_ERR_BAD_CONFIG;
#else
    if (verify) verify->hostname = hostname;
    mbedtls_ssl_set_verify(context, sp_tls_verify_cb, verify);
#endif
  }
  return SP_TLS_OK;
}

s32 sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags) {
  *flags = 0;
  if (depth != 0) return 0;
  sp_tls_verify_t* verify = (sp_tls_verify_t*)user_data;
  sp_str_t hostname = verify ? verify->hostname : sp_zero_s(sp_str_t);
  if (sp_tls_macos_eval(crt, hostname) != SP_TLS_OK) {
    *flags = MBEDTLS_X509_BADCERT_NOT_TRUSTED;
  }
  return 0;
}

sp_tls_error_t sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
#if defined(SP_MACOS) && defined(SP_TLS_MACOS_SECTRUST)
  CFMutableArrayRef certs = CFArrayCreateMutable(SP_NULLPTR, 0, &kCFTypeArrayCallBacks);
  const mbedtls_x509_crt* it = (const mbedtls_x509_crt*)chain;
  while (it) {
    CFDataRef der = CFDataCreate(SP_NULLPTR, it->raw.p, (CFIndex)it->raw.len);
    SecCertificateRef cert = SecCertificateCreateWithData(SP_NULLPTR, der);
    CFRelease(der);
    if (cert) {
      CFArrayAppendValue(certs, cert);
      CFRelease(cert);
    }
    it = it->next;
  }

  c8 host[SP_PATH_MAX];
  sp_cstr_copy_to_n(hostname.data, hostname.len, host, sizeof(host));
  CFStringRef cfhost = CFStringCreateWithCString(SP_NULLPTR, host, kCFStringEncodingUTF8);
  SecPolicyRef policy = SecPolicyCreateSSL(true, cfhost);

  SecTrustRef trust = SP_NULLPTR;
  sp_tls_error_t result = SP_TLS_ERR_UNTRUSTED;
  if (SecTrustCreateWithCertificates(certs, policy, &trust) == errSecSuccess) {
    CFErrorRef error = SP_NULLPTR;
    if (SecTrustEvaluateWithError(trust, &error)) result = SP_TLS_OK;
    if (error) CFRelease(error);
  }

  if (trust) CFRelease(trust);
  if (policy) CFRelease(policy);
  if (cfhost) CFRelease(cfhost);
  CFRelease(certs);
  return result;
#else
  (void)chain; (void)hostname;
  return SP_TLS_ERR_UNSUPPORTED;
#endif
}

sp_tls_error_t sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count) {
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
  return SP_TLS_OK;
}

#define SP_HTTP_BUFFER_SIZE 16384
#define SP_HTTP_LINE_MAX    128

typedef struct {
  mbedtls_net_context     net;
  mbedtls_ssl_context     ssl;
  mbedtls_ssl_config      conf;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context drbg;
  sp_tls_verify_t         verify;
  bool                    tls;
} sp_http_conn_t;

typedef struct {
  sp_http_conn_t* conn;
  u8   buf[SP_HTTP_BUFFER_SIZE];
  u32  len;
  u32  pos;
  bool eof;
} sp_http_reader_t;

typedef struct {
  s32      status;
  sp_str_t location;
  bool     chunked;
  bool     has_length;
  u64      length;
} sp_http_head_t;

SP_PRIVATE s32 sp_http_conn_read(sp_http_conn_t* conn, u8* buf, u32 len) {
  for (;;) {
    s32 n = conn->tls
      ? mbedtls_ssl_read(&conn->ssl, buf, len)
      : mbedtls_net_recv(&conn->net, buf, len);
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
    if (n == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) return 0;
    return n;
  }
}

SP_PRIVATE sp_tls_error_t sp_http_conn_write(sp_http_conn_t* conn, sp_str_t data) {
  u32 sent = 0;
  while (sent < data.len) {
    s32 n = conn->tls
      ? mbedtls_ssl_write(&conn->ssl, (const unsigned char*)data.data + sent, data.len - sent)
      : mbedtls_net_send(&conn->net, (const unsigned char*)data.data + sent, data.len - sent);
    if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
    if (n <= 0) return SP_TLS_ERR_OS;
    sent += (u32)n;
  }
  return SP_TLS_OK;
}

SP_PRIVATE sp_tls_error_t sp_http_conn_open(sp_http_conn_t* conn, const sp_tls_trust_t* trust, sp_http_url_t url) {
  mbedtls_net_init(&conn->net);
  mbedtls_ssl_init(&conn->ssl);
  mbedtls_ssl_config_init(&conn->conf);
  mbedtls_entropy_init(&conn->entropy);
  mbedtls_ctr_drbg_init(&conn->drbg);
  conn->verify = sp_zero_s(sp_tls_verify_t);
  conn->tls = url.tls;

  c8 host[SP_PATH_MAX];
  c8 port[16];
  sp_cstr_copy_to_n(url.host.data, url.host.len, host, sizeof(host));
  sp_cstr_copy_to_n(url.port.data, url.port.len, port, sizeof(port));

  if (mbedtls_net_connect(&conn->net, host, port, MBEDTLS_NET_PROTO_TCP) != 0) return SP_TLS_ERR_CONNECT;
  if (!conn->tls) return SP_TLS_OK;

  if (mbedtls_ctr_drbg_seed(&conn->drbg, mbedtls_entropy_func, &conn->entropy, SP_NULLPTR, 0) != 0) return SP_TLS_ERR_OS;
  mbedtls_ssl_config_defaults(&conn->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
  mbedtls_ssl_conf_rng(&conn->conf, mbedtls_ctr_drbg_random, &conn->drbg);
  if (sp_tls_conf_apply(trust, &conn->conf) != SP_TLS_OK) return SP_TLS_ERR_BAD_CONFIG;
  if (mbedtls_ssl_setup(&conn->ssl, &conn->conf) != 0) return SP_TLS_ERR_OS;
  if (sp_tls_ssl_attach(trust, &conn->ssl, &conn->verify, url.host) != SP_TLS_OK) return SP_TLS_ERR_BAD_CONFIG;
  mbedtls_ssl_set_bio(&conn->ssl, &conn->net, mbedtls_net_send, mbedtls_net_recv, SP_NULLPTR);

  s32 rc;
  while ((rc = mbedtls_ssl_handshake(&conn->ssl)) != 0) {
    if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) return SP_TLS_ERR_HANDSHAKE;
  }
  return SP_TLS_OK;
}

SP_PRIVATE void sp_http_conn_close(sp_http_conn_t* conn) {
  if (conn->tls) mbedtls_ssl_close_notify(&conn->ssl);
  mbedtls_ssl_free(&conn->ssl);
  mbedtls_ssl_config_free(&conn->conf);
  mbedtls_ctr_drbg_free(&conn->drbg);
  mbedtls_entropy_free(&conn->entropy);
  mbedtls_net_free(&conn->net);
}

SP_PRIVATE bool sp_http_reader_fill(sp_http_reader_t* reader) {
  if (reader->pos < reader->len) return true;
  if (reader->eof) return false;
  s32 n = sp_http_conn_read(reader->conn, reader->buf, sizeof(reader->buf));
  if (n <= 0) {
    reader->eof = true;
    return false;
  }
  reader->len = (u32)n;
  reader->pos = 0;
  return true;
}

SP_PRIVATE bool sp_http_read_line(sp_http_reader_t* reader, c8* buf, u32 cap, u32* out_len) {
  u32 n = 0;
  for (;;) {
    if (!sp_http_reader_fill(reader)) return false;
    c8 c = (c8)reader->buf[reader->pos++];
    if (c == '\n') break;
    if (c == '\r') continue;
    if (n < cap) buf[n++] = c;
  }
  *out_len = n;
  return true;
}

SP_PRIVATE sp_tls_error_t sp_http_copy_n(sp_http_reader_t* reader, sp_io_writer_t* body, u64 n, u64* written) {
  while (n > 0) {
    if (!sp_http_reader_fill(reader)) return SP_TLS_ERR_PROTOCOL;
    u32 avail = reader->len - reader->pos;
    u32 take = (u64)avail < n ? avail : (u32)n;
    sp_io_write(body, reader->buf + reader->pos, take, SP_NULLPTR);
    reader->pos += take;
    n -= take;
    if (written) *written += take;
  }
  return SP_TLS_OK;
}

SP_PRIVATE sp_tls_error_t sp_http_read_head(sp_http_reader_t* reader, sp_mem_t mem, sp_str_t* head, sp_str_t* leftover) {
  sp_io_dyn_mem_writer_t writer = sp_zero;
  sp_io_dyn_mem_writer_init(mem, &writer);
  for (;;) {
    if (!sp_http_reader_fill(reader)) return SP_TLS_ERR_PROTOCOL;
    u32 avail = reader->len - reader->pos;
    sp_io_write(&writer.base, reader->buf + reader->pos, avail, SP_NULLPTR);
    reader->pos = reader->len;

    sp_str_t acc = sp_io_dyn_mem_writer_as_str(&writer);
    s32 term = sp_str_find(acc, sp_str_lit("\r\n\r\n"));
    if (term != SP_STR_NO_MATCH) {
      *head = sp_str_sub(acc, 0, term);
      *leftover = sp_http_str_tail(acc, term + 4);
      return SP_TLS_OK;
    }
  }
}

SP_PRIVATE sp_tls_error_t sp_http_parse_head(sp_str_t head, sp_http_head_t* out) {
  *out = sp_zero_s(sp_http_head_t);

  sp_str_t rest = head;
  bool first = true;
  for (;;) {
    s32 nl = sp_str_find(rest, sp_str_lit("\r\n"));
    sp_str_t line = nl == SP_STR_NO_MATCH ? rest : sp_str_sub(rest, 0, nl);

    if (first) {
      s32 sp1 = sp_str_find_c8(line, ' ');
      if (sp1 == SP_STR_NO_MATCH) return SP_TLS_ERR_PROTOCOL;
      sp_str_t after = sp_str_trim_left(sp_http_str_tail(line, sp1 + 1));
      s32 sp2 = sp_str_find_c8(after, ' ');
      sp_str_t code = sp2 == SP_STR_NO_MATCH ? after : sp_str_sub(after, 0, sp2);
      u32 status = 0;
      if (!sp_parse_u32_ex(code, &status)) return SP_TLS_ERR_PROTOCOL;
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
        else if (sp_http_ci_equal(name, sp_str_lit("content-length"))) {
          out->has_length = sp_parse_u64_ex(value, &out->length);
        }
        else if (sp_http_ci_equal(name, sp_str_lit("transfer-encoding"))) {
          out->chunked = sp_http_ci_contains(value, sp_str_lit("chunked"));
        }
      }
    }

    if (nl == SP_STR_NO_MATCH) break;
    rest = sp_http_str_tail(rest, nl + 2);
  }
  return SP_TLS_OK;
}

SP_PRIVATE sp_tls_error_t sp_http_read_body(sp_http_reader_t* reader, sp_http_head_t head, sp_io_writer_t* body, u64* written) {
  if (head.status < 200 || head.status == 204 || head.status == 304) {
    return SP_TLS_OK;
  }
  if (head.chunked) {
    for (;;) {
      c8 line[SP_HTTP_LINE_MAX];
      u32 line_len = 0;
      if (!sp_http_read_line(reader, line, sizeof(line), &line_len)) return SP_TLS_ERR_PROTOCOL;
      sp_str_t size_str = sp_str(line, line_len);
      s32 semi = sp_str_find_c8(size_str, ';');
      if (semi != SP_STR_NO_MATCH) size_str = sp_str_sub(size_str, 0, semi);
      size_str = sp_str_trim(size_str);

      u64 size = 0;
      if (!sp_parse_hex_ex(size_str, &size)) return SP_TLS_ERR_PROTOCOL;
      if (size == 0) break;

      sp_tls_error_t err = sp_http_copy_n(reader, body, size, written);
      if (err != SP_TLS_OK) return err;

      c8 crlf[SP_HTTP_LINE_MAX];
      u32 crlf_len = 0;
      if (!sp_http_read_line(reader, crlf, sizeof(crlf), &crlf_len)) return SP_TLS_ERR_PROTOCOL;
    }
    return SP_TLS_OK;
  }
  if (head.has_length) {
    return sp_http_copy_n(reader, body, head.length, written);
  }
  while (sp_http_reader_fill(reader)) {
    u32 avail = reader->len - reader->pos;
    sp_io_write(body, reader->buf + reader->pos, avail, SP_NULLPTR);
    reader->pos = reader->len;
    if (written) *written += avail;
  }
  return SP_TLS_OK;
}

SP_PRIVATE sp_str_t sp_http_build_request(sp_mem_t mem, sp_http_url_t url) {
  bool default_port =
    (url.tls && sp_str_equal_cstr(url.port, "443")) ||
    (!url.tls && sp_str_equal_cstr(url.port, "80"));
  sp_str_t host_header = default_port
    ? url.host
    : sp_fmt(mem, "{}:{}", sp_fmt_str(url.host), sp_fmt_str(url.port)).value;
  return sp_fmt(mem,
    "GET {} HTTP/1.1\r\n"
    "Host: {}\r\n"
    "User-Agent: sp-tls/1.0\r\n"
    "Accept: */*\r\n"
    "Connection: close\r\n"
    "\r\n",
    sp_fmt_str(url.path), sp_fmt_str(host_header)).value;
}

SP_PRIVATE sp_str_t sp_http_resolve_url(sp_mem_t mem, sp_http_url_t base, sp_str_t location) {
  if (sp_str_find(location, sp_str_lit("://")) != SP_STR_NO_MATCH) {
    return sp_str_copy(mem, location);
  }
  sp_str_t scheme = base.tls ? sp_str_lit("https") : sp_str_lit("http");
  if (!sp_str_empty(location) && location.data[0] == '/') {
    return sp_fmt(mem, "{}://{}:{}{}",
      sp_fmt_str(scheme), sp_fmt_str(base.host), sp_fmt_str(base.port), sp_fmt_str(location)).value;
  }
  s32 slash = sp_str_find_c8_reverse(base.path, '/');
  sp_str_t dir = slash == SP_STR_NO_MATCH ? sp_str_lit("/") : sp_str_sub(base.path, 0, slash + 1);
  return sp_fmt(mem, "{}://{}:{}{}{}",
    sp_fmt_str(scheme), sp_fmt_str(base.host), sp_fmt_str(base.port), sp_fmt_str(dir), sp_fmt_str(location)).value;
}

sp_tls_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response) {
  sp_http_response_t resp = sp_zero_s(sp_http_response_t);
  u32 max = request.max_redirects ? request.max_redirects : SP_HTTP_DEFAULT_REDIRECTS;
  sp_str_t current = request.url;
  sp_tls_error_t result = SP_TLS_ERR_PROTOCOL;
  u32 redirects = 0;

  for (;;) {
    sp_http_url_t url = sp_zero_s(sp_http_url_t);
    if (!sp_http_url_parse(current, &url)) {
      result = SP_TLS_ERR_URL;
      break;
    }
    if (url.tls && !request.trust) {
      result = SP_TLS_ERR_BAD_CONFIG;
      break;
    }

    sp_http_conn_t conn = sp_zero_s(sp_http_conn_t);
    result = sp_http_conn_open(&conn, request.trust, url);
    if (result != SP_TLS_OK) {
      sp_http_conn_close(&conn);
      break;
    }

    result = sp_http_conn_write(&conn, sp_http_build_request(mem, url));
    if (result != SP_TLS_OK) {
      sp_http_conn_close(&conn);
      break;
    }

    sp_http_reader_t* reader = sp_alloc_type(mem, sp_http_reader_t);
    *reader = sp_zero_s(sp_http_reader_t);
    reader->conn = &conn;

    sp_str_t head = sp_zero;
    sp_str_t leftover = sp_zero;
    result = sp_http_read_head(reader, mem, &head, &leftover);
    if (result != SP_TLS_OK) {
      sp_http_conn_close(&conn);
      break;
    }

    sp_http_head_t parsed = sp_zero;
    result = sp_http_parse_head(head, &parsed);
    if (result != SP_TLS_OK) {
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
        result = SP_TLS_ERR_REDIRECTS;
        sp_http_conn_close(&conn);
        break;
      }
      current = sp_http_resolve_url(mem, url, parsed.location);
      sp_http_conn_close(&conn);
      continue;
    }

    sp_mem_copy(reader->buf, leftover.data, leftover.len);
    reader->len = leftover.len;
    reader->pos = 0;

    u64 written = 0;
    result = sp_http_read_body(reader, parsed, request.body, &written);
    resp.body_len = written;
    sp_http_conn_close(&conn);

    if (result != SP_TLS_OK) break;
    if (parsed.status >= 400)                          result = SP_TLS_ERR_STATUS;
    else if (parsed.status >= 200 && parsed.status < 300) result = SP_TLS_OK;
    else                                               result = SP_TLS_ERR_PROTOCOL;
    break;
  }

  if (response) *response = resp;
  return result;
}

#else

sp_tls_error_t sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem) {
  *trust = sp_zero_s(sp_tls_trust_t);
  trust->mem = mem;
  trust->backend = sp_tls_native_backend();
  return SP_TLS_OK;
}

void sp_tls_trust_free(sp_tls_trust_t* trust) {
  *trust = sp_zero_s(sp_tls_trust_t);
}

sp_tls_error_t sp_tls_trust_load(sp_tls_trust_t* trust) {
  trust->backend = sp_tls_native_backend();
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped) {
  (void)chain; (void)path;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf) {
  (void)trust; (void)conf;
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname) {
  (void)trust; (void)ssl;
  if (verify) verify->hostname = hostname;
  return SP_TLS_ERR_UNSUPPORTED;
}

s32 sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags) {
  (void)user_data; (void)crt; (void)depth; (void)flags;
  return 0;
}

sp_tls_error_t sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
  (void)chain; (void)hostname;
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count) {
  (void)chain; (void)mem;
  if (ders)  *ders = SP_NULLPTR;
  if (count) *count = 0;
  return SP_TLS_ERR_UNSUPPORTED;
}

sp_tls_error_t sp_http_fetch(sp_mem_t mem, sp_http_request_t request, sp_http_response_t* response) {
  (void)mem; (void)request;
  if (response) *response = sp_zero_s(sp_http_response_t);
  return SP_TLS_ERR_UNSUPPORTED;
}

#endif

#endif
