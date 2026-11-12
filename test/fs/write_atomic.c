#include "fs.h"

#define A16 "AAAAAAAAAAAAAAAA"
#define A255 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 "AAAAAAAAAAAAAAA"

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
  {
    .name = "name_at_component_limit",
    .path = A255,
    .content = "A",
    .expect = {
      .paths = {
        { .path = A255, .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
};

sp_test_each(fs, write_atomic, test_t, tests) {
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_path_t path = sp_path_join(sp_test_arena(t), sandbox, sp_cstr_as_str(it->path));
  sp_expect_err_eq(t, sp_fs_write_atomic_at(path, sp_cstr_as_str(it->content)), it->expect.err);
  fs_expect_paths(t, sandbox, it->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
