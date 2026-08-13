#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define PIPE_MAX_STEPS 8
#define PIPE_BUF_SIZE 64

typedef enum {
  STEP_NONE,
  STEP_WRITE,
  STEP_READ,
  STEP_READY,
  STEP_PREAD,
  STEP_PWRITE,
  STEP_CLOSE_R,
  STEP_CLOSE_W,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { const c8* data; sp_err_t err; } write;
    struct { const c8* expect; sp_err_t err; } read;
    struct { u8 expect; } ready;
    struct { u64 offset; sp_err_t err; } pread;
    struct { const c8* data; u64 offset; sp_err_t err; } pwrite;
  };
} step_t;

typedef struct {
  const c8* name;
  sp_sys_pipe_desc_t desc;
  step_t steps [PIPE_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "zero_desc_roundtrip",
    .steps = {
      { .kind = STEP_WRITE, .write = { .data = "A" } },
      { .kind = STEP_READ, .read = { .expect = "A" } },
    },
  },
  {
    // Primed with data so an implementation that honors the offset consumes
    // stream bytes instead of blocking.
    .name = "pread_refuses_positioned_read",
    .steps = {
      { .kind = STEP_WRITE, .write = { .data = "AB" } },
      { .kind = STEP_PREAD, .pread = { .err = SP_ERR_SYS_UNSEEKABLE } },
    },
  },
  {
    .name = "pwrite_refuses_positioned_write",
    .steps = {
      { .kind = STEP_PWRITE, .pwrite = { .data = "A", .err = SP_ERR_SYS_UNSEEKABLE } },
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
  {
    .name = "write_after_reader_close_reports_broken_pipe",
    .steps = {
      { .kind = STEP_CLOSE_R },
      { .kind = STEP_WRITE, .write = { .data = "A", .err = SP_ERR_SYS_BROKEN_PIPE } },
    },
  },
};

static const c8* step_name(step_kind_t kind) {
  switch (kind) {
    case STEP_NONE:    return "none";
    case STEP_WRITE:   return "write";
    case STEP_READ:    return "read";
    case STEP_READY:   return "ready";
    case STEP_PREAD:   return "pread";
    case STEP_PWRITE:  return "pwrite";
    case STEP_CLOSE_R: return "close_r";
    case STEP_CLOSE_W: return "close_w";
  }
  return "";
}

static sp_err_t run(sp_test_t* t, test_t* c) {
#if defined(SP_FREESTANDING)
  sp_carr_for(c->steps, it) {
    if (c->steps[it].kind == STEP_WRITE && c->steps[it].write.err == SP_ERR_SYS_BROKEN_PIPE) {
      return sp_test_skip(t, "no libc to own the SIGPIPE disposition");
    }
  }
#elif !defined(SP_WIN32)
  // EPIPE only surfaces as an error while SIGPIPE is ignored; the disposition
  // is process-global, so this suite owns it instead of the shared test main
  static bool sigpipe_ignored = false;
  if (!sigpipe_ignored) {
    signal(SIGPIPE, SIG_IGN);
    sigpipe_ignored = true;
  }
#endif

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
      case STEP_PREAD: {
        c8 buf [PIPE_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_expect_err_eq(t, sp_sys_pread(r, buf, 1, step->pread.offset, &n), step->pread.err);
        break;
      }
      case STEP_PWRITE: {
        u64 n = 0;
        sp_expect_err_eq(t, sp_sys_pwrite(w, step->pwrite.data, sp_cstr_len(step->pwrite.data), step->pwrite.offset, &n), step->pwrite.err);
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
