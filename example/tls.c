#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_tls.h"

static const c8* backend_name(sp_tls_backend_t backend) {
  switch (backend) {
    case SP_TLS_BACKEND_ANCHORS:   return "anchors (mbedTLS verifies extracted roots)";
    case SP_TLS_BACKEND_OS_VERIFY: return "os-verify (SecTrust renders the verdict)";
    case SP_TLS_BACKEND_NONE:      return "none (no native store on this platform)";
    default:                       return "unknown";
  }
}

s32 tls_main(s32 argc, const c8** argv) {
  const c8* host = argc >= 2 ? argv[1] : "example.com";

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);

  sp_tls_error_t err = sp_tls_trust_load(&trust);
  if (err == SP_TLS_ERR_UNSUPPORTED) {
    sp_log("no native trust on this platform; bundle a PEM and use sp_tls_load_pem");
    return 0;
  }

  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("anchors: {} loaded, {} skipped", sp_fmt_uint(trust.loaded), sp_fmt_uint(trust.skipped));

  struct mbedtls_ssl_config*  conf = SP_NULLPTR;
  struct mbedtls_ssl_context* ssl  = SP_NULLPTR;

  sp_tls_conf_apply(&trust, conf);

  sp_tls_verify_t verify = sp_zero;
  sp_tls_ssl_attach(&trust, ssl, &verify, sp_str_view(host));

  sp_log("would handshake with {} and enforce native trust", sp_fmt_cstr(host));

  sp_tls_trust_free(&trust);
  return 0;
}
SP_MAIN(tls_main)
