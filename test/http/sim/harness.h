#ifndef HTTP_SIM_TEST_H
#define HTTP_SIM_TEST_H

#include "../http.h"

#define SIM_TEST_PORT    80
#define SIM_TEST_CAPTURE 1024
#define SIM_TEST_RECV    256

typedef struct {
  sp_io_sim_t      sim;
  sp_http_server_t server;
} fixture_t;

typedef struct {
  u32 max_conns;
  u32 idle_ms;
} server_opts_t;

typedef struct {
  sp_io_t         io;
  sp_sys_socket_t socket;
  sp_err_t        err;
  bool            eof;
  sp_io_op_t*     done_op;
  c8              captured [SIM_TEST_CAPTURE];
  u32             len;
} client_t;

SP_INLINE void client_on_done(sp_io_t io, sp_io_op_t* op) {
  sp_unused(io);
  client_t* client = sp_cast(client_t*, op->user_data);
  client->done_op = op;
}

SP_INLINE void server_run(fixture_t* fx) {
  u64 last = SP_LIMIT_U64_MAX;
  while (last != fx->sim.completions) {
    last = fx->sim.completions;
    sp_http_server_pump(&fx->server, sp_io_timeout_after(0));
  }
}

SP_INLINE void server_start(sp_test_t* t, fixture_t* fx, sp_http_router_t router, server_opts_t opts) {
  sp_io_sim_init(&fx->sim);
  sp_http_error_t err = sp_http_server_init(&fx->server, (sp_http_server_desc_t) {
    .io = sp_io_sim_actor(&fx->sim),
    .addr = { .port = SIM_TEST_PORT },
    .listener = sp_io_sim_listen(&fx->sim, SIM_TEST_PORT),
    .router = router,
    .conn = { .mem = sp_test_arena(t) },
    .max_conns = opts.max_conns,
    .idle_ms = opts.idle_ms,
  });
  sp_assert(err == SP_HTTP_OK);
  server_run(fx);
}

SP_INLINE void server_stop(fixture_t* fx) {
  sp_http_server_deinit(&fx->server);
}

SP_INLINE sp_io_op_t* client_reap(client_t* client) {
  sp_assert(sp_io_dispatch(client->io, sp_io_timeout_after(0), SP_NULLPTR) == SP_OK);
  sp_io_op_t* done = client->done_op;
  client->done_op = SP_NULLPTR;
  return done;
}

SP_INLINE client_t client_connect(fixture_t* fx) {
  client_t client = sp_zero;
  client.io = sp_io_sim_actor(&fx->sim);
  client.socket = SP_SYS_INVALID_SOCKET;

  sp_io_op_t op = {
    .kind = SP_IO_OP_CONNECT,
    .connect = { .addr = { .port = SIM_TEST_PORT } },
    .callback = client_on_done,
    .user_data = &client,
  };
  sp_assert(sp_io_submit(client.io, &op) == SP_OK);
  server_run(fx);
  sp_assert(client_reap(&client) == &op);
  client.err = op.result.err;
  client.socket = op.result.socket;
  client.done_op = SP_NULLPTR;
  return client;
}

SP_INLINE void client_send(fixture_t* fx, client_t* client, const c8* bytes) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_SEND,
    .send = {
      .socket = client->socket,
      .buf = { .data = (u8*)bytes, .len = sp_cstr_len(bytes) },
    },
    .callback = client_on_done,
    .user_data = client,
  };
  sp_assert(sp_io_submit(client->io, &op) == SP_OK);
  server_run(fx);
  sp_assert(client_reap(client) == &op);
  client->err = op.result.err;
}

SP_INLINE sp_str_t client_recv(fixture_t* fx, client_t* client) {
  while (true) {
    u8 buf [SIM_TEST_RECV];
    sp_io_op_t op = {
      .kind = SP_IO_OP_RECV,
      .recv = {
        .socket = client->socket,
        .buf = { .data = buf, .len = sizeof(buf) },
      },
      .callback = client_on_done,
      .user_data = client,
    };
    sp_assert(sp_io_submit(client->io, &op) == SP_OK);
    server_run(fx);

    if (!client_reap(client)) {
      sp_assert(sp_io_cancel(client->io, &op) == SP_OK);
      sp_assert(client_reap(client) == &op);
      break;
    }
    if (op.result.err != SP_OK) {
      client->err = op.result.err;
      break;
    }
    if (op.result.len == 0) {
      client->eof = true;
      break;
    }
    sp_assert(client->len + op.result.len <= SIM_TEST_CAPTURE);
    sp_mem_copy(client->captured + client->len, buf, op.result.len);
    client->len += (u32)op.result.len;
  }
  return sp_str(client->captured, client->len);
}

SP_INLINE void client_close(client_t* client) {
  sp_assert(sp_io_close(client->io, client->socket) == SP_OK);
  client->socket = SP_SYS_INVALID_SOCKET;
}

#endif
