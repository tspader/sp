#include "fs.h"

UTEST(fs_get_cwd, basic_properties) {
  sp_str_t cwd = sp_fs_get_cwd();
  ASSERT_GT(cwd.len, 0);
  ASSERT_TRUE(sp_fs_is_dir(cwd));
  ASSERT_NE(cwd.data[cwd.len - 1], '/');
}

UTEST(fs_get_cwd, is_normalized) {
  sp_str_t cwd = sp_fs_get_cwd();
  // no backslashes
  sp_for(i, cwd.len) {
    ASSERT_NE(cwd.data[i], '\\');
  }
}

UTEST(fs_get_cwd, is_absolute) {
  sp_str_t cwd = sp_fs_get_cwd();
  bool is_absolute = (cwd.data[0] == '/') || (cwd.len >= 2 && cwd.data[1] == ':');
  ASSERT_TRUE(is_absolute);
}

SP_TEST_MAIN()
