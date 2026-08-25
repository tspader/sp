#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"
#include "sp/sp_http.h"

#define SERVE_BUFFER_SIZE 16384
#define SERVE_IO_TIMEOUT_MS 10000

typedef struct {
  u16       port;
  const c8* output;
  s32       exit;
} serve_t;

static void respond(sp_io_writer_t* writer, s32 status, sp_str_t content_type, sp_str_t body) {
  sp_http_header_t headers [2];
  u32 count = 0;
  if (!sp_str_empty(content_type)) {
    headers[count++] = (sp_http_header_t) { .name = sp_str_lit("Content-Type"), .value = content_type };
  }
  headers[count++] = (sp_http_header_t) { .name = sp_str_lit("Connection"), .value = sp_str_lit("close") };
  sp_http_response_write(writer, status, headers, count, body);
}

static void serve_client(serve_t* serve, sp_sys_socket_t client) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();

  sp_io_socket_reader_t reader = sp_zero;
  sp_io_socket_writer_t writer = sp_zero;
  sp_io_socket_reader_init(&reader, client, SERVE_IO_TIMEOUT_MS);
  sp_io_socket_writer_init(&writer, client, SERVE_IO_TIMEOUT_MS);
  u8* buffer = sp_alloc_n(scratch.mem, u8, SERVE_BUFFER_SIZE);
  sp_io_reader_set_buffer(&reader.base, buffer, SERVE_BUFFER_SIZE);

  sp_str_t head_str = sp_zero;
  sp_http_request_head_t request = sp_zero;
  sp_http_body_t body = sp_zero;
  sp_http_method_t method = SP_HTTP_GET;
  bool known_method = false;
  sp_str_t target = sp_zero;
  sp_io_dyn_mem_writer_t payload = sp_zero;
  sp_io_dyn_mem_writer_init(scratch.mem, &payload);

  if (sp_http_head_read(&reader.base, &head_str) != SP_HTTP_OK ||
      sp_http_request_head_parse(head_str, &request) != SP_HTTP_OK ||
      sp_http_body_parse(request.headers, &body) != SP_HTTP_OK) {
    respond(&writer.base, 400, sp_str_lit("text/plain"), sp_str_lit("bad request\n"));
    goto done;
  }

  known_method = sp_http_method_parse(request.method, &method);
  target = sp_str_copy(scratch.mem, request.target);

  if (sp_http_body_read(&reader.base, body, &payload.base, SP_NULLPTR) != SP_HTTP_OK) {
    respond(&writer.base, 400, sp_str_lit("text/plain"), sp_str_lit("bad body\n"));
    goto done;
  }

  if (!known_method) {
    respond(&writer.base, 501, sp_zero_s(sp_str_t), sp_zero_s(sp_str_t));
    goto done;
  }

  sp_log("{} {}", sp_fmt_str(sp_http_method_name(method)), sp_fmt_str(target));

  if (method == SP_HTTP_GET && sp_str_equal_cstr(target, "/")) {
    respond(&writer.base, 200, sp_str_lit("text/plain"), sp_str_lit("sp serve: POST /screenshot to save a capture\n"));
  }
  else if (method == SP_HTTP_POST && sp_str_equal_cstr(target, "/screenshot")) {
    sp_str_t data = sp_io_dyn_mem_writer_as_str(&payload);
    sp_str_t path = sp_cstr_as_str(serve->output);
    if (sp_fs_create_file_str(path, data) != SP_OK) {
      respond(&writer.base, 500, sp_str_lit("text/plain"), sp_str_lit("could not write capture\n"));
      goto done;
    }
    sp_str_t reply = sp_fmt(scratch.mem, "{{\"path\": \"{}\", \"bytes\": {}}}\n", sp_fmt_str(path), sp_fmt_uint(data.len)).value;
    respond(&writer.base, 200, sp_str_lit("application/json"), reply);
  }
  else {
    respond(&writer.base, 404, sp_str_lit("text/plain"), sp_str_lit("not found\n"));
  }

done:
  sp_mem_end_scratch(scratch);
  sp_sys_socket_close(client);
}

sp_cli_result_t serve_run(sp_cli_t* cli) {
  serve_t* serve = sp_cast(serve_t*, cli->user_data);

  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  if (sp_sys_socket_open(&listener, (sp_sys_handle_desc_t) { SP_SYS_BLOCKING }) != SP_OK ||
      sp_sys_socket_reuse_addr(listener) != SP_OK ||
      sp_sys_socket_bind(listener, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 }, .port = serve->port }) != SP_OK ||
      sp_sys_socket_listen(listener, 4) != SP_OK) {
    sp_log("could not listen on 127.0.0.1:{}", sp_fmt_uint(serve->port));
    serve->exit = 1;
    return SP_CLI_OK;
  }

  u16 port = 0;
  sp_sys_socket_local_port(listener, &port);
  sp_log("listening on http://127.0.0.1:{}", sp_fmt_uint(port));

  for (;;) {
    sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
    if (sp_sys_socket_accept(listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }, &client) != SP_OK) break;
    serve_client(serve, client);
  }

  sp_sys_socket_close(listener);
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
