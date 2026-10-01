#include "sp.h"
#include "sp/sp_test.h"

sp_test(fs, get_cwd_contract) {
  sp_test_skip_on_wasm()

  sp_str_t cwd = sp_zero;
  sp_must_ok(t, sp_fs_get_cwd_path(sp_test_arena(t), &cwd));
  sp_must(t, sp_fs_is_dir(cwd));
  sp_must(t, sp_fs_is_absolute(cwd));
  return SP_OK;
}

sp_test(fs, get_cwd_unlinked_cwd_has_no_path, .serial = true) {
  // @spader
#if !defined(SP_LINUX)
  return sp_test_skip(t, "unlinked-cwd is a Linux-only scenario");
#else
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t original = sp_zero;
  sp_must_ok(t, sp_fs_get_cwd_path(mem, &original));

  sp_path_t sandbox = sp_test_dir(t);
  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_path_t doomed = sp_path(dir, sp_str_lit("A"));
  sp_str_t canonical = sp_zero;
  sp_err_t err = sp_fs_create_dir_at(doomed);
  if (!err) err = sp_fs_canonicalize_path_at(mem, doomed, &canonical);
  if (!err) err = sp_sys_chdir_s(canonical);
  if (!err) err = sp_fs_remove_dir_at(doomed);

  sp_str_t cwd = sp_zero;
  sp_err_t unlinked = sp_fs_get_cwd_path(mem, &cwd);

  // restore cwd before any assertion can fail, so other tests aren't poisoned
  sp_sys_chdir_s(original);
  sp_sys_close(dir);

  sp_must_ok(t, err);
  sp_expect_err_eq(t, unlinked, SP_ERR_SYS_NOT_FOUND);
  return SP_OK;
#endif
}

sp_test(fs, get_cwd_is_root_zero) {
  sp_test_skip_on_wasm()
  sp_expect_eq(t, sp_fs_get_cwd(), sp_sys_get_root(0));
  return SP_OK;
}

sp_test(fs, path_cwd_passes_through) {
  sp_test_skip_on_wasm()

  sp_path_t file = sp_path_join(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_create_file_at(file));

  sp_path_t path = sp_path_cwd(file.sub);
  sp_expect_eq(t, path.dir, sp_fs_get_cwd());
  sp_expect_str_eq(t, path.sub, file.sub);
  sp_expect(t, sp_fs_is_file_at(path));
  sp_expect(t, sp_fs_is_dir_at(sp_path_cwd(sp_str_lit("."))));
  return SP_OK;
}
