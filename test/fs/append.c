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
    .name = "existing",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .path = "A",
    .content = "B",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "AB" },
      },
    },
  },
  {
    .name = "through_symlink",
    .setup = {
      { .path = "T", .content = "T" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .path = "L",
    .content = "A",
    .expect = {
      .paths = {
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "T" },
        { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "TA" },
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
    .name = "onto_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .path = "A",
    .content = "A",
    .expect = {
      .err = SP_ERR_SYS_IS_DIR,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
};

sp_test_each(fs, append, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_err_t err = sp_fs_append_at(sp_path(dir, sp_cstr_as_str(it->path)), sp_cstr_as_str(it->content), sp_sys_default_file_perms);
  sp_fs_close_dir(dir);
  sp_expect_err_eq(t, err, it->expect.err);

  fs_expect_paths(t, sandbox, it->expect.paths);
  return SP_OK;
}
