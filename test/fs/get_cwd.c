#include "sp.h"
#include "sp/sp_test.h"

sp_test(fs, get_cwd_contract) {
  sp_test_skip_on_wasm()

  sp_str_t cwd = sp_fs_get_cwd(sp_test_arena(t));
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
  sp_str_t original = sp_fs_get_cwd(mem);
  sp_must_gt(t, original.len, 0);

  sp_str_t sandbox = sp_test_dir(t);
  sp_must_ok(t, sp_sys_chdir_s(sandbox));
  sp_must_ok(t, sp_fs_remove_dir(sandbox));

  sp_str_t cwd = sp_fs_get_cwd(mem);

  // restore cwd before any assertion can fail, so other tests aren't poisoned
  sp_sys_chdir_s(original);

  sp_must(t, !sp_str_contains(cwd, sp_str_lit(" (deleted)")));
  return SP_OK;
#endif
}
