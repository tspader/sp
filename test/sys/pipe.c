#include "sys.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(sys_pipe)

typedef enum {
  SYS_PIPE_STEP_NONE,
  SYS_PIPE_STEP_WRITE,
  SYS_PIPE_STEP_READ,
  SYS_PIPE_STEP_CLOSE_READ,
  SYS_PIPE_STEP_CLOSE_WRITE,
} sys_pipe_step_kind_t;

typedef struct {
  sys_pipe_step_kind_t kind;
  union {
    struct { const c8* data; sp_err_t err; } write;
    struct { const c8* expect; sp_err_t err; } read;
  };
} sys_pipe_step_t;

typedef struct {
  sys_pipe_step_t steps [SYS_TEST_MAX_STEPS];
} sys_pipe_test_t;

static void run_sys_pipe_test(s32* utest_result, sys_pipe_test_t t) {
  sp_sys_fd_t r = SP_SYS_INVALID_FD;
  sp_sys_fd_t w = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_pipe(&r, &w), SP_OK);

  sp_carr_for(t.steps, it) {
    sys_pipe_step_t* step = &t.steps[it];
    if (step->kind == SYS_PIPE_STEP_NONE) break;

    switch (step->kind) {
      case SYS_PIPE_STEP_NONE: break;

      case SYS_PIPE_STEP_WRITE: {
        u64 len = sp_cstr_len(step->write.data);
        u64 n = 0;
        sp_err_t err = sp_sys_write(w, step->write.data, len, &n);
        sys_expect_err(utest_result, "write", err, step->write.err);
        if (err == SP_OK && !step->write.err) {
          EXPECT_EQ(n, len);
        }
        break;
      }
      case SYS_PIPE_STEP_READ: {
        c8 buf [SYS_TEST_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_err_t err = sp_sys_read(r, buf, sizeof(buf), &n);
        sys_expect_err(utest_result, "read", err, step->read.err);
        if (err == SP_OK && !step->read.err) {
          sys_expect_bytes(utest_result, "read", buf, (s64)n, step->read.expect);
        }
        break;
      }
      case SYS_PIPE_STEP_CLOSE_READ: {
        sp_sys_close(r);
        r = SP_SYS_INVALID_FD;
        break;
      }
      case SYS_PIPE_STEP_CLOSE_WRITE: {
        sp_sys_close(w);
        w = SP_SYS_INVALID_FD;
        break;
      }
    }
  }

  if (r != SP_SYS_INVALID_FD) sp_sys_close(r);
  if (w != SP_SYS_INVALID_FD) sp_sys_close(w);
}

UTEST_F(sys_pipe, roundtrip) {
  run_sys_pipe_test(utest_result, (sys_pipe_test_t) {
    .steps = {
      { .kind = SYS_PIPE_STEP_WRITE, .write = { .data = "hello" } },
      { .kind = SYS_PIPE_STEP_READ, .read = { .expect = "hello" } },
    },
  });
}

UTEST_F(sys_pipe, empty_read_would_block) {
  run_sys_pipe_test(utest_result, (sys_pipe_test_t) {
    .steps = {
      { .kind = SYS_PIPE_STEP_READ, .read = { .err = SP_ERR_SYS_WOULD_BLOCK } },
    },
  });
}

UTEST_F(sys_pipe, read_after_writer_closed_is_eof) {
  run_sys_pipe_test(utest_result, (sys_pipe_test_t) {
    .steps = {
      { .kind = SYS_PIPE_STEP_WRITE, .write = { .data = "x" } },
      { .kind = SYS_PIPE_STEP_CLOSE_WRITE },
      { .kind = SYS_PIPE_STEP_READ, .read = { .expect = "x" } },
      { .kind = SYS_PIPE_STEP_READ, .read = { .expect = "" } },
    },
  });
}

// A write that moves zero bytes must report WOULD_BLOCK, never (0, SP_OK) —
// win32 reports a full PIPE_NOWAIT pipe as WriteFile success with zero
// written, and sp_sys_write translates it; POSIX reports EAGAIN natively.
UTEST_F(sys_pipe, write_to_full_pipe_reports_would_block) {
  sp_sys_fd_t r = SP_SYS_INVALID_FD;
  sp_sys_fd_t w = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_pipe(&r, &w), SP_OK);

  u8 chunk [4096] = sp_zero;
  sp_err_t err = SP_OK;
  while (err == SP_OK) {
    u64 n = 0;
    err = sp_sys_write(w, chunk, sizeof(chunk), &n);
    if (err == SP_OK) EXPECT_TRUE(n > 0);
  }
  EXPECT_EQ(err, SP_ERR_SYS_WOULD_BLOCK);

  sp_sys_close(r);
  sp_sys_close(w);
}

#if defined(SP_WIN32)
UTEST_F(sys_pipe, write_after_reader_closed_reports_broken_pipe) {
  run_sys_pipe_test(utest_result, (sys_pipe_test_t) {
    .steps = {
      { .kind = SYS_PIPE_STEP_CLOSE_READ },
      { .kind = SYS_PIPE_STEP_WRITE, .write = { .data = "x", .err = SP_ERR_SYS_BROKEN_PIPE } },
    },
  });
}
#endif

#endif
