#include "sp.h"
#include "sp/sp_test.h"

sp_test(fs, get_cwd_contract) {
  sp_test_skip_on_wasm()

  sp_str_t cwd = sp_fs_get_cwd_path(sp_test_arena(t));
  sp_must(t, sp_fs_is_dir(cwd));
  sp_must(t, sp_fs_is_absolute(cwd));
  return SP_OK;
}

sp_test(fs, get_cwd_unlinked_cwd_does_not_leak_deleted_suffix, .serial = true) {
  // @spader
#if !defined(SP_LINUX)
  return sp_test_skip(t, "unlinked-cwd is a Linux-only scenario");
#else
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t original = sp_fs_get_cwd_path(mem);
  sp_must_gt(t, original.len, 0);

  sp_str_t sandbox = sp_test_dir(t);
  sp_must_ok(t, sp_sys_chdir_s(sandbox));
  sp_must_ok(t, sp_fs_remove_dir(sandbox));

  sp_str_t cwd = sp_fs_get_cwd_path(mem);

  // restore cwd before any assertion can fail, so other tests aren't poisoned
  sp_sys_chdir_s(original);

  sp_must(t, !sp_str_contains(cwd, sp_str_lit(" (deleted)")));
  return SP_OK;
#endif
}

sp_test(fs, get_cwd_is_root_zero) {
  sp_test_skip_on_wasm()
  sp_expect_eq(t, sp_fs_get_cwd(), sp_sys_get_root(0));
  return SP_OK;
}

sp_test(fs, get_cwd_override, .serial = true) {
  sp_test_skip_on_wasm()

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sp_test_dir(t), 0, &dir));
  sp_must_ok(t, sp_fs_create_file_at((sp_path_t) { .dir = dir, .sub = sp_str_lit("A") }));

  sp_rt.cwd = dir;
  sp_sys_fd_t cwd = sp_fs_get_cwd();
  sp_path_t path = sp_path_at_cwd(sp_str_lit("A"));
  bool found = sp_fs_is_file(sp_str_lit("A"));
  sp_str_t cwd_path = sp_fs_get_cwd_path(sp_test_arena(t));
  sp_rt.cwd = SP_SYS_INVALID_FD;
  sp_sys_fd_t restored = sp_fs_get_cwd();
  sp_sys_close(dir);

  sp_expect_eq(t, cwd, dir);
  sp_expect_eq(t, path.dir, dir);
  sp_expect_str_eq(t, path.sub, sp_str_lit("A"));
  sp_expect(t, found);
  sp_expect_str_eq(t, cwd_path, sp_fs_canonicalize_path(sp_test_arena(t), sp_test_dir(t)));
  sp_expect_eq(t, restored, sp_sys_get_root(0));
  return SP_OK;
}

sp_test(fs, path_at_cwd_passes_through) {
  sp_test_skip_on_wasm()

  sp_str_t file = sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_create_file(file));

  sp_path_t path = sp_path_at_cwd(file);
  sp_expect_eq(t, path.dir, sp_fs_get_cwd());
  sp_expect_str_eq(t, path.sub, file);
  sp_expect(t, sp_fs_is_file_at(path));
  sp_expect(t, sp_fs_is_dir_at(sp_path_at_cwd(sp_str_lit("."))));
  return SP_OK;
}
