#include "sp.h"
#include "sp/sp_test.h"

sp_test(fs, get_exe_path_basic_properties) {
  sp_test_skip_on_wasm()

  sp_str_t exe = sp_zero;
  sp_must_ok(t, sp_fs_get_exe_path(sp_test_arena(t), &exe));
  sp_must_gt(t, exe.len, 0);

  // normalized: no backslashes, no trailing slash
  sp_must(t, !sp_str_contains(exe, sp_str_lit("\\")));
  sp_must_ne(t, exe.data[exe.len - 1], '/');

  // has a filename component
  sp_must_gt(t, sp_fs_get_name(exe).len, 0);
  return SP_OK;
}

sp_test(fs, get_exe_path_is_absolute) {
  sp_test_skip_on_wasm()

  sp_str_t exe = sp_zero;
  sp_must_ok(t, sp_fs_get_exe_path(sp_test_arena(t), &exe));
  // absolute: starts with / on POSIX, or X: on Windows
  sp_must(t, (exe.data[0] == '/') || (exe.len >= 2 && exe.data[1] == ':'));
  return SP_OK;
}

sp_test(fs, get_exe_path_exists_on_disk) {
  sp_test_skip_on_wasm()

  sp_str_t exe = sp_zero;
  sp_must_ok(t, sp_fs_get_exe_path(sp_test_arena(t), &exe));
  sp_must(t, sp_fs_exists(exe));
  return SP_OK;
}

sp_test(fs, get_exe_path_is_canonical) {
  sp_test_skip_on_wasm()

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t exe = sp_zero;
  sp_str_t canonical = sp_zero;
  sp_must_ok(t, sp_fs_get_exe_path(mem, &exe));
  sp_must_ok(t, sp_fs_canonicalize_path(mem, exe, &canonical));
  sp_expect_str_eq(t, canonical, exe);
  return SP_OK;
}

sp_test(fs, get_exe_path_no_dotdot) {
  sp_test_skip_on_wasm()

  sp_str_t exe = sp_zero;
  sp_must_ok(t, sp_fs_get_exe_path(sp_test_arena(t), &exe));
  sp_must(t, !sp_str_contains(exe, sp_str_lit("..")));
  return SP_OK;
}
