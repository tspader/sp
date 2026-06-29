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

#if defined(SP_TLS_WITH_MBEDTLS)

#include <mbedtls/x509_crt.h>
#include <mbedtls/ssl.h>
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

#endif

#endif
