#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_tls.h"

#if defined(SP_TLS_WITH_MBEDTLS)
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#endif

static const c8* backend_name(sp_tls_backend_t backend) {
  switch (backend) {
    case SP_TLS_BACKEND_ANCHORS:   return "anchors (mbedTLS verifies extracted roots)";
    case SP_TLS_BACKEND_OS_VERIFY: return "os-verify (SecTrust renders the verdict)";
    case SP_TLS_BACKEND_NONE:      return "none (no native store on this platform)";
  }
  return "unknown";
}

#if defined(SP_TLS_WITH_MBEDTLS)
s32 tls_main(s32 argc, const c8** argv) {
  const c8* host = argc >= 2 ? argv[1] : "example.com";

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  if (sp_tls_trust_load(&trust) != SP_TLS_OK && trust.backend == SP_TLS_BACKEND_ANCHORS) {
    sp_log("failed to load native trust store");
    return 1;
  }

  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("anchors: {} loaded, {} skipped", sp_fmt_uint(trust.loaded), sp_fmt_uint(trust.skipped));

  mbedtls_net_context net;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context drbg;
  mbedtls_net_init(&net);
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&drbg);

  s32 status = 1;
  do {
    if (mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy, SP_NULLPTR, 0) != 0) {
      sp_log("failed to seed rng");
      break;
    }

    if (mbedtls_net_connect(&net, host, "443", MBEDTLS_NET_PROTO_TCP) != 0) {
      sp_log("failed to connect to {}", sp_fmt_cstr(host));
      break;
    }

    mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &drbg);
    sp_tls_conf_apply(&trust, &conf);
    mbedtls_ssl_setup(&ssl, &conf);

    sp_tls_verify_t verify = sp_zero;
    sp_tls_ssl_attach(&trust, &ssl, &verify, sp_cstr_as_str(host));
    mbedtls_ssl_set_bio(&ssl, &net, mbedtls_net_send, mbedtls_net_recv, SP_NULLPTR);

    s32 rc;
    bool handshook = true;
    while ((rc = mbedtls_ssl_handshake(&ssl)) != 0) {
      if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
        u32 verdict = mbedtls_ssl_get_verify_result(&ssl);
        sp_log("handshake rejected (rc -0x{:X} verify 0x{:X})", sp_fmt_uint((u32)-rc), sp_fmt_uint(verdict));
        handshook = false;
        break;
      }
    }
    if (!handshook) break;

    sp_log("handshake ok: {} via {}", sp_fmt_cstr(host), sp_fmt_cstr(mbedtls_ssl_get_ciphersuite(&ssl)));

    sp_str_t request = sp_fmt(mem, "GET / HTTP/1.0\r\nHost: {}\r\nConnection: close\r\n\r\n", sp_fmt_cstr(host)).value;
    mbedtls_ssl_write(&ssl, (const unsigned char*)request.data, request.len);

    unsigned char buf[1024];
    s32 n = mbedtls_ssl_read(&ssl, buf, sizeof(buf) - 1);
    if (n > 0) {
      sp_str_t response = (sp_str_t) { .data = (const c8*)buf, .len = (u32)n };
      s32 newline = sp_str_find(response, sp_str_lit("\r\n"));
      s32 end = newline == SP_STR_NO_MATCH ? (s32)response.len : newline;
      sp_str_t line = sp_str_sub(response, 0, end);
      sp_log("response: {}", sp_fmt_str(line));
      status = 0;
    }

    mbedtls_ssl_close_notify(&ssl);
  } while (0);

  mbedtls_ssl_free(&ssl);
  mbedtls_ssl_config_free(&conf);
  mbedtls_ctr_drbg_free(&drbg);
  mbedtls_entropy_free(&entropy);
  mbedtls_net_free(&net);
  sp_tls_trust_free(&trust);
  return status;
}
#else
s32 tls_main(s32 argc, const c8** argv) {
  const c8* host = argc >= 2 ? argv[1] : "example.com";

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  sp_tls_trust_load(&trust);

  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("mbedTLS not compiled in for this target; cannot fetch {}", sp_fmt_cstr(host));

  sp_tls_trust_free(&trust);
  return 0;
}
#endif
SP_MAIN(tls_main)
