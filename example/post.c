#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"
#include "sp/sp_http.h"

typedef struct {
  const c8* method;
  const c8* data;
  const c8* type;
  const c8* output;
  const c8* url;
  bool      verbose;
  s32       exit;
} post_t;

#if defined(SP_TLS_WITH_MBEDTLS)
static const c8* method_names[] = { "GET", "POST", "PUT", "PATCH", "DELETE" };

static bool method_parse(sp_mem_t mem, sp_str_t str, sp_http_method_t* method) {
  sp_str_t upper = sp_str_to_upper(mem, str);
  sp_carr_for(method_names, it) {
    if (sp_str_equal_cstr(upper, method_names[it])) {
      *method = (sp_http_method_t)it;
      return true;
    }
  }
  return false;
}

sp_cli_result_t post_run(sp_cli_t* cli) {
  post_t* post = sp_cast(post_t*, cli->user_data);

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);
  sp_cli_result_t result = SP_CLI_OK;
  sp_tls_trust_t trust = sp_zero;
  sp_tls_mbedtls_t client = sp_zero;
  sp_io_file_writer_t file = sp_zero;
  sp_http_response_t response = sp_zero;
  sp_http_error_t err = sp_zero;

  sp_http_request_t request = {
    .url = sp_cstr_as_str(post->url),
  };

  if (post->data) {
    sp_str_t data = sp_cstr_as_str(post->data);
    if (!sp_str_empty(data) && data.data[0] == '@') {
      sp_str_t path = sp_str_sub(data, 1, (s32)data.len - 1);
      if (sp_io_read_file(mem, path, &request.payload) != SP_OK) {
        result = sp_cli_set_error(cli, sp_fmt(mem, "could not read {}", sp_fmt_str(path)).value);
        goto done;
      }
    }
    else {
      request.payload = data;
    }
    request.method = SP_HTTP_POST;
  }

  if (post->method && !method_parse(mem, sp_cstr_as_str(post->method), &request.method)) {
    result = sp_cli_set_error_c(cli, "method must be one of GET, POST, PUT, PATCH, DELETE");
    goto done;
  }
  if (post->type) {
    request.content_type = sp_cstr_as_str(post->type);
  }

  for (u32 it = 0; cli->rest[it]; it++) {
    if (it >= SP_HTTP_MAX_HEADERS) {
      result = sp_cli_set_error_c(cli, "too many headers");
      goto done;
    }
    sp_str_t header = sp_cstr_as_str(cli->rest[it]);
    s32 colon = sp_str_find_c8(header, ':');
    if (colon == SP_STR_NO_MATCH) {
      result = sp_cli_set_error(cli, sp_fmt(mem, "header must be 'Name: Value', got {}", sp_fmt_str(header)).value);
      goto done;
    }
    request.headers[it].name = sp_str_sub(header, 0, colon);
    request.headers[it].value = sp_str_trim(sp_str_sub(header, colon + 1, (s32)header.len - colon - 1));
  }

  sp_tls_trust_init(&trust, mem);
  if (sp_tls_trust_load(&trust) != SP_HTTP_OK && trust.backend == SP_TLS_BACKEND_ANCHORS) {
    sp_log("failed to load native trust store");
    post->exit = 1;
    goto done;
  }
  sp_tls_mbedtls_init(&client, &trust);
  request.tls = &client.base;

  if (post->output) {
    if (sp_io_file_writer_from_path(&file, sp_cstr_as_str(post->output)) != SP_OK) {
      sp_log("failed to open {} for writing", sp_fmt_cstr(post->output));
      post->exit = 1;
      goto done;
    }
    request.sink = &file.base;
  }
  else {
    request.sink = sp_io_get_std_out();
  }

  if (post->verbose) {
    sp_log("{} {} ({} bytes)", sp_fmt_cstr(method_names[request.method]), sp_fmt_str(request.url), sp_fmt_uint(request.payload.len));
  }

  err = sp_http_fetch(mem, request, &response);
  if (post->output) sp_io_file_writer_close(&file);

  switch (err) {
    case SP_HTTP_OK:
      break;
    case SP_HTTP_ERR_STATUS:
      sp_log("server returned status {}", sp_fmt_int(response.status));
      post->exit = 1;
      break;
    case SP_HTTP_ERR_URL:        sp_log("could not parse url");                  post->exit = 1; break;
    case SP_HTTP_ERR_CONNECT:    sp_log("could not connect to host");            post->exit = 1; break;
    case SP_HTTP_ERR_UNTRUSTED:  sp_log("server certificate is not trusted");    post->exit = 1; break;
    case SP_HTTP_ERR_HANDSHAKE:  sp_log("tls handshake rejected");               post->exit = 1; break;
    case SP_HTTP_ERR_REDIRECTS:  sp_log("too many redirects");                   post->exit = 1; break;
    case SP_HTTP_ERR_PROTOCOL:   sp_log("malformed or truncated http response"); post->exit = 1; break;
    case SP_HTTP_ERR_TIMEOUT:    sp_log("timed out");                            post->exit = 1; break;
    case SP_HTTP_ERR_PROXY:      sp_log("proxy refused or misbehaved");          post->exit = 1; break;
    case SP_HTTP_ERR_BAD_CONFIG: sp_log("bad request configuration");            post->exit = 1; break;
    case SP_HTTP_ERR_NO_STORE:
    case SP_HTTP_ERR_PARSE:
    case SP_HTTP_ERR_OS:
    case SP_HTTP_ERR_UNSUPPORTED:
      sp_log("request failed (error {})", sp_fmt_int(err));
      post->exit = 1;
      break;
  }

  if (post->verbose && err == SP_HTTP_OK) {
    sp_log("status {} ({} bytes, {})",
      sp_fmt_int(response.status),
      sp_fmt_uint(response.body_len),
      sp_fmt_str(sp_str_empty(response.content_type) ? sp_str_lit("no content type") : response.content_type));
  }

done:
  sp_tls_trust_free(&trust);
  sp_mem_heap_destroy(heap);
  return result;
}
#else
sp_cli_result_t post_run(sp_cli_t* cli) {
  post_t* post = sp_cast(post_t*, cli->user_data);
  sp_log("mbedTLS not compiled in for this target; cannot request {}", sp_fmt_cstr(post->url));
  return SP_CLI_OK;
}
#endif

s32 run(s32 num_args, const c8** args) {
  post_t post = sp_zero;

  sp_cli_cmd_t root = {
    .name = "post",
    .summary = "Make an HTTP request",
    .opts = {
      {
        .brief = 'X', .name = "method", .kind = SP_CLI_OPT_CSTR,
        .summary = "Request method; defaults to POST when --data is given, GET otherwise", .placeholder = "METHOD", .ptr = &post.method,
      },
      {
        .brief = 'd',
        .name = "data",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "Request body; @path reads the body from a file",
        .placeholder = "DATA",
        .ptr = &post.data,
      },
      {
        .brief = 't',
        .name = "type",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "Content-Type of the request body",
        .placeholder = "TYPE",
        .ptr = &post.type,
      },
      {
        .brief = 'o',
        .name = "output",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "Write the response body to a file instead of stdout",
        .placeholder = "FILE",
        .ptr = &post.output,
      },
      {
        .brief = 'v',
        .name = "verbose",
        .summary = "Log the request and response summary",
        .ptr = &post.verbose,
      },
    },
    .args = {
      {
        .name = "url",
        .summary = "The url to request",
        .ptr = &post.url,
      },
      {
        .name = "headers",
        .arity = SP_CLI_ARG_REST,
        .summary = "Extra request headers, each as 'Name: Value'",
      },
    },
    .handler = post_run,
  };

  s32 code = sp_cli_main((sp_cli_desc_t) {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &post,
  });
  return code ? code : post.exit;
}
SP_MAIN(run)
