#include "harness.h"

#define TRANSFER_MAX_STEPS 8
#define TRANSFER_PIPE_OVERFLOW ((1u << 20) + 4096)

typedef enum {
  STEP_NONE,
  STEP_SUBMIT,
  STEP_WAIT,
  STEP_PEER_SEND,
  STEP_PEER_RECV,
  STEP_PEER_CLOSE,
  STEP_PEER_CONNECT,
  STEP_PIPE_WRITE,
  STEP_PIPE_READ,
  STEP_PIPE_CLOSE_W,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    ops_submit_t submit;
    ops_wait_t wait;
    struct { const c8* data; } peer_send;
    struct { const c8* expect; } peer_recv;
    struct { const c8* data; } pipe_write;
    struct { const c8* expect; } pipe_read;
  };
} step_t;

typedef struct {
  const c8* name;
  ops_fixture_t fixture;
  step_t steps [TRANSFER_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "recv_completes_after_peer_send",
    .fixture = FIXTURE_SOCKETS,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_RECV, .handle = H_SERVER, .size = 8 } },
      { .kind = STEP_PEER_SEND, .peer_send = { .data = "AB" } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .len = 2, .content = "AB" } } } } },
    },
  },
  {
    .name = "recv_reports_eof_after_peer_close",
    .fixture = FIXTURE_SOCKETS,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_RECV, .handle = H_SERVER, .size = 8 } },
      { .kind = STEP_PEER_CLOSE },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1 } } },
    },
  },
  {
    .name = "send_lands_on_peer",
    .fixture = FIXTURE_SOCKETS,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_SEND, .handle = H_SERVER, .data = "AB" } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .len = 2 } } } } },
      { .kind = STEP_PEER_RECV, .peer_recv = { .expect = "AB" } },
    },
  },
  {
    .name = "accept_returns_connected_socket",
    .fixture = FIXTURE_LISTENER,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_ACCEPT, .handle = H_LISTENER } },
      { .kind = STEP_PEER_CONNECT },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1 } } },
      { .kind = STEP_PEER_SEND, .peer_send = { .data = "A" } },
      { .kind = STEP_SUBMIT, .submit = { .slot = 1, .op = SP_IO_OP_RECV, .handle = H_ACCEPTED, .size = 8 } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .slot = 1, .len = 1, .content = "A" } } } } },
    },
  },
  {
    .name = "read_completes_after_pipe_write",
    .fixture = FIXTURE_PIPE,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_READ, .handle = H_PIPE_R, .size = 8 } },
      { .kind = STEP_PIPE_WRITE, .pipe_write = { .data = "AB" } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .len = 2, .content = "AB" } } } } },
    },
  },
  {
    .name = "read_reports_eof_after_write_end_close",
    .fixture = FIXTURE_PIPE,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_READ, .handle = H_PIPE_R, .size = 8 } },
      { .kind = STEP_PIPE_CLOSE_W },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1 } } },
    },
  },
  {
    .name = "write_lands_in_pipe",
    .fixture = FIXTURE_PIPE,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_WRITE, .handle = H_PIPE_W, .data = "AB" } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .len = 2 } } } } },
      { .kind = STEP_PIPE_READ, .pipe_read = { .expect = "AB" } },
    },
  },
  {
    .name = "write_larger_than_pipe_completes_partially",
    .fixture = FIXTURE_PIPE,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_WRITE, .handle = H_PIPE_W, .size = TRANSFER_PIPE_OVERFLOW } },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .partial = true } } } } },
    },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  ops_harness_t h;
  sp_try(harness_open(t, &h, OPS_ENTRIES, c->fixture));

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_SUBMIT: {
        sp_expect_ok(t, harness_submit(t, &h, &step->submit));
        break;
      }
      case STEP_WAIT: {
        harness_wait(t, &h, &step->wait);
        break;
      }
      case STEP_PEER_SEND: {
        u64 len = sp_cstr_len(step->peer_send.data);
        u64 n = 0;
        sp_expect_ok(t, sp_sys_socket_send(h.client, step->peer_send.data, len, &n));
        sp_expect_eq(t, n, len);
        break;
      }
      case STEP_PEER_RECV: {
        c8 buf [OPS_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_expect_ok(t, sp_sys_socket_recv(h.client, buf, sizeof(buf), &n));
        sp_expect_str_eq_c(t, sp_str(buf, (u32)n), step->peer_recv.expect);
        break;
      }
      case STEP_PEER_CLOSE: {
        sp_sys_socket_close(h.client);
        h.client = SP_SYS_INVALID_SOCKET;
        break;
      }
      case STEP_PEER_CONNECT: {
        sp_try(harness_connect(t, &h));
        break;
      }
      case STEP_PIPE_WRITE: {
        u64 len = sp_cstr_len(step->pipe_write.data);
        u64 n = 0;
        sp_expect_ok(t, sp_sys_write(h.pipe.w, step->pipe_write.data, len, &n));
        sp_expect_eq(t, n, len);
        break;
      }
      case STEP_PIPE_READ: {
        c8 buf [OPS_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_expect_ok(t, sp_sys_read(h.pipe.r, buf, sizeof(buf), &n));
        sp_expect_str_eq_c(t, sp_str(buf, (u32)n), step->pipe_read.expect);
        break;
      }
      case STEP_PIPE_CLOSE_W: {
        sp_sys_close(h.pipe.w);
        h.pipe.w = SP_SYS_INVALID_FD;
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, transfer, test_t, tests, run);
