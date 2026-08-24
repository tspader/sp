#include "harness.h"

#define CANCEL_MAX_STEPS 6
#define CANCEL_LONG_MS   10000

typedef enum {
  STEP_NONE,
  STEP_SUBMIT,
  STEP_FLUSH,
  STEP_PEER_SEND,
  STEP_CANCEL,
  STEP_WAIT,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    ops_submit_t submit;
    struct { const c8* data; } peer_send;
    struct { u32 slot; } cancel;
    ops_wait_t wait;
  };
} step_t;

typedef struct {
  const c8* name;
  ops_fixture_t fixture;
  step_t steps [CANCEL_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "recv_in_flight",
    .fixture = FIXTURE_SOCKETS,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_RECV, .handle = H_SERVER, .size = 8 } },
      { .kind = STEP_FLUSH },
      { .kind = STEP_CANCEL },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .err = SP_ERR_IO_CANCELED } } } } },
    },
  },
  {
    .name = "accept_in_flight",
    .fixture = FIXTURE_LISTENER,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_ACCEPT, .handle = H_LISTENER } },
      { .kind = STEP_FLUSH },
      { .kind = STEP_CANCEL },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .err = SP_ERR_IO_CANCELED } } } } },
    },
  },
  {
    .name = "timeout_in_flight",
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_TIMEOUT, .ms = CANCEL_LONG_MS } },
      { .kind = STEP_FLUSH },
      { .kind = STEP_CANCEL },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .err = SP_ERR_IO_CANCELED } } } } },
    },
  },
  {
    .name = "loses_to_already_completed_recv",
    .fixture = FIXTURE_SOCKETS,
    .steps = {
      { .kind = STEP_PEER_SEND, .peer_send = { .data = "A" } },
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_RECV, .handle = H_SERVER, .size = 8 } },
      { .kind = STEP_CANCEL },
      { .kind = STEP_WAIT, .wait = { .expect = { .count = 1, .done = { { .len = 1, .content = "A" } } } } },
    },
  },
  {
    .name = "never_submitted_op_completes_nothing",
    .steps = {
      { .kind = STEP_CANCEL },
      { .kind = STEP_WAIT, .wait = { .kind = WAIT_AFTER, .ms = 50, .expect = { .min_elapsed_ms = 50 } } },
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
      case STEP_FLUSH: {
        ops_wait_t flush = { .kind = WAIT_AFTER };
        harness_wait(t, &h, &flush);
        break;
      }
      case STEP_PEER_SEND: {
        u64 len = sp_cstr_len(step->peer_send.data);
        u64 n = 0;
        sp_expect_ok(t, sp_sys_socket_send(h.client, step->peer_send.data, len, &n));
        sp_expect_eq(t, n, len);
        sp_must_ok(t, sp_sys_socket_wait(h.server, true, OPS_WAIT_MS));
        break;
      }
      case STEP_CANCEL: {
        sp_expect_ok(t, sp_io_cancel(h.io, &h.ops[step->cancel.slot]));
        break;
      }
      case STEP_WAIT: {
        harness_wait(t, &h, &step->wait);
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, cancel, test_t, tests, run);
