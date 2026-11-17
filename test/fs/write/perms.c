#include "fs.h"

typedef enum {
  OP_WRITE,
  OP_APPEND,
} op_t;

typedef enum {
  EXISTING_NONE,
  EXISTING_WRITABLE,
  EXISTING_READ_ONLY,
} existing_t;

typedef struct {
  bool read_only;
} expect_t;

typedef struct {
  const c8* name;
  existing_t existing;
  op_t op;
  sp_fs_write_mode_t mode;
  bool read_only;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "truncate_creates", .read_only = true, .expect = { .read_only = true } },
  { .name = "new_creates", .mode = SP_FS_WRITE_NEW, .read_only = true, .expect = { .read_only = true } },
  { .name = "replace_creates", .mode = SP_FS_WRITE_REPLACE, .read_only = true, .expect = { .read_only = true } },
  { .name = "exclusive_creates", .mode = SP_FS_WRITE_EXCLUSIVE, .read_only = true, .expect = { .read_only = true } },
  { .name = "append_creates", .op = OP_APPEND, .read_only = true, .expect = { .read_only = true } },
  { .name = "truncate_keeps_existing", .existing = EXISTING_WRITABLE, .read_only = true },
  { .name = "append_keeps_existing", .existing = EXISTING_WRITABLE, .op = OP_APPEND, .read_only = true },
  { .name = "replace_does_not_keep_existing", .existing = EXISTING_READ_ONLY, .mode = SP_FS_WRITE_REPLACE },
};

sp_test_each(fs, write_perms, test_t, tests) {
  sp_test_skip_on_wasm();
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t path = sp_path_join(mem, sandbox, sp_str_lit("A"));

  sp_sys_file_perms_t read_only = sp_sys_default_file_perms;
  sp_sys_set_read_only(&read_only, true);
  switch (it->existing) {
    case EXISTING_NONE:      break;
    case EXISTING_WRITABLE:  sp_must_ok(t, sp_fs_write_at(path, sp_str_lit(""), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms)); break;
    case EXISTING_READ_ONLY: sp_must_ok(t, sp_fs_write_at(path, sp_str_lit(""), SP_FS_WRITE_TRUNCATE, read_only)); break;
  }

  sp_sys_file_perms_t perms = it->read_only ? read_only : sp_sys_default_file_perms;
  switch (it->op) {
    case OP_WRITE:  sp_expect_ok(t, sp_fs_write_at(path, sp_str_lit("B"), it->mode, perms)); break;
    case OP_APPEND: sp_expect_ok(t, sp_fs_append_at(path, sp_str_lit("B"), perms)); break;
  }

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_fs_get_path_metadata_at(path, &meta));
  sp_expect_eq(t, sp_sys_is_read_only(meta.perms), it->expect.read_only);

  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read_at(mem, path, &content));
  sp_expect_str_eq(t, content, sp_str_lit("B"));
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
