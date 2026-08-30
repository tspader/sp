#include "harness.h"

sp_test(io, new_yields_usable_default) {
  sp_io_t io = sp_zero;
  sp_err_t err = sp_io_new(&io);
  if (err == SP_ERR_SYS_UNSUPPORTED || err == SP_ERR_SYS_ACCESS_DENIED) {
    return sp_test_skip(t, "no default io for this platform");
  }
  sp_must_ok(t, err);

  sp_io_op_t op = { .kind = SP_IO_OP_TIMEOUT, .timeout = { .timeout = sp_io_timeout_after(0) } };
  sp_must_ok(t, sp_io_submit(io, &op));

  if (!op.done) {
    u32 count = 0;
    sp_expect_ok(t, sp_io_dispatch(io, sp_io_timeout_after(sp_tm_ms_to_ns(OPS_WAIT_MS)), &count));
    sp_expect_eq(t, count, (u32)1);
  }
  sp_expect_eq(t, op.done, true);
  sp_expect_ok(t, op.result.err);

  sp_io_destroy(io);
  return SP_OK;
}
