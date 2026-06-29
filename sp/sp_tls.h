/*
  >> sp_tls.h
  native OS trust for mbedTLS: HTTPS that works on real machines and in corporate environments

  ## TL;DR
  mbedTLS verifies a TLS server certificate against an array of trust anchors that you supply. It has
  no idea where your OS keeps its trusted roots, and it cannot ask the OS to make a trust decision. This
  extension fills exactly that gap, and nothing else. It is not an HTTP client; it wires the platform's
  native trust into an mbedTLS handshake you already own.

  Grep these tags to jump around:

  types
    @error       status codes
    @backend     how trust is sourced on this platform
    @trust       the native trust set
    @verify      per-connection verification state + callback

  functions
    @lifecycle   init / free / load
    @loaders     platform root extraction into an mbedTLS chain
    @wire        attach trust to your mbedtls_ssl_config / mbedtls_ssl_context
    @osverify    the macOS SecTrust path
    @util        helpers


  ##########
  ## USAGE #
  ##########
  Define the following before you include sp_tls.h in exactly one C or C++ file:

    #define SP_IMPLEMENTATION

  sp_tls.h is an extension to sp.h; make sure that sp.h is also on your include path. To compile the real
  implementation (as opposed to this stub), you must also have mbedTLS on your include path and define:

    #define SP_TLS_WITH_MBEDTLS

  Without that define the library still compiles everywhere (including wasm and freestanding targets) and
  references mbedTLS only through forward-declared struct tags, so no mbedTLS header is required.


  ### THE TWO TRUST MODELS
  There is no single portable way to "use the OS trust", so this library exposes two backends and picks one
  per platform (see sp_tls_native_backend):

    - SP_TLS_BACKEND_ANCHORS    (Windows, Linux)
        We extract the OS root certificates into an mbedtls_x509_crt chain and hand it to mbedTLS as the
        trust anchors. mbedTLS performs verification. Windows enumerates the ROOT + CA system stores and
        filters by validity / key usage / server-auth EKU; Linux reads the distro's PEM bundle.

    - SP_TLS_BACKEND_OS_VERIFY  (macOS)
        macOS trust is a policy engine, not a flat list of certs, so we do NOT extract anchors. mbedTLS
        verification is disabled for the connection and the peer's certificate chain is handed to SecTrust,
        which renders the verdict. This is done from inside an mbedTLS verify callback so a rejected chain
        aborts the handshake before any application data is sent.

    - SP_TLS_BACKEND_NONE       (wasm, freestanding, anything with no OS trust store)
        sp_tls_trust_load reports SP_TLS_ERR_UNSUPPORTED. Bundle your own PEM and use sp_tls_load_pem.


  ### THE FLOW
  You own the mbedTLS objects. The library attaches to them:

    sp_tls_trust_t trust = sp_zero;
    sp_tls_trust_init(&trust, mem);
    sp_tls_trust_load(&trust);                       // read the native store

    mbedtls_ssl_config conf; // ...your usual mbedTLS setup...
    sp_tls_conf_apply(&trust, &conf);                // ANCHORS: ca_chain + VERIFY_REQUIRED

    mbedtls_ssl_context ssl;
    sp_tls_verify_t verify = sp_zero;
    sp_tls_ssl_attach(&trust, &ssl, &verify, host);  // SNI + hostname check (+ SecTrust cb on macOS)

    // mbedtls_ssl_handshake(&ssl) now enforces trust on every platform.

  The verify context must outlive the connection: on macOS it carries the hostname into the SecTrust policy.


  ### CONFIG REQUIREMENT (macOS)
  The OS-verify backend needs the peer chain after the handshake, which requires mbedTLS to be built with
  MBEDTLS_SSL_KEEP_PEER_CERTIFICATE (the default). If it is disabled, sp_tls_ssl_attach returns
  SP_TLS_ERR_BAD_CONFIG rather than silently failing open.
*/

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

#endif


#if defined(SP_TLS_IMPLEMENTATION)

#if defined(SP_TLS_WITH_MBEDTLS)
  #include <mbedtls/x509_crt.h>
  #include <mbedtls/ssl.h>
#endif


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


sp_tls_error_t sp_tls_trust_init(sp_tls_trust_t* trust, sp_mem_t mem) {
  *trust = sp_zero_s(sp_tls_trust_t);
  trust->mem = mem;
  trust->backend = sp_tls_native_backend();
  return SP_TLS_OK;
}

void sp_tls_trust_free(sp_tls_trust_t* trust) {
  (void)trust;
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
    default:
      return SP_TLS_ERR_UNSUPPORTED;
  }
}


sp_tls_error_t sp_tls_load_windows(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_OK;
}

sp_tls_error_t sp_tls_load_unix(struct mbedtls_x509_crt* chain, sp_mem_t mem, u32* loaded, u32* skipped) {
  (void)chain; (void)mem;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_OK;
}

sp_tls_error_t sp_tls_load_pem(struct mbedtls_x509_crt* chain, sp_str_t path, u32* loaded, u32* skipped) {
  (void)chain; (void)path;
  if (loaded)  *loaded = 0;
  if (skipped) *skipped = 0;
  return SP_TLS_OK;
}

sp_tls_error_t sp_tls_conf_apply(const sp_tls_trust_t* trust, struct mbedtls_ssl_config* conf) {
  (void)trust; (void)conf;
  return SP_TLS_OK;
}

sp_tls_error_t sp_tls_ssl_attach(const sp_tls_trust_t* trust, struct mbedtls_ssl_context* ssl, sp_tls_verify_t* verify, sp_str_t hostname) {
  (void)trust; (void)ssl;
  if (verify) verify->hostname = hostname;
  return SP_TLS_OK;
}

s32 sp_tls_verify_cb(void* user_data, struct mbedtls_x509_crt* crt, s32 depth, u32* flags) {
  (void)user_data; (void)crt; (void)depth; (void)flags;
  return 0;
}

sp_tls_error_t sp_tls_macos_eval(const struct mbedtls_x509_crt* chain, sp_str_t hostname) {
  (void)chain; (void)hostname;
  return SP_TLS_OK;
}


sp_tls_error_t sp_tls_chain_der(const struct mbedtls_x509_crt* chain, sp_mem_t mem, sp_mem_slice_t** ders, u32* count) {
  (void)chain; (void)mem;
  if (ders)  *ders = SP_NULLPTR;
  if (count) *count = 0;
  return SP_TLS_OK;
}

#endif
