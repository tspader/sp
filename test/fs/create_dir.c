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
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
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
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
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
  skip_if_symlinks_needed(t, it->setup);

  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t target = sp_fs_join_path(sp_test_arena(t), sandbox, sp_str_view(it->target));
  sp_expect_err_eq(t, sp_fs_create_dir(target), it->expect.err);

  fs_expect_paths(t, sandbox, it->expect.paths);
  return SP_OK;
}

sp_test(fs, create_dir_create_multi_level_relative, .serial = true) {
  sp_test_skip_on_wasm()

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);

  sp_str_t cwd = sp_fs_get_cwd(mem);
  sp_must_ok(t, sp_sys_chdir_s(sandbox));
  sp_err_t result = sp_fs_create_dir(sp_str_lit("A/B"));
  sp_must_ok(t, sp_sys_chdir_s(cwd));

  sp_expect_ok(t, result);
  sp_str_t created = sp_fs_join_path(mem, sandbox, sp_str_lit("A/B"));
  sp_expect(t, sp_fs_exists(created));
  sp_expect_eq(t, sp_fs_get_kind(created), SP_FS_KIND_DIR);
  return SP_OK;
}
