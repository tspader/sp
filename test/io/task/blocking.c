#include "sp.h"
#include "sp/sp_test.h"
#include "sp/sp_io.h"

typedef struct {
  bool ran;
} blocking_work_t;

static void blocking_work_fn(void* context) {
  blocking_work_t* w = sp_cast(blocking_work_t*, context);
  w->ran = true;
}

sp_test(task, blocking_ops_complete_inline) {
  sp_test_skip_on_freestanding();

  sp_io_blocking_t io_mem;
  sp_io_t io = sp_io_blocking_init(&io_mem);

  sp_sys_pipe_t pipe;
  sp_must_ok(t, sp_sys_pipe(&pipe, sp_zero_s(sp_sys_pipe_desc_t)));

  sp_str_t payload = sp_str_lit("spum");
  sp_must_ok(t, sp_io_fd_write_all(io, pipe.w, sp_mem_slice((u8*)payload.data, payload.len)));

  u8 buf [8] = sp_zero;
  u64 n = 0;
  sp_must_ok(t, sp_io_fd_read(io, pipe.r, sp_mem_slice(buf, sizeof(buf)), &n));
  sp_expect_eq(t, n, (u64)payload.len);
  sp_expect_str_eq_c(t, sp_str((const c8*)buf, (u32)n), "spum");

  bool tty = true;
  sp_must_ok(t, sp_io_is_tty(io, pipe.r, &tty));
  sp_expect_eq(t, tty, false);

  blocking_work_t work = sp_zero;
  sp_must_ok(t, sp_io_work(io, blocking_work_fn, &work));
  sp_expect_eq(t, work.ran, true);

  sp_tm_timer_t timer = sp_tm_start_timer();
  sp_must_ok(t, sp_io_sleep(io, sp_tm_ms_to_ns(1)));
  sp_expect_ge(t, sp_tm_read_timer(&timer), sp_tm_ms_to_ns(1));

  sp_sys_close(pipe.r);
  sp_sys_close(pipe.w);
  sp_io_destroy(io);
  return SP_OK;
}
