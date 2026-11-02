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
      { .entries = { { "B", SP_FS_KIND_FILE, .absent = true }, { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "C", SIM_OP_UNLINK }, { "", SIM_OP_RMDIR } },
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "dir_vanished_before_open_is_ok",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR, .absent = true } } },
    },
    .expect = {
      .removed = { { "", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "root_vanished_after_walk_is_ok",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_FOUND, .entries = { { "B", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "B", SIM_OP_UNLINK } },
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "root_vanished_before_open_is_not_found",
    .dirs = {
      { .open = SP_ERR_SYS_NOT_FOUND },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .opens = 1,
    },
  },
  {
    .name = "root_vanished_before_retry_is_ok",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_EMPTY, .reopen = SP_ERR_SYS_NOT_FOUND, .entries = { { "A", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "A", SIM_OP_UNLINK } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "file_hint_that_is_dir_is_walked",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_FILE, .unlink = SP_ERR_SYS_IS_DIR } } },
      { .path = "B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "B/C", SIM_OP_UNLINK }, { "B", SIM_OP_RMDIR }, { "", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 2,
    },
  },
  {
    .name = "dir_hint_that_is_not_dir_is_unlinked",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .open = SP_ERR_SYS_NOT_DIR },
    },
    .expect = {
      .removed = { { "B", SIM_OP_UNLINK }, { "", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "dir_hint_that_is_symlink_is_unlinked",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .open = SP_ERR_SYS_LOOP },
    },
    .expect = {
      .removed = { { "B", SIM_OP_UNLINK }, { "", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "not_empty_without_progress_propagates",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_EMPTY },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "vanished_entry_is_not_progress",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_EMPTY, .entries = { { "B", SP_FS_KIND_FILE, .absent = true } } },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "vanished_subdir_at_rmdir_is_not_progress",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_EMPTY, .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .rmdir = SP_ERR_SYS_NOT_FOUND, .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .removed = { { "B/C", SIM_OP_UNLINK } },
      .opens = 2,
      .rmdirs = 2,
    },
  },
  {
    .name = "parent_progress_survives_child_pass",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_EMPTY, .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .rmdir = SP_ERR_SYS_NOT_FOUND },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .removed = { { "A", SIM_OP_UNLINK } },
      .opens = 3,
      .rmdirs = 3,
    },
  },
  {
    .name = "not_empty_after_progress_retries_once",
    .dirs = {
      { .rmdir = SP_ERR_SYS_NOT_EMPTY, .entries = { { "A", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .removed = { { "A", SIM_OP_UNLINK } },
      .opens = 2,
      .rmdirs = 2,
    },
  },
  {
    .name = "subdir_not_empty_after_progress_retries_once",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .rmdir = SP_ERR_SYS_NOT_EMPTY, .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_EMPTY,
      .removed = { { "B/C", SIM_OP_UNLINK } },
      .opens = 3,
      .rmdirs = 2,
    },
  },
  {
    .name = "shifted_listing_is_reopened",
    .dirs = {
      { .batch = 1, .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "A", SIM_OP_UNLINK }, { "B", SIM_OP_UNLINK }, { "", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 2,
    },
  },
  {
    .name = "shifted_subdir_listing_is_reopened",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .batch = 1, .entries = { { "A", SP_FS_KIND_FILE }, { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "B/A", SIM_OP_UNLINK }, { "B/C", SIM_OP_UNLINK }, { "B", SIM_OP_RMDIR }, { "", SIM_OP_RMDIR } },
      .opens = 3,
      .rmdirs = 3,
    },
  },
  {
    .name = "removed_subdir_shifts_parent_listing",
    .dirs = {
      { .batch = 1, .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B" },
    },
    .expect = {
      .removed = { { "B", SIM_OP_RMDIR }, { "C", SIM_OP_UNLINK }, { "", SIM_OP_RMDIR } },
      .opens = 3,
      .rmdirs = 3,
    },
  },
  {
    .name = "unlink_error_propagates_without_retry",
    .dirs = {
      { .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_FILE, .unlink = SP_ERR_SYS_ACCESS_DENIED } } },
    },
    .expect = {
      .err = SP_ERR_SYS_ACCESS_DENIED,
      .removed = { { "A", SIM_OP_UNLINK } },
      .opens = 1,
    },
  },
  {
    .name = "rmdir_error_propagates_without_retry",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .rmdir = SP_ERR_SYS_ACCESS_DENIED, .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .err = SP_ERR_SYS_ACCESS_DENIED,
      .removed = { { "B/C", SIM_OP_UNLINK } },
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "unreadable_empty_subdir_is_removed",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .open = SP_ERR_SYS_ACCESS_DENIED },
    },
    .expect = {
      .removed = { { "B", SIM_OP_RMDIR }, { "", SIM_OP_RMDIR } },
      .opens = 2,
      .rmdirs = 2,
    },
  },
  {
    .name = "unreadable_empty_root_is_removed",
    .dirs = {
      { .open = SP_ERR_SYS_ACCESS_DENIED },
    },
    .expect = {
      .removed = { { "", SIM_OP_RMDIR } },
      .opens = 1,
      .rmdirs = 1,
    },
  },
  {
    .name = "unreadable_subdir_with_entries_is_access_denied",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .open = SP_ERR_SYS_ACCESS_DENIED, .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .err = SP_ERR_SYS_ACCESS_DENIED,
      .opens = 2,
      .rmdirs = 1,
    },
  },
  {
    .name = "read_error_fails_fast_and_unwinds",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .read = SP_ERR_SYS_IO },
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
  sp_err_t err = sp_fs_remove_dir_at(sim_root());
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
  sp_expect_eq(t, s.count.fd_closes, s.count.dirs);
  return SP_OK;
}
