#include "fs.h"

struct fs_get_cwd {
  sp_mem_heap_t* heap;
  sp_mem_t mem;
};

UTEST_F_SETUP(fs_get_cwd) {
  ut.heap = sp_mem_heap_new();
  ut.mem = sp_mem_heap_as_allocator(ut.heap);
}

UTEST_F_TEARDOWN(fs_get_cwd) {
  sp_mem_heap_destroy(ut.heap);
}

UTEST_F(fs_get_cwd, contract) {
  SKIP_ON_WASM()
  sp_str_t cwd = sp_fs_get_cwd(ut.mem);
  ASSERT_TRUE(sp_fs_is_dir(cwd));
  ASSERT_TRUE(sp_fs_is_absolute(cwd));
}

UTEST_F(fs_get_cwd, unlinked_cwd_does_not_leak_deleted_suffix) {
  // @spader
#if !defined(SP_LINUX)
  UTEST_SKIP("unlinked-cwd is a Linux-only scenario");
#else
  sp_str_t original = sp_fs_get_cwd(ut.mem);
  ASSERT_GT(original.len, 0);

  sp_str_t sandbox = sp_fs_join_path(ut.mem, original, sp_str_lit(".tmp/unlinked_cwd_test"));
  sp_fs_remove_dir(sandbox);
  ASSERT_EQ(sp_fs_create_dir(sandbox), SP_OK);

  ASSERT_EQ(sp_sys_chdir_s(sandbox), 0);
  ASSERT_EQ(sp_fs_remove_dir(sandbox), SP_OK);

  sp_str_t cwd = sp_fs_get_cwd(ut.mem);

  // restore cwd before any assertion can fail, so other tests aren't poisoned
  sp_sys_chdir_s(original);

  ASSERT_FALSE(sp_str_contains(cwd, sp_str_lit(" (deleted)")));
#endif
}
