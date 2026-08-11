#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

typedef enum {
  FD_PIPE_R,
  FD_PIPE_W,
  FD_INVALID,
} fd_kind_t;

typedef struct {
  sp_err_t get;
} expect_t;

typedef struct {
  const c8* name;
  fd_kind_t fd;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "pipe_read_end_is_not_a_tty",
    .expect = { .get = SP_ERR_SYS_NOT_TTY },
  },
  {
    .name = "pipe_write_end_is_not_a_tty",
    .fd = FD_PIPE_W,
    .expect = { .get = SP_ERR_SYS_NOT_TTY },
  },
  {
    .name = "invalid_fd_is_not_a_tty",
    .fd = FD_INVALID,
    .expect = { .get = SP_ERR_SYS_BAD_FD },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_pipe_t p = { SP_SYS_INVALID_FD, SP_SYS_INVALID_FD };
  sp_sys_fd_t fd = SP_SYS_INVALID_FD;

  if (c->fd != FD_INVALID) {
    sp_must_ok(t, sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)));
    fd = c->fd == FD_PIPE_R ? p.r : p.w;
  }

  sp_expect(t, !sp_sys_is_tty(fd));

  sp_sys_tty_attr_t attr = sp_zero;
  sp_expect_err_eq(t, sp_sys_tty_get(fd, &attr), c->expect.get);
  sp_expect(t, !attr.present);

  sp_expect_ok(t, sp_sys_tty_set(fd, &attr));

  sp_sys_tty_attr_t out = sp_zero;
  sp_expect_err_eq(t, sp_sys_tty_mode_apply(&attr, &out, SP_SYS_TTY_MODE_RAW), SP_ERR_SYS_NOT_TTY);

  u32 cols = 1;
  u32 rows = 1;
  sp_expect_ne(t, sp_sys_tty_size(fd, &cols, &rows), SP_OK);
  sp_expect_eq(t, cols, (u32)0);
  sp_expect_eq(t, rows, (u32)0);

  sp_sys_tty_state_t saved = sp_zero;
  sp_expect_ne(t, sp_tty_set_mode(fd, fd, SP_SYS_TTY_MODE_RAW, &saved), SP_OK);
  sp_expect(t, !saved.in.present);
  sp_expect(t, !saved.out.present);
  sp_expect_ok(t, sp_tty_restore(fd, fd, &saved));

  if (p.r != SP_SYS_INVALID_FD) sp_sys_close(p.r);
  if (p.w != SP_SYS_INVALID_FD) sp_sys_close(p.w);
  return SP_OK;
}

sp_test_each_fn(sys, tty, test_t, tests, run);

#endif
