#include "sp/sp_test.h"
#include "sock.h"

#if !defined(SP_WASM)

#define SOCKET_MAX_STEPS 8
#define SOCKET_BUF_SIZE 64

typedef enum {
  STEP_NONE,
  STEP_SEND,
  STEP_RECV,
  STEP_WAIT,
  STEP_CLOSE_CLIENT,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { const c8* data; } send;
    struct { u64 request; u64 expect; const c8* content; sp_err_t err; } recv;
    struct { u32 timeout_ms; sp_err_t err; } wait;
  };
} step_t;

typedef struct {
  const c8* name;
  step_t steps [SOCKET_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "send_recv_roundtrip",
    .steps = {
      { .kind = STEP_SEND, .send = { .data = "AB" } },
      { .kind = STEP_RECV, .recv = { .request = 8, .expect = 2, .content = "AB" } },
    },
  },
  {
    .name = "recv_returns_zero_after_peer_close",
    .steps = {
      { .kind = STEP_SEND, .send = { .data = "A" } },
      { .kind = STEP_CLOSE_CLIENT },
      { .kind = STEP_RECV, .recv = { .request = 8, .expect = 1, .content = "A" } },
      { .kind = STEP_RECV, .recv = { .request = 8 } },
    },
  },
  {
    .name = "wait_readable_ready_after_send",
    .steps = {
      { .kind = STEP_SEND, .send = { .data = "A" } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
    },
  },
  {
    .name = "wait_readable_times_out_when_idle",
    .steps = {
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .err = SP_ERR_SYS_TIMED_OUT } },
    },
  },
  {
    .name = "recv_would_block_when_idle",
    .steps = {
      { .kind = STEP_RECV, .recv = { .request = 8, .err = SP_ERR_SYS_WOULD_BLOCK } },
    },
  },
};

static bool socket_dial(sp_sys_socket_t socket, sp_sys_ipv4_t addr) {
  sp_err_t err = sp_sys_socket_connect(socket, addr);
  if (err == SP_OK) return true;
  if (err != SP_ERR_SYS_WOULD_BLOCK) return false;
  if (sp_sys_socket_wait(socket, false, 1000) != SP_OK) return false;
  return sp_sys_socket_error(socket) == SP_OK;
}

static bool pair(sp_sys_socket_t* listener, sp_sys_socket_t* client, sp_sys_socket_t* server) {
  sp_sys_ipv4_t addr = sp_zero;
  if (!socket_open_listener(listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }, &addr)) return false;
  if (sp_sys_socket_open(client, SP_SYS_SOCKET_STREAM, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }) != SP_OK) return false;
  if (!socket_dial(*client, addr)) return false;

  while (true) {
    sp_err_t err = sp_sys_socket_accept(*listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }, server);
    if (err == SP_OK) return true;
    if (err != SP_ERR_SYS_WOULD_BLOCK) return false;
    if (sp_sys_socket_wait(*listener, true, 1000) != SP_OK) return false;
  }
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t server = SP_SYS_INVALID_SOCKET;
  sp_must(t, pair(&listener, &client, &server));

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_SEND: {
        u64 len = sp_cstr_len(step->send.data);
        u64 sent = 0;
        while (sent < len) {
          u64 n = 0;
          sp_err_t err = sp_sys_socket_send(client, step->send.data + sent, len - sent, &n);
          if (err == SP_ERR_SYS_WOULD_BLOCK) {
            sp_must_ok(t, sp_sys_socket_wait(client, false, 1000));
            continue;
          }
          sp_must_ok(t, err);
          sp_must_gt(t, n, (u64)0);
          sent += n;
        }
        break;
      }
      case STEP_RECV: {
        c8 buf [SOCKET_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_err_t err = SP_OK;
        while (true) {
          err = sp_sys_socket_recv(server, buf, step->recv.request, &n);
          if (err != SP_ERR_SYS_WOULD_BLOCK) break;
          if (step->recv.err == SP_ERR_SYS_WOULD_BLOCK) break;
          sp_must_ok(t, sp_sys_socket_wait(server, true, 1000));
        }
        sp_expect_err_eq(t, err, step->recv.err);
        if (!err && !step->recv.err) {
          sp_expect_eq(t, n, step->recv.expect);
          if (step->recv.content) {
            sp_expect_str_eq_c(t, sp_str(buf, (u32)n), step->recv.content);
          }
        }
        break;
      }
      case STEP_WAIT: {
        sp_expect_err_eq(t, sp_sys_socket_wait(server, true, step->wait.timeout_ms), step->wait.err);
        break;
      }
      case STEP_CLOSE_CLIENT: {
        sp_sys_socket_close(client);
        client = SP_SYS_INVALID_SOCKET;
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  if (client != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(client);
  sp_sys_socket_close(server);
  sp_sys_socket_close(listener);
  return SP_OK;
}

sp_test_each_fn(sys, socket, test_t, tests, run);

#endif
