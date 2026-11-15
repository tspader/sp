#include "fs.h"

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* path;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "basic",
    .path = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "idempotent",
    .setup = {
      { "A" },
    },
    .path = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "unicode",
    .path = "\xc3\xb1\x61\x6d\x65.txt",
    .expect = {
      { .path = "\xc3\xb1\x61\x6d\x65.txt", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
};

sp_test_each(fs, create_file, test_t, tests) {
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_path_t path = sp_path_join(sp_test_arena(t), sandbox, sp_cstr_as_str(it->path));
  sp_expect_ok(t, sp_fs_create_file_at(path));

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}
