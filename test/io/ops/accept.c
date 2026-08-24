#include "harness.h"

#define ACCEPT_SEND_DELAY_MS 50

typedef struct {
  sp_err_t recv;
} expect_t;

typedef struct {
  const c8* name;
  sp_sys_handle_desc_t desc;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "nonblocking_desc_applies_to_accepted_socket",
    .desc = { .mode = SP_SYS_NONBLOCKING },
    .expect = { .recv = SP_ERR_SYS_WOULD_BLOCK },
  },
};

typedef struct {
  sp_sys_socket_t socket;
  sp_err_t err;
} sender_t;

static s32 sender(void* user_data) {
  sender_t* s = (sender_t*)user_data;
  sp_os_sleep_ms(ACCEPT_SEND_DELAY_MS);
  u64 n = 0;
  s->err = sp_sys_socket_send(s->socket, "A", 1, &n);
  return 0;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
#if defined(SP_FREESTANDING)
  sp_unused(c);
  return sp_test_skip(t, "threads are unsupported on freestanding");
#else
  ops_harness_t h;
  sp_try(harness_open(t, &h, OPS_ENTRIES, FIXTURE_LISTENER));

  sp_io_op_t* op = &h.ops[0];
  op->kind = SP_IO_OP_ACCEPT;
  op->accept.socket = h.listener;
  op->accept.desc = c->desc;
  sp_must_ok(t, sp_io_submit(h.io, op));
  sp_try(harness_connect(t, &h));

  ops_wait_t wait = { .expect = { .count = 1 } };
  harness_wait(t, &h, &wait);
  sp_must_ne(t, h.accepted, SP_SYS_INVALID_SOCKET);

  sender_t s = { .socket = h.client };
  sp_thread_t thread = sp_zero;
  sp_thread_init(&thread, sender, &s);
  c8 buf [OPS_BUF_SIZE] = sp_zero;
  u64 n = 0;
  sp_expect_err_eq(t, sp_sys_socket_recv(h.accepted, buf, sizeof(buf), &n), c->expect.recv);
  sp_thread_join(&thread);
  sp_expect_ok(t, s.err);

  harness_close(&h);
  return SP_OK;
#endif
}

sp_test_each_fn(io, accept, test_t, tests, run);
