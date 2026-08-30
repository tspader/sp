#include "harness.h"
#include "test/probe.h"

#if !defined(SP_WASM) && !defined(SP_FREESTANDING)

typedef struct {
  const c8* name;
  sp_sys_inherited_t inherited;
} test_t;

static const test_t tests [] = {
  { .name = "accepted_socket" },
};

static const c8* inherited_name(s64 value) {
  switch ((sp_sys_inherited_t)value) {
    case SP_SYS_NOT_INHERITED: return "no";
    case SP_SYS_INHERITED:     return "yes";
  }
  return "?";
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  s32 want_exit = c->inherited == SP_SYS_INHERITED ? 0 : 1;
  u64 want_bytes = c->inherited == SP_SYS_INHERITED ? 1 : 0;

  ops_harness_t h;
  sp_try(harness_open(t, &h, OPS_ENTRIES, FIXTURE_LISTENER));

  sp_io_op_t* op = &h.ops[0];
  op->kind = SP_IO_OP_ACCEPT;
  op->accept.socket = h.listener;
  op->accept.desc = (sp_sys_handle_desc_t) { .inherited = c->inherited };
  harness_arm(&h, op);
  sp_must_ok(t, sp_io_submit(h.io, op));
  sp_try(harness_connect(t, &h));

  ops_wait_t wait = { .expect = { .count = 1 } };
  harness_wait(t, &h, &wait);
  sp_must_ne(t, h.accepted, SP_SYS_INVALID_SOCKET);

  sp_expect_eq(t, spawn_probe(t, "send", (s64)h.accepted), want_exit);
  sp_sys_socket_close(h.accepted);
  h.accepted = SP_SYS_INVALID_SOCKET;

  c8 byte = 0;
  u64 n = 0;
  sp_expect_ok(t, sp_sys_socket_recv(h.client, &byte, 1, &n));
  sp_expect_eq(t, n, want_bytes);
  if (n == 1) sp_expect_eq(t, byte, 'A');

  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, inherit, test_t, tests, run, .axes = {
  sp_test_axis_named(test_t, inherited, inherited_name, SP_SYS_NOT_INHERITED, SP_SYS_INHERITED)
});

#endif
