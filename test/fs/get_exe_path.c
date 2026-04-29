#include "fs.h"

UTEST(fs_get_exe_path, basic_properties) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t exe = sp_fs_get_exe_path_a(a);
  ASSERT_GT(exe.len, 0);

  // normalized: no backslashes
  sp_for(i, exe.len) {
    ASSERT_NE(exe.data[i], '\\');
  }

  // normalized: no trailing slash
  ASSERT_NE(exe.data[exe.len - 1], '/');

  // has a filename component
  sp_str_t name = sp_fs_get_name(exe);
  ASSERT_GT(name.len, 0);
}

UTEST(fs_get_exe_path, is_absolute) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t exe = sp_fs_get_exe_path_a(a);
  // absolute: starts with / on POSIX, or X: on Windows
  bool is_absolute = (exe.data[0] == '/') || (exe.len >= 2 && exe.data[1] == ':');
  ASSERT_TRUE(is_absolute);
}

UTEST(fs_get_exe_path, exists_on_disk) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t exe = sp_fs_get_exe_path_a(a);
  ASSERT_TRUE(sp_fs_exists_a(exe));
}

UTEST(fs_get_exe_path, is_canonical) {
  // canonicalizing the exe path should be a no-op
  sp_mem_t a = sp_mem_os_new();
  sp_str_t exe = sp_fs_get_exe_path_a(a);
  sp_str_t canonical = sp_fs_canonicalize_path_a(a, exe);
  SP_EXPECT_STR_EQ(canonical, exe);
}

UTEST(fs_get_exe_path, no_dotdot) {
  sp_mem_t a = sp_mem_os_new();
  sp_str_t exe = sp_fs_get_exe_path_a(a);
  ASSERT_FALSE(sp_str_contains(exe, SP_LIT("..")));
}

SP_TEST_MAIN()
