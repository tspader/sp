#include "fs.h"

static void assert_normalized(s32* utest_result, sp_str_t path, const c8* label) {
  sp_for(i, path.len) {
    if (path.data[i] == '\\') {
      SP_TEST_REPORT("{}: contains backslash", SP_FMT_CSTR(label));
      SP_FAIL();
      return;
    }
  }
  if (path.len > 0 && path.data[path.len - 1] == '/') {
    SP_TEST_REPORT("{}: trailing slash", SP_FMT_CSTR(label));
    SP_FAIL();
  }
}

UTEST(fs_system_paths, nonempty) {
  ASSERT_GT(sp_fs_get_storage_path().len, 0);
  ASSERT_GT(sp_fs_get_config_path().len, 0);
}

UTEST(fs_system_paths, storage_path_normalized) {
  sp_str_t path = sp_fs_get_storage_path();
  assert_normalized(&ur, path, "storage_path");
}

UTEST(fs_system_paths, config_path_normalized) {
  sp_str_t path = sp_fs_get_config_path();
  assert_normalized(&ur, path, "config_path");
}

UTEST(fs_system_paths, storage_path_exists) {
  sp_str_t path = sp_fs_get_storage_path();
  ASSERT_TRUE(sp_fs_is_dir(path));
}

UTEST(fs_system_paths, config_path_exists) {
  sp_str_t path = sp_fs_get_config_path();
  ASSERT_TRUE(sp_fs_is_dir(path));
}

SP_TEST_MAIN()
