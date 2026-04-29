#include "fs.h"

static void assert_normalized(s32* utest_result, sp_str_t path, const c8* label) {
  sp_for(i, path.len) {
    if (path.data[i] == '\\') {
      SP_TEST_REPORT("{}: contains backslash", sp_fmt_cstr(label));
      SP_FAIL();
      return;
    }
  }
  if (path.len > 0 && path.data[path.len - 1] == '/') {
    SP_TEST_REPORT("{}: trailing slash", sp_fmt_cstr(label));
    SP_FAIL();
  }
}

UTEST(fs_system_paths, nonempty) {
  sp_mem_t a = sp_mem_os_new();
  ASSERT_GT(sp_fs_get_storage_path_a(a).len, 0);
  ASSERT_GT(sp_fs_get_config_path_a(a).len, 0);
}

UTEST(fs_system_paths, storage_path_normalized) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t path = sp_fs_get_storage_path_a(a);
  assert_normalized(&ur, path, "storage_path");
}

UTEST(fs_system_paths, config_path_normalized) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t path = sp_fs_get_config_path_a(a);
  assert_normalized(&ur, path, "config_path");
}

UTEST(fs_system_paths, storage_path_exists) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t path = sp_fs_get_storage_path_a(a);
  ASSERT_TRUE(sp_fs_is_dir_a(path));
}

UTEST(fs_system_paths, config_path_exists) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t path = sp_fs_get_config_path_a(a);
  ASSERT_TRUE(sp_fs_is_dir_a(path));
}

SP_TEST_MAIN()
