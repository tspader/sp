#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_tls.h"

static const c8* backend_name(sp_tls_backend_t backend) {
  switch (backend) {
    case SP_TLS_BACKEND_ANCHORS:   return "anchors (mbedTLS verifies extracted roots)";
    case SP_TLS_BACKEND_OS_VERIFY: return "os-verify (SecTrust renders the verdict)";
    case SP_TLS_BACKEND_NONE:      return "none (no native store on this platform)";
  }
  return "unknown";
}

static sp_str_t output_name(sp_mem_t mem, sp_http_url_t url, const c8* override) {
  if (override) return sp_cstr_as_str(override);
  sp_str_t name = sp_fs_get_name(url.path);
  if (sp_str_empty(name) || sp_str_equal_cstr(name, "/")) return sp_str_lit("index.html");
  return sp_str_copy(mem, name);
}

#if defined(SP_TLS_WITH_MBEDTLS)
s32 tls_main(s32 argc, const c8** argv) {
  if (argc < 2) {
    sp_log("usage: {} <url> [output]", sp_fmt_cstr(argv[0]));
    return 2;
  }

  sp_str_t url = sp_cstr_as_str(argv[1]);
  const c8* override = argc >= 3 ? argv[2] : SP_NULLPTR;

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_http_url_t parsed = sp_zero;
  if (!sp_http_url_parse(url, &parsed)) {
    sp_log("not a valid http(s) url: {}", sp_fmt_str(url));
    return 1;
  }

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  if (sp_tls_trust_load(&trust) != SP_TLS_OK && trust.backend == SP_TLS_BACKEND_ANCHORS) {
    sp_log("failed to load native trust store");
    return 1;
  }
  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("anchors: {} loaded, {} skipped", sp_fmt_uint(trust.loaded), sp_fmt_uint(trust.skipped));

  sp_str_t out_path = output_name(mem, parsed, override);
  sp_io_file_writer_t out = sp_zero;
  if (sp_io_file_writer_from_path(&out, out_path) != SP_OK) {
    sp_log("failed to open {} for writing", sp_fmt_str(out_path));
    return 1;
  }

  sp_log("fetching {}", sp_fmt_str(url));
  sp_http_response_t res = sp_zero;
  sp_tls_error_t err = sp_http_fetch(mem, (sp_http_request_t) {
    .url   = url,
    .trust = &trust,
    .body  = &out.base,
  }, &res);
  sp_io_file_writer_close(&out);

  s32 status = 0;
  switch (err) {
    case SP_TLS_OK:
      sp_log("wrote {} bytes to {} (status {})", sp_fmt_uint(res.body_len), sp_fmt_str(out_path), sp_fmt_int(res.status));
      break;
    case SP_TLS_ERR_STATUS:
      sp_log("server returned status {}", sp_fmt_int(res.status));
      status = 1;
      break;
    case SP_TLS_ERR_URL:        sp_log("could not parse url");                  status = 1; break;
    case SP_TLS_ERR_CONNECT:    sp_log("could not connect to host");            status = 1; break;
    case SP_TLS_ERR_HANDSHAKE:  sp_log("tls handshake rejected");               status = 1; break;
    case SP_TLS_ERR_REDIRECTS:  sp_log("too many redirects");                   status = 1; break;
    case SP_TLS_ERR_PROTOCOL:   sp_log("malformed or truncated http response"); status = 1; break;
    case SP_TLS_ERR_TIMEOUT:    sp_log("timed out");                            status = 1; break;
    case SP_TLS_ERR_PROXY:      sp_log("proxy refused or misbehaved");          status = 1; break;
    case SP_TLS_ERR_NO_STORE:
    case SP_TLS_ERR_PARSE:
    case SP_TLS_ERR_OS:
    case SP_TLS_ERR_UNTRUSTED:
    case SP_TLS_ERR_BAD_CONFIG:
    case SP_TLS_ERR_UNSUPPORTED:
      sp_log("fetch failed (error {})", sp_fmt_int(err));
      status = 1;
      break;
  }

  if (status) sp_fs_remove_file(out_path);
  sp_tls_trust_free(&trust);
  return status;
}
#else
s32 tls_main(s32 argc, const c8** argv) {
  const c8* url = argc >= 2 ? argv[1] : "https://example.com";

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  sp_tls_trust_load(&trust);

  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("mbedTLS not compiled in for this target; cannot fetch {}", sp_fmt_cstr(url));

  sp_tls_trust_free(&trust);
  return 0;
}
#endif
SP_MAIN(tls_main)
