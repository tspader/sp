#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

typedef struct {
  sp_err_t open;
  sp_err_t walk;
  u32 fd_closes;
} expect_t;

typedef struct {
  const c8* name;
  sim_dir_t dirs [SIM_MAX_DIRS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "propagates_read_error",
    .dirs = {
      { .path = "T", .read = SP_ERR_SYS_BAD_FD },
    },
    .expect = {
      .walk = SP_ERR_SYS_BAD_FD,
    },
  },
  {
    .name = "closes_fd_when_from_fd_fails",
    .dirs = {
      { .path = "T", .from_fd = SP_ERR_SYS_IO },
    },
    .expect = {
      .open = SP_ERR_SYS_IO,
      .fd_closes = 1,
    },
  },
};

sp_test_each(fs, dir_sim, test_t, tests, .serial = true) {
  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_t dir = sp_zero;
  sp_err_t open_err = sp_fs_dir_open(&dir, sp_sys_get_root(0), sp_str_lit("T"), sp_mem_slice(buf, sizeof(buf)));
  sp_expect_err_eq(t, open_err, it->expect.open);

  if (!open_err) {
    sp_err_t walk = SP_OK;
    while (true) {
      sp_fs_dir_entry_t entry = sp_zero;
      walk = sp_fs_dir_next(&dir, &entry);
      if (walk || !entry.name.data) break;
    }
    sp_expect_err_eq(t, walk, it->expect.walk);
    sp_expect_ok(t, sp_fs_dir_close(&dir));
  }

  sim_end(&s);

  sp_expect_eq(t, s.count.fd_closes, it->expect.fd_closes);
  sp_expect_eq(t, s.count.closes, open_err ? (u32)0 : (u32)1);
  return SP_OK;
}
