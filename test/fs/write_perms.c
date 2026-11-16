#include "fs.h"

typedef struct {
  bool read_only;
} expect_t;

typedef struct {
  const c8* name;
  bool existing_read_only;
  sp_fs_write_mode_t mode;
  bool read_only;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "truncate_new", .read_only = true, .expect = { .read_only = true } },
  { .name = "append_new", .mode = SP_FS_WRITE_APPEND, .read_only = true, .expect = { .read_only = true } },
  { .name = "replace_new", .mode = SP_FS_WRITE_REPLACE, .read_only = true, .expect = { .read_only = true } },
  { .name = "exclusive_new", .mode = SP_FS_WRITE_EXCLUSIVE, .read_only = true, .expect = { .read_only = true } },
  { .name = "replace_does_not_keep_existing", .existing_read_only = true, .mode = SP_FS_WRITE_REPLACE },
};

sp_test_each(fs, write_perms, test_t, tests) {
  sp_test_skip_on_wasm();
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t path = sp_path_join(mem, sandbox, sp_str_lit("A"));

  sp_sys_file_perms_t read_only = sp_sys_default_file_perms;
  sp_sys_set_read_only(&read_only, true);
  if (it->existing_read_only) {
    sp_must_ok(t, sp_fs_write_at(path, sp_str_lit("A"), SP_FS_WRITE_EXCLUSIVE, read_only));
  }

  sp_expect_ok(t, sp_fs_write_at(path, sp_str_lit("B"), it->mode, it->read_only ? read_only : sp_sys_default_file_perms));

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_fs_get_path_metadata_at(path, &meta));
  sp_expect_eq(t, sp_sys_is_read_only(meta.perms), it->expect.read_only);

  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read_at(mem, path, &content));
  sp_expect_str_eq(t, content, sp_str_lit("B"));
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
