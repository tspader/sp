#include "fs.h"

UTEST(fs_get_cwd, contract) {
  SKIP_ON_WASM()
  sp_mem_t a = sp_mem_os_new();
  sp_str_t cwd = sp_fs_get_cwd(a);
  ASSERT_TRUE(sp_fs_is_dir(cwd));
  ASSERT_TRUE(sp_fs_is_absolute(cwd));
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
