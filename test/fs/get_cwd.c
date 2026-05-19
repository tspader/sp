#include "fs.h"

UTEST(fs_get_cwd, basic_properties) {
  SKIP_ON_WASM()
  sp_mem_t a = sp_mem_os_new();
  sp_str_t cwd = sp_fs_get_cwd(a);
  ASSERT_GT(cwd.len, 0);
  ASSERT_TRUE(sp_fs_is_dir(cwd));
  ASSERT_NE(cwd.data[cwd.len - 1], '/');
}

UTEST(fs_get_cwd, is_normalized) {
  SKIP_ON_WASM()
  sp_mem_t a = sp_mem_os_new();
  sp_str_t cwd = sp_fs_get_cwd(a);
  // no backslashes
  sp_for(i, cwd.len) {
    ASSERT_NE(cwd.data[i], '\\');
  }
}

UTEST(fs_get_cwd, is_absolute) {
  SKIP_ON_WASM()
  sp_mem_t a = sp_mem_os_new();
  sp_str_t cwd = sp_fs_get_cwd(a);
  bool is_absolute = (cwd.data[0] == '/') || (cwd.len >= 2 && cwd.data[1] == ':');
  ASSERT_TRUE(is_absolute);
}

UTEST(fs_get_cwd, unlinked_cwd_does_not_leak_deleted_suffix) {
  // @spader
#if !defined(SP_LINUX)
  UTEST_SKIP("unlinked-cwd is a Linux-only scenario");
#else
  sp_mem_t a = sp_mem_os_new();
  sp_str_t original = sp_fs_get_cwd(a);
  ASSERT_GT(original.len, 0);

  sp_str_t sandbox = sp_fs_join_path(a, original, sp_str_lit(".tmp/unlinked_cwd_test"));
  sp_fs_remove_dir(sandbox);
  ASSERT_EQ(sp_fs_create_dir(sandbox), SP_OK);

  ASSERT_EQ(sp_sys_chdir_s(sandbox), 0);
  ASSERT_EQ(sp_fs_remove_dir(sandbox), SP_OK);

  sp_str_t cwd = sp_fs_get_cwd(a);

  // restore cwd before any assertion can fail, so other tests aren't poisoned
  sp_sys_chdir_s(original);

  ASSERT_FALSE(sp_str_contains(cwd, sp_str_lit(" (deleted)")));
#endif
}
