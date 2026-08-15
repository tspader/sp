#include "sp.h"
#include "sp/sp_test.h"

static void expect_normalized(sp_test_t* t, sp_str_t path, const c8* label) {
  if (sp_str_contains(path, sp_str_lit("\\"))) {
    sp_test_fail(t, "{}: contains backslash", sp_fmt_cstr(label));
  }
  if (path.len > 0 && path.data[path.len - 1] == '/') {
    sp_test_fail(t, "{}: trailing slash", sp_fmt_cstr(label));
  }
}

sp_test(fs, system_paths_nonempty) {
  sp_test_skip_on_wasm()

  sp_mem_t mem = sp_test_arena(t);
  sp_must_gt(t, sp_fs_get_storage_path(mem).len, 0);
  sp_must_gt(t, sp_fs_get_config_path(mem).len, 0);
  return SP_OK;
}

sp_test(fs, system_paths_storage_path_normalized) {
  sp_test_skip_on_wasm()

  expect_normalized(t, sp_fs_get_storage_path(sp_test_arena(t)), "storage_path");
  return SP_OK;
}

sp_test(fs, system_paths_config_path_normalized) {
  sp_test_skip_on_wasm()

  expect_normalized(t, sp_fs_get_config_path(sp_test_arena(t)), "config_path");
  return SP_OK;
}
