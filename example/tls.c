#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"
#include "sp/sp_tls.h"

typedef struct {
  const c8* url;
  const c8* output;
  s32       exit;
} tls_t;

static const c8* backend_name(sp_tls_backend_t backend) {
  switch (backend) {
    case SP_TLS_BACKEND_ANCHORS:   return "anchors (mbedTLS verifies extracted roots)";
    case SP_TLS_BACKEND_OS_VERIFY: return "os-verify (the OS renders the verdict)";
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
sp_cli_result_t tls_run(sp_cli_t* cli) {
  tls_t* tls = sp_cast(tls_t*, cli->user_data);

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);
  sp_http_response_t res = sp_zero;
  sp_tls_error_t err = sp_zero;
  sp_str_t out_path = sp_zero;
  sp_io_file_writer_t out = sp_zero;

  sp_str_t url = sp_cstr_as_str(tls->url);
  sp_http_url_t parsed = sp_zero;
  if (!sp_http_url_parse(url, &parsed)) {
    sp_log("not a valid http(s) url: {}", sp_fmt_str(url));
    tls->exit = 1;
    return SP_CLI_OK;
  }

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  if (sp_tls_trust_load(&trust) != SP_TLS_OK && trust.backend == SP_TLS_BACKEND_ANCHORS) {
    sp_log("failed to load native trust store");
    tls->exit = 1;
    goto done;
  }
  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("anchors: {} loaded, {} skipped", sp_fmt_uint(trust.loaded), sp_fmt_uint(trust.skipped));

  out_path = output_name(mem, parsed, tls->output);
  if (sp_io_file_writer_from_path(&out, out_path) != SP_OK) {
    sp_log("failed to open {} for writing", sp_fmt_str(out_path));
    tls->exit = 1;
    goto done;
  }

  sp_log("fetching {}", sp_fmt_str(url));
  err = sp_http_fetch(mem, (sp_http_request_t) {
    .url   = url,
    .trust = &trust,
    .sink  = &out.base,
  }, &res);
  sp_io_file_writer_close(&out);

  switch (err) {
    case SP_TLS_OK:
      sp_log("wrote {} bytes to {} (status {})", sp_fmt_uint(res.body_len), sp_fmt_str(out_path), sp_fmt_int(res.status));
      break;
    case SP_TLS_ERR_STATUS:
      sp_log("server returned status {}", sp_fmt_int(res.status));
      tls->exit = 1;
      break;
    case SP_TLS_ERR_URL:        sp_log("could not parse url");                  tls->exit = 1; break;
    case SP_TLS_ERR_CONNECT:    sp_log("could not connect to host");            tls->exit = 1; break;
    case SP_TLS_ERR_UNTRUSTED:  sp_log("server certificate is not trusted");    tls->exit = 1; break;
    case SP_TLS_ERR_HANDSHAKE:  sp_log("tls handshake rejected");               tls->exit = 1; break;
    case SP_TLS_ERR_REDIRECTS:  sp_log("too many redirects");                   tls->exit = 1; break;
    case SP_TLS_ERR_PROTOCOL:   sp_log("malformed or truncated http response"); tls->exit = 1; break;
    case SP_TLS_ERR_TIMEOUT:    sp_log("timed out");                            tls->exit = 1; break;
    case SP_TLS_ERR_PROXY:      sp_log("proxy refused or misbehaved");          tls->exit = 1; break;
    case SP_TLS_ERR_NO_STORE:
    case SP_TLS_ERR_PARSE:
    case SP_TLS_ERR_OS:
    case SP_TLS_ERR_BAD_CONFIG:
    case SP_TLS_ERR_UNSUPPORTED:
      sp_log("fetch failed (error {})", sp_fmt_int(err));
      tls->exit = 1;
      break;
  }

  if (tls->exit) sp_fs_remove_file(out_path);

done:
  sp_tls_trust_free(&trust);
  sp_mem_heap_destroy(heap);
  return SP_CLI_OK;
}
#else
sp_cli_result_t tls_run(sp_cli_t* cli) {
  tls_t* tls = sp_cast(tls_t*, cli->user_data);

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  sp_tls_trust_t trust = sp_zero;
  sp_tls_trust_init(&trust, mem);
  sp_tls_trust_load(&trust);

  sp_log("backend: {}", sp_fmt_cstr(backend_name(trust.backend)));
  sp_log("mbedTLS not compiled in for this target; cannot fetch {}", sp_fmt_cstr(tls->url));

  sp_tls_trust_free(&trust);
  sp_mem_heap_destroy(heap);
  return SP_CLI_OK;
}
#endif

s32 run(s32 num_args, const c8** args) {
  tls_t tls = sp_zero;

  sp_cli_cmd_t root = {
    .name = "tls",
    .summary = "Fetch a url over https and save the response body to a file",
    .opts = {
      {
        .brief = "o",
        .name = "output",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "Write the response body to this file instead of inferring one from the url",
        .placeholder = "FILE",
        .ptr = &tls.output,
      },
    },
    .args = {
      {
        .name = "url",
        .summary = "The url to fetch",
        .ptr = &tls.url,
      },
    },
    .handler = tls_run,
  };

  sp_cli_desc_t cli = {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &tls,
  };

  s32 code = sp_cli_main(cli);
  return code ? code : tls.exit;
}
SP_MAIN(run)
