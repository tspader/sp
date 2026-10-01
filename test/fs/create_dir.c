#include "fs.h"

typedef struct {
  sp_err_t err;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* target;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "existing_directory",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .target = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "create_one_level",
    .target = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "create_multi_level",
    .target = "A/B",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "destination_is_file",
    .setup = {
      { "A" },
    },
    .target = "A",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "destination_parent_is_file",
    .setup = {
      { "A" },
    },
    .target = "A/B",
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
        { .path = "A/B" },
      },
    },
  },
#if defined(SP_POSIX)
  {
    .name = "destination_is_symlink_to_directory",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .target = "L",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK },
      },
    },
  },
  {
    .name = "destination_under_symlink_to_directory",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .target = "L/B",
    .expect = {
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "destination_is_dangling_symlink",
    .setup = {
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .target = "L",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A" },
      },
    },
  },
  {
    .name = "destination_is_symlink_to_file",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .target = "L",
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
        { .path = "L", .exists = true, .kind = SP_FS_KIND_SYMLINK },
      },
    },
  },
#endif
};

sp_test_each(fs, create_dir, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_path_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_path_t target = sp_path_join(sp_test_arena(t), sandbox, sp_cstr_as_str(it->target));
  sp_expect_err_eq(t, sp_fs_create_dir_at(target), it->expect.err);

  fs_expect_paths(t, sandbox, it->expect.paths);
  return SP_OK;
}
