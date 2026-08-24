#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"
#include "sp/sp_http.h"

#define SERVE_HEAD_MAX 16384
#define SERVE_BODY_MAX (1 << 20)

typedef struct {
  u16       port;
  const c8* output;
  s32       exit;
} serve_t;

static sp_http_reply_t on_index(sp_http_ctx_t* c) {
  sp_unused(c);
  return sp_http_reply_text(200, sp_str_lit("sp serve: POST /screenshot to save a capture\n"));
}

static sp_http_reply_t on_screenshot(sp_http_ctx_t* c) {
  serve_t* serve = sp_cast(serve_t*, c->user_data);
  sp_str_t path = sp_cstr_as_str(serve->output);
  if (sp_fs_create_file_str(path, c->body) != SP_OK) {
    return sp_http_reply_text(500, sp_str_lit("could not write capture\n"));
  }
  sp_str_t reply = sp_fmt(c->mem, "{{\"path\": \"{}\", \"bytes\": {}}}\n", sp_fmt_str(path), sp_fmt_uint(c->body.len)).value;
  return sp_http_reply_json(200, reply);
}

static const sp_http_route_t routes [] = {
  { SP_HTTP_GET,  "/",           on_index },
  { SP_HTTP_POST, "/screenshot", on_screenshot },
};

sp_cli_result_t serve_run(sp_cli_t* cli) {
  serve_t* serve = sp_cast(serve_t*, cli->user_data);

  sp_io_t io = sp_zero;
  if (sp_io_new(&io) != SP_OK) {
    sp_log("could not create an io backend");
    serve->exit = 1;
    return SP_CLI_OK;
  }

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_http_server_t server = sp_zero;
  sp_http_error_t err = sp_http_server_init(&server, (sp_http_server_desc_t) {
    .io = io,
    .addr = {
      .octets = { 127, 0, 0, 1 },
      .port = serve->port
    },
    .router = {
      .routes = routes,
      .count = sp_carr_len(routes),
      .user_data = serve,
    },
    .conn = {
      .mem = sp_mem_heap_as_allocator(heap),
      .head_max = SERVE_HEAD_MAX,
      .body_max = SERVE_BODY_MAX,
    },
  });
  if (err != SP_HTTP_OK) {
    sp_log("could not listen on 127.0.0.1:{}", sp_fmt_uint(serve->port));
    sp_mem_heap_destroy(heap);
    sp_io_destroy(io);
    serve->exit = 1;
    return SP_CLI_OK;
  }

  sp_log("listening on http://127.0.0.1:{}", sp_fmt_uint(server.port));
  sp_http_server_run(&server);

  sp_http_server_deinit(&server);
  sp_mem_heap_destroy(heap);
  sp_io_destroy(io);
  return SP_CLI_OK;
}

s32 run(s32 num_args, const c8** args) {
  serve_t serve = {
    .output = "screenshot.bin",
  };

  sp_cli_cmd_t root = {
    .name = "serve",
    .summary = "Serve a tiny HTTP remote control API",
    .opts = {
      {
        .brief = 'p',
        .name = "port",
        .kind = SP_CLI_OPT_U16,
        .summary = "Port to listen on; defaults to an OS-assigned port",
        .placeholder = "PORT",
        .ptr = &serve.port,
      },
      {
        .brief = 'o',
        .name = "output",
        .kind = SP_CLI_OPT_CSTR,
        .summary = "File to write POST /screenshot payloads to",
        .placeholder = "FILE",
        .ptr = &serve.output,
      },
    },
    .handler = serve_run,
  };

  s32 code = sp_cli_main((sp_cli_desc_t) {
    .root = &root,
    .args = args,
    .num_args = num_args,
    .user_data = &serve,
  });
  return code ? code : serve.exit;
}
SP_MAIN(run)
