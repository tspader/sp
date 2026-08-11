#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define PIPE_MAX_STEPS 8
#define PIPE_BUF_SIZE 64
#define PIPE_FILL_CHUNK 4096

typedef enum {
  STEP_NONE,
  STEP_WRITE,
  STEP_READ,
  STEP_READY,
  STEP_FILL,
  STEP_CLOSE_R,
  STEP_CLOSE_W,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { const c8* data; sp_err_t err; } write;
    struct { const c8* expect; sp_err_t err; } read;
    struct { u8 expect; } ready;
  };
} step_t;

typedef struct {
  const c8* name;
  sp_sys_pipe_desc_t desc;
  step_t steps [PIPE_MAX_STEPS];
} test_t;

// A Win32 NONBLOCKING pipe end is an overlapped handle; sp_sys_read/write on
// it is a contract violation, so the WOULD_BLOCK cases are POSIX-only. The
// BROKEN_PIPE write case is Win32-only; on POSIX the write raises SIGPIPE.
static const test_t tests [] = {
  {
    .name = "zero_desc_roundtrip",
    .steps = {
      { .kind = STEP_WRITE, .write = { .data = "A" } },
      { .kind = STEP_READ, .read = { .expect = "A" } },
    },
  },
  {
    .name = "zero_desc_eof_after_writer_close",
    .steps = {
      { .kind = STEP_WRITE, .write = { .data = "A" } },
      { .kind = STEP_CLOSE_W },
      { .kind = STEP_READ, .read = { .expect = "A" } },
      { .kind = STEP_READ, .read = { .expect = "" } },
    },
  },
  {
    .name = "not_ready_when_empty",
    .steps = {
      { .kind = STEP_READY },
    },
  },
  {
    .name = "ready_after_write",
    .steps = {
      { .kind = STEP_WRITE, .write = { .data = "A" } },
      { .kind = STEP_READY, .ready = { .expect = 1 } },
    },
  },
  {
    .name = "ready_while_draining_after_writer_close",
    .steps = {
      { .kind = STEP_WRITE, .write = { .data = "A" } },
      { .kind = STEP_CLOSE_W },
      { .kind = STEP_READY, .ready = { .expect = 1 } },
      { .kind = STEP_READ, .read = { .expect = "A" } },
      { .kind = STEP_READY, .ready = { .expect = 1 } },
    },
  },
#if !defined(SP_WIN32)
  {
    .name = "empty_read_would_block",
    .desc = { .r = { SP_SYS_NONBLOCKING } },
    .steps = {
      { .kind = STEP_READ, .read = { .err = SP_ERR_SYS_WOULD_BLOCK } },
    },
  },
  {
    .name = "full_write_would_block",
    .desc = { .w = { SP_SYS_NONBLOCKING } },
    .steps = {
      { .kind = STEP_FILL },
    },
  },
#else
  {
    .name = "write_after_reader_close_reports_broken_pipe",
    .steps = {
      { .kind = STEP_CLOSE_R },
      { .kind = STEP_WRITE, .write = { .data = "A", .err = SP_ERR_SYS_BROKEN_PIPE } },
    },
  },
#endif
};

static const c8* step_name(step_kind_t kind) {
  switch (kind) {
    case STEP_NONE:    return "none";
    case STEP_WRITE:   return "write";
    case STEP_READ:    return "read";
    case STEP_READY:   return "ready";
    case STEP_FILL:    return "fill";
    case STEP_CLOSE_R: return "close_r";
    case STEP_CLOSE_W: return "close_w";
  }
  return "";
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_pipe_t p = sp_zero;
  sp_must_ok(t, sp_sys_pipe(&p, c->desc));
  sp_sys_fd_t r = p.r;
  sp_sys_fd_t w = p.w;

  sp_carr_for(c->steps, it) {
    step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "[{}] {}", sp_fmt_uint(it), sp_fmt_cstr(step_name(step->kind))));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_WRITE: {
        u64 len = sp_cstr_len(step->write.data);
        u64 n = 0;
        sp_err_t err = sp_sys_write(w, step->write.data, len, &n);
        sp_expect_err_eq(t, err, step->write.err);
        if (!err && !step->write.err) {
          sp_expect_eq(t, n, len);
        }
        break;
      }
      case STEP_READ: {
        c8 buf [PIPE_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_err_t err = sp_sys_read(r, buf, sizeof(buf), &n);
        sp_expect_err_eq(t, err, step->read.err);
        if (!err && !step->read.err) {
          u64 want = sp_cstr_len(step->read.expect);
          if (n != want || (n && !sp_mem_is_equal(buf, step->read.expect, want))) {
            sp_test_record(t, (sp_test_failure_t) {
              .message = sp_test_format(t, "read {} of {} bytes", sp_fmt_uint(n), sp_fmt_uint(want)),
              .expected = sp_test_format(t, "{.quote}", sp_fmt_cstr(step->read.expect)),
              .actual = sp_test_format(t, "{.quote}", sp_fmt_str(sp_str(buf, (u32)n))),
            });
          }
        }
        break;
      }
      case STEP_READY: {
        u8 ready = 0;
        sp_expect_ok(t, sp_sys_pipe_ready(r, &ready));
        sp_expect_eq(t, ready, step->ready.expect);
        break;
      }
      case STEP_FILL: {
        u8 chunk [PIPE_FILL_CHUNK] = sp_zero;
        sp_err_t err = SP_OK;
        while (!err) {
          u64 n = 0;
          err = sp_sys_write(w, chunk, sizeof(chunk), &n);
          if (!err) sp_expect_gt(t, n, (u64)0);
        }
        sp_expect_err_eq(t, err, SP_ERR_SYS_WOULD_BLOCK);
        break;
      }
      case STEP_CLOSE_R: {
        sp_sys_close(r);
        r = SP_SYS_INVALID_FD;
        break;
      }
      case STEP_CLOSE_W: {
        sp_sys_close(w);
        w = SP_SYS_INVALID_FD;
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  if (r != SP_SYS_INVALID_FD) sp_sys_close(r);
  if (w != SP_SYS_INVALID_FD) sp_sys_close(w);
  return SP_OK;
}

sp_test_each_fn(sys, pipe, test_t, tests, run);

#endif
