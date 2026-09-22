#include "fs.h"

typedef struct {
  sp_err_t err;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* path;
  const c8* content;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "new",
    .path = "A",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "empty",
    .path = "A",
    .content = "",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
      },
    },
  },
  {
    .name = "missing_parent",
    .path = "D/A",
    .content = "A",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "D" },
      },
    },
  },
  {
    .name = "replace_existing",
    .setup = {
      { .path = "A", .content = "old" },
    },
    .path = "A",
    .content = "new",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "new" },
      },
    },
  },
};

sp_test_each(fs, write_atomic, test_t, tests) {
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &dir));

  sp_path_t path = { .dir = dir, .sub = sp_str_view(it->path) };
  sp_err_t err = sp_fs_write_atomic_at(path, sp_str_view(it->content));
  sp_sys_close(dir);

  sp_expect_err_eq(t, err, it->expect.err);
  fs_expect_paths(t, sandbox, it->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
