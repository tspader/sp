#define SP_MSVC_IMPLEMENTATION
#include "msvc.h"

#if defined(SP_WIN32)

sp_test(msvc, find) {
  sp_msvc_t msvc = sp_zero;
  sp_msvc_err_t err = sp_msvc_find(sp_test_arena(t), SP_MSVC_ARCH_X64, &msvc);
  if (err) return sp_test_skip(t, "no msvc on this machine");

  sp_must_gt(t, sp_da_size(msvc.sdks), 0);
  sp_da_for(msvc.sdks, it) {
    sp_msvc_sdk_t* sdk = &msvc.sdks[it];
    sp_expect_eq(t, sdk->version.major, 10u);
    sp_expect(t, sp_fs_is_absolute(sdk->root));
    sp_expect(t, sp_fs_is_dir(sdk->lib_ucrt));
    if (it) sp_expect(t, !sp_msvc_version_gt(sdk->version, msvc.sdks[it - 1].version));
  }

  sp_must_gt(t, sp_da_size(msvc.installations), 0);
  sp_da_for(msvc.installations, it) {
    sp_msvc_vs_t* vs = &msvc.installations[it];
    sp_expect(t, sp_fs_is_absolute(vs->install_path));
    sp_expect(t, sp_fs_exists(sp_fs_join_path(msvc.mem, vs->lib, sp_str_lit("vcruntime.lib"))));
    if (it) sp_expect(t, !sp_msvc_version_gt(vs->version.build, msvc.installations[it - 1].version.build));
  }

  sp_msvc_free(&msvc);
  return SP_OK;
}

#else

sp_test(msvc, find_skipped_non_windows) {
  return sp_test_skip(t, "msvc discovery requires windows");
}

#endif
