#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

typedef struct {
  sp_err_t open;
  sp_err_t walk;
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
      { .read = SP_ERR_SYS_BAD_FD },
    },
    .expect = {
      .walk = SP_ERR_SYS_BAD_FD,
    },
  },
  {
    .name = "propagates_it_open_error",
    .dirs = {
      { .it_open = SP_ERR_SYS_IO },
    },
    .expect = {
      .open = SP_ERR_SYS_IO,
    },
  },
};

sp_test_each(fs, dir_sim, test_t, tests, .serial = true) {
  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  sp_sys_dir_t dir = sp_zero;
  sp_err_t err = sp_fs_open_dir_at(sim_root(), &dir);
  sp_expect_ok(t, err);

  sp_err_t open_err = SP_OK;
  if (!err) {
    sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
    open_err = iter.err;
    sp_expect_err_eq(t, open_err, it->expect.open);

    if (!open_err) {
      SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
      sp_fs_dir_entry_t entry = sp_zero;
      while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) {}
      sp_expect_err_eq(t, iter.err, it->expect.walk);
      sp_expect_ok(t, sp_fs_dir_it_close(&iter));
    }
    sp_fs_close_dir(dir);
  }

  sim_end(&s);

  sp_expect_eq(t, s.count.closes, open_err ? (u32)0 : (u32)1);
  sp_expect_eq(t, s.count.fd_closes, (u32)1);
  return SP_OK;
}
