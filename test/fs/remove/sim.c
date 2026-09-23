#include "fs.h"
#include "sim.h"

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} removed_t;

typedef struct {
  sp_err_t err;
  removed_t removed [SIM_MAX_REMOVED];
  u32 opens;
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
      .removed = { { "T/C", SP_FS_KIND_FILE }, { "T", SP_FS_KIND_DIR } },
      .opens = 1,
    },
  },
  {
    .name = "root_vanished_after_walk_is_ok",
    .dirs = {
      { .path = "T", .rmdir = SP_ERR_SYS_NOT_FOUND, .entries = { { "B", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "T/B", SP_FS_KIND_FILE } },
      .opens = 1,
    },
  },
  {
    .name = "file_hint_that_is_dir_is_walked",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_FILE, .unlink = SP_ERR_SYS_IS_DIR } } },
      { .path = "T/B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .removed = { { "T/B/C", SP_FS_KIND_FILE }, { "T/B", SP_FS_KIND_DIR }, { "T", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "dir_hint_that_is_not_dir_is_unlinked",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B", .open = SP_ERR_SYS_NOT_DIR },
    },
    .expect = {
      .removed = { { "T/B", SP_FS_KIND_FILE }, { "T", SP_FS_KIND_DIR } },
      .opens = 2,
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
    },
  },
  {
    .name = "read_error_unwinds",
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

  fs_match_t matches [SIM_MAX_REMOVED] = sp_zero;
  u32 n = 0;
  sp_carr_for(it->expect.removed, i) {
    const removed_t* want = &it->expect.removed[i];
    if (!want->path) break;
    matches[n++] = (fs_match_t) { .key = sp_cstr_as_str(want->path), .kind = want->kind };
  }
  sp_for(i, s.num_removed) {
    fs_match(t, matches, n, sp_cstr_as_str(s.removed[i].path), s.removed[i].kind);
  }
  fs_match_finish(t, matches, n);

  sp_expect_eq(t, s.count.opens, it->expect.opens);
  sp_expect_eq(t, s.count.closes, s.count.dirs);
  return SP_OK;
}
