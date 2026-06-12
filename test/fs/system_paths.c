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

struct fs_system_paths {
  sp_mem_heap_t* heap;
  sp_mem_t mem;
};

UTEST_F_SETUP(fs_system_paths) {
  ut.heap = sp_mem_heap_new();
  ut.mem = sp_mem_heap_as_allocator(ut.heap);
}

UTEST_F_TEARDOWN(fs_system_paths) {
  sp_mem_heap_destroy(ut.heap);
}

UTEST_F(fs_system_paths, nonempty) {
  SKIP_ON_WASM()
  ASSERT_GT(sp_fs_get_storage_path(ut.mem).len, 0);
  ASSERT_GT(sp_fs_get_config_path(ut.mem).len, 0);
}

UTEST_F(fs_system_paths, storage_path_normalized) {
  SKIP_ON_WASM()
  sp_str_t path = sp_fs_get_storage_path(ut.mem);
  assert_normalized(&ur, path, "storage_path");
}

UTEST_F(fs_system_paths, config_path_normalized) {
  SKIP_ON_WASM()
  sp_str_t path = sp_fs_get_config_path(ut.mem);
  assert_normalized(&ur, path, "config_path");
}
