#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

typedef struct {
  const c8* path;
  sim_op_t op;
} removed_t;

typedef struct {
  sp_err_t err;
  removed_t removed [SIM_MAX_REMOVED];
  u32 opens;
  u32 rmdirs;
} expect_t;

typedef struct {
  const c8* name;
  sim_dir_t dirs [SIM_MAX_DIRS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "entry_vanished_is_ok",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_FILE, .unlink = SP_ERR_SYS_NOT_FOUND }, { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "T/C", SIM_OP_UNLINK }, { "T", SIM_OP_RMDIR } },
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "dir_vanished_before_open_is_ok",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B", .open = SP_ERR_SYS_NOT_FOUND },
    },
    .expect = {
      .removed = { { "T", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "root_vanished_after_walk_is_ok",
    .dirs = {
      { .path = "T", .rmdir = SP_ERR_SYS_NOT_FOUND, .entries = { { "B", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "T/B", SIM_OP_UNLINK } },
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "file_hint_that_is_dir_is_walked",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_FILE, .unlink = SP_ERR_SYS_IS_DIR } } },
      { .path = "T/B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "T/B/C", SIM_OP_UNLINK }, { "T/B", SIM_OP_RMDIR }, { "T", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 2,
    },
  },
  {
    .name = "dir_hint_that_is_not_dir_is_unlinked",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B", .open = SP_ERR_SYS_NOT_DIR },
    },
    .expect = {
      .removed = { { "T/B", SIM_OP_UNLINK }, { "T", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "dir_hint_that_is_symlink_is_unlinked",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B", .open = SP_ERR_SYS_LOOP },
    },
    .expect = {
      .removed = { { "T/B", SIM_OP_UNLINK }, { "T", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "not_empty_propagates_without_rescan",
    .dirs = {
      { .path = "T", .rmdir = SP_ERR_SYS_NOT_EMPTY },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "read_error_fails_fast_and_unwinds",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "T/B", .read = SP_ERR_SYS_IO },
    },
    .expect = {
      .err = SP_ERR_SYS_IO,
      .opens = 2,
    },
  },
};

sp_test_each(fs, remove_sim, test_t, tests, .serial = true) {
  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);
  sp_err_t err = sp_fs_remove_dir_at((sp_path_t) { .dir = sp_sys_get_root(0), .sub = sp_str_lit("T") });
  sim_end(&s);

  sp_expect_err_eq(t, err, it->expect.err);

  u32 expected = 0;
  sp_carr_for(it->expect.removed, e) {
    if (!it->expect.removed[e].path) break;
    expected++;
  }
  sp_expect_eq(t, s.num_removed, expected);
  sp_for(i, sp_min(s.num_removed, expected)) {
    sp_expect_str_eq_c(t, sp_cstr_as_str(s.removed[i].path), it->expect.removed[i].path);
    sp_expect_eq(t, (u32)s.removed[i].op, (u32)it->expect.removed[i].op);
  }

  sp_expect_eq(t, s.count.opens, it->expect.opens);
  sp_expect_eq(t, s.count.nofollow, s.count.opens);
  sp_expect_eq(t, s.count.rmdirs, it->expect.rmdirs);
  sp_expect_eq(t, s.count.closes, s.count.dirs);
  sp_expect_eq(t, s.count.fd_closes, (u32)0);
  return SP_OK;
}
