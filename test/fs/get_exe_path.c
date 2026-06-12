#include "fs.h"

struct fs_get_exe_path {
  sp_mem_heap_t* heap;
  sp_mem_t mem;
};

UTEST_F_SETUP(fs_get_exe_path) {
  ut.heap = sp_mem_heap_new();
  ut.mem = sp_mem_heap_as_allocator(ut.heap);
}

UTEST_F_TEARDOWN(fs_get_exe_path) {
  sp_mem_heap_destroy(ut.heap);
}

UTEST_F(fs_get_exe_path, basic_properties) {
  SKIP_ON_WASM()
  sp_str_t exe = sp_fs_get_exe_path(ut.mem);
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

UTEST_F(fs_get_exe_path, is_absolute) {
  SKIP_ON_WASM()
  sp_str_t exe = sp_fs_get_exe_path(ut.mem);
  // absolute: starts with / on POSIX, or X: on Windows
  bool is_absolute = (exe.data[0] == '/') || (exe.len >= 2 && exe.data[1] == ':');
  ASSERT_TRUE(is_absolute);
}

UTEST_F(fs_get_exe_path, exists_on_disk) {
  SKIP_ON_WASM()
  sp_str_t exe = sp_fs_get_exe_path(ut.mem);
  ASSERT_TRUE(sp_fs_exists(exe));
}

UTEST_F(fs_get_exe_path, is_canonical) {
  SKIP_ON_WASM()
  sp_str_t exe = sp_fs_get_exe_path(ut.mem);
  sp_str_t canonical = sp_fs_canonicalize_path(ut.mem, exe);
  SP_EXPECT_STR_EQ(canonical, exe);
}

UTEST_F(fs_get_exe_path, no_dotdot) {
  SKIP_ON_WASM()
  sp_str_t exe = sp_fs_get_exe_path(ut.mem);
  ASSERT_FALSE(sp_str_contains(exe, sp_str_lit("..")));
}
