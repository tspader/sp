#include "fs.h"

typedef enum {
  OP_PATH_AT,
  OP_LINK_AT,
} op_t;

typedef struct {
  sp_fs_kind_t kind;
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  op_t op;
  const c8* path;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "path_at_follows_symlink",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .path = "L",
    .expect = { .kind = SP_FS_KIND_FILE },
  },
  {
    .name = "link_at_reports_symlink",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .op = OP_LINK_AT,
    .path = "L",
    .expect = { .kind = SP_FS_KIND_SYMLINK },
  },
};

sp_test_each(fs, metadata, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));
  sp_path_t at = sp_path(dir, sp_cstr_as_str(it->path));

  sp_sys_file_meta_t meta = sp_zero;
  sp_err_t err = SP_OK;
  switch (it->op) {
    case OP_PATH_AT: err = sp_fs_get_path_metadata_at(at, &meta); break;
    case OP_LINK_AT: err = sp_fs_get_link_metadata_at(at, &meta); break;
  }
  sp_fs_close_dir(dir);

  sp_expect_ok(t, err);
  sp_expect_eq(t, meta.kind, it->expect.kind);
  return SP_OK;
}
