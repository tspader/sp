#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define TRANSFER_MAX_XFERS 2
#define TRANSFER_BUF_SIZE 64
#define TRANSFER_MOVED_SENTINEL 99

typedef enum {
  END_FILE,
  END_PIPE,
} end_t;

typedef enum {
  OP_STREAM,
  OP_POSITIONAL,
} op_t;

typedef struct {
  u64 moved;
  u64 in_pos;
  sp_err_t err;
} xfer_expect_t;

typedef struct {
  u64 count;
  u64 offset;
  xfer_expect_t expect;
} xfer_t;

typedef struct {
  const c8* name;
  end_t src;
  end_t dst;
  op_t op;
  bool track;
  const c8* data;
  const c8* prefill;
  xfer_t xfers [TRANSFER_MAX_XFERS];
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "copies_file_to_file",
    .op = OP_POSITIONAL,
    .track = true,
    .data = "AB",
    .xfers = {
      { .count = 64, .expect = { .moved = 2, .in_pos = 2 } },
    },
    .expect = "AB",
  },
  {
    .name = "streams_file_to_pipe",
    .dst = END_PIPE,
    .track = true,
    .data = "AB",
    .xfers = {
      { .count = 64, .expect = { .moved = 2, .in_pos = 2 } },
    },
    .expect = "AB",
  },
  {
    .name = "positional_pipe_source_unsupported",
    .src = END_PIPE,
    .op = OP_POSITIONAL,
    .data = "A",
    .xfers = {
      { .count = 64, .expect = { .err = SP_ERR_SYS_UNSUPPORTED } },
    },
  },
  {
    .name = "streaming_pipe_source_unsupported",
    .src = END_PIPE,
    .dst = END_PIPE,
    .data = "A",
    .xfers = {
      { .count = 64, .expect = { .err = SP_ERR_SYS_UNSUPPORTED } },
    },
  },
  {
    .name = "reports_zero_moved_at_eof",
    .op = OP_POSITIONAL,
    .track = true,
    .data = "ABCD",
    .xfers = {
      { .count = 64, .expect = { .moved = 4, .in_pos = 4 } },
      { .count = 64, .offset = 4, .expect = { .in_pos = 4 } },
    },
    .expect = "ABCD",
  },
  {
    .name = "positional_writes_at_offset",
    .op = OP_POSITIONAL,
    .track = true,
    .data = "AB",
    .prefill = "....",
    .xfers = {
      { .count = 64, .offset = 2, .expect = { .moved = 2, .in_pos = 2 } },
    },
    .expect = "..AB",
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  // Transfer has a fast path only on Linux; everywhere else the first call
  // reports UNSUPPORTED and moves nothing
#if !defined(SP_LINUX)
  c->xfers[0] = (xfer_t) { .count = 1, .expect = { .err = SP_ERR_SYS_UNSUPPORTED } };
  c->xfers[1] = sp_zero_s(xfer_t);
  c->expect = SP_NULLPTR;
#endif

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  sp_sys_fd_t in = SP_SYS_INVALID_FD;
  sp_sys_fd_t out = SP_SYS_INVALID_FD;
  sp_sys_fd_t src_w = SP_SYS_INVALID_FD;
  sp_sys_fd_t dst_r = SP_SYS_INVALID_FD;
  u64 len = sp_cstr_len(c->data);

  sp_try(sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd));

  switch (c->src) {
    case END_FILE: {
      sp_fs_create_file_str(sp_fs_join_path(mem, sandbox, sp_str_lit("src")), sp_cstr_as_str(c->data));
      sp_must_ok(t, sp_sys_open_s(sandbox_fd, sp_str_lit("src"), SP_SYS_OPEN_MODE_RO, 0, &in));
      break;
    }
    case END_PIPE: {
      sp_sys_pipe_t p = sp_zero;
      sp_must_ok(t, sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)));
      in = p.r;
      src_w = p.w;
      u64 n = 0;
      sp_must_ok(t, sp_sys_write(src_w, c->data, len, &n));
      sp_must_eq(t, n, len);
      break;
    }
  }

  switch (c->dst) {
    case END_FILE: {
      u32 flags = SP_SYS_OPEN_CREATE;
      if (c->prefill) {
        sp_fs_create_file_str(sp_fs_join_path(mem, sandbox, sp_str_lit("dst")), sp_cstr_as_str(c->prefill));
        flags = 0;
      }
      sp_must_ok(t, sp_sys_open_s(sandbox_fd, sp_str_lit("dst"), SP_SYS_OPEN_MODE_WO, flags, &out));
      break;
    }
    case END_PIPE: {
      sp_sys_pipe_t p = sp_zero;
      sp_must_ok(t, sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)));
      dst_r = p.r;
      out = p.w;
      break;
    }
  }

  u64 in_pos = 0;
  sp_carr_for(c->xfers, it) {
    const xfer_t* xf = &c->xfers[it];
    if (!xf->count) break;
    sp_test_kv(t, "xfer", sp_test_format(t, "{}", sp_fmt_uint(it)));

    u64 moved = TRANSFER_MOVED_SENTINEL;
    sp_err_t err = c->op == OP_POSITIONAL
      ? sp_sys_transfer_positional(in, c->track ? &in_pos : SP_NULLPTR, out, xf->count, xf->offset, &moved)
      : sp_sys_transfer(in, c->track ? &in_pos : SP_NULLPTR, out, xf->count, &moved);
    sp_expect_err_eq(t, err, xf->expect.err);
    sp_expect_eq(t, moved, xf->expect.moved);
    if (c->track) sp_expect_eq(t, in_pos, xf->expect.in_pos);
  }
  sp_test_kv_clear(t, "xfer");

  if (c->expect) {
    switch (c->dst) {
      case END_FILE: {
        sp_sys_close(out);
        out = SP_SYS_INVALID_FD;
        sp_str_t actual = sp_zero;
        sp_io_read_file(mem, sp_fs_join_path(mem, sandbox, sp_str_lit("dst")), &actual);
        sp_expect_str_eq_c(t, actual, c->expect);
        break;
      }
      case END_PIPE: {
        c8 buf [TRANSFER_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_expect_ok(t, sp_sys_read(dst_r, buf, sizeof(buf), &n));
        sp_expect_str_eq_c(t, sp_str(buf, (u32)n), c->expect);
        break;
      }
    }
  }

  if (in != SP_SYS_INVALID_FD) sp_sys_close(in);
  if (out != SP_SYS_INVALID_FD) sp_sys_close(out);
  if (src_w != SP_SYS_INVALID_FD) sp_sys_close(src_w);
  if (dst_r != SP_SYS_INVALID_FD) sp_sys_close(dst_r);
  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
  return SP_OK;
}

sp_test_each_fn(sys, transfer, test_t, tests, run);

#endif
