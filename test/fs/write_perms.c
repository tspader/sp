#include "fs.h"

typedef struct {
  const c8* name;
  u32 perms;
} test_t;

static const test_t tests [] = {
#if defined(SP_WIN32)
  { .name = "read_only", .perms = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE },
#else
  { .name = "private", .perms = 0600 },
  { .name = "read_only", .perms = 0444 },
#endif
};

sp_test_each(fs, write_replace_keeps_perms, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t path = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_must_ok(t, sp_fs_write_at(path, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE));

  sp_err_t set = sp_sys_set_file_perms_s(path.dir, path.sub, (sp_sys_file_perms_t) { .value = it->perms });
  if (set == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "permissions not available");
  sp_must_ok(t, set);

  sp_sys_file_meta_t before = sp_zero;
  sp_must_ok(t, sp_fs_get_path_metadata_at(path, &before));

  sp_expect_ok(t, sp_fs_write_at(path, sp_str_lit("B"), SP_FS_WRITE_REPLACE));

  sp_sys_file_meta_t after = sp_zero;
  sp_must_ok(t, sp_fs_get_path_metadata_at(path, &after));
  sp_expect_eq(t, after.perms.value, before.perms.value);

  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read_at(mem, path, &content));
  sp_expect_str_eq(t, content, sp_str_lit("B"));
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
