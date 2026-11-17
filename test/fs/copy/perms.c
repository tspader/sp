#include "fs.h"

typedef struct {
  bool read_only;
} expect_t;

typedef struct {
  const c8* name;
  sp_fs_write_mode_t mode;
  bool existing;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "truncate_creates_with_source_perms", .expect = { .read_only = true } },
  { .name = "new_creates_with_source_perms", .mode = SP_FS_WRITE_NEW, .expect = { .read_only = true } },
  { .name = "replace_creates_with_source_perms", .mode = SP_FS_WRITE_REPLACE, .expect = { .read_only = true } },
  { .name = "exclusive_creates_with_source_perms", .mode = SP_FS_WRITE_EXCLUSIVE, .expect = { .read_only = true } },
  { .name = "truncate_keeps_existing", .existing = true },
  { .name = "replace_does_not_keep_existing", .mode = SP_FS_WRITE_REPLACE, .existing = true, .expect = { .read_only = true } },
};

sp_test_each(fs, copy_perms, test_t, tests) {
  sp_test_skip_on_wasm();
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));

  sp_sys_file_perms_t read_only = sp_sys_default_file_perms;
  sp_sys_set_read_only(&read_only, true);
  sp_must_ok(t, sp_fs_write_at(from, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, read_only));
  if (it->existing) {
    sp_must_ok(t, sp_fs_write_at(to, sp_str_lit("B"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  }

  sp_expect_ok(t, sp_fs_copy_file_at(from, to, it->mode));

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_fs_get_path_metadata_at(to, &meta));
  sp_expect_eq(t, sp_sys_is_read_only(meta.perms), it->expect.read_only);

  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read_at(mem, to, &content));
  sp_expect_str_eq(t, content, sp_str_lit("A"));
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
