#define SP_MSVC_IMPLEMENTATION
#include "msvc.h"

#if defined(SP_WIN32)

sp_test(msvc, find) {
  sp_msvc_t msvc = sp_zero;
  sp_msvc_err_t err = sp_msvc_find(&msvc);
  if (err) return sp_test_skip(t, "no msvc on this machine");

  c8 buf [SP_MSVC_PATH_MAX] = sp_zero;

  sp_must_gt(t, msvc.num_sdks, 0);
  sp_for(it, msvc.num_sdks) {
    sp_msvc_sdk_t* sdk = &msvc.sdks[it];
    sp_expect_eq(t, sdk->version.major, 10u);
    sp_expect(t, sp_fs_is_absolute(sp_msvc_path_str(&sdk->root)));

    sp_str_r lib_ucrt = sp_msvc_sdk_path_buf(buf, sizeof(buf), sdk, SP_MSVC_SDK_PATH_LIB_UCRT);
    sp_must_eq(t, lib_ucrt.err, SP_OK);
    sp_expect(t, sp_fs_is_dir(lib_ucrt.value));
    if (it) sp_expect(t, !sp_msvc_version_gt(sdk->version, msvc.sdks[it - 1].version));
  }

  sp_must_gt(t, msvc.num_installations, 0);
  sp_for(it, msvc.num_installations) {
    sp_msvc_vs_t* vs = &msvc.installations[it];
    sp_expect(t, sp_fs_is_absolute(sp_msvc_path_str(&vs->install_path)));

    sp_io_mem_writer_t io = sp_zero;
    sp_io_mem_writer_from_buffer(&io, buf, sizeof(buf));
    sp_must_eq(t, sp_msvc_vs_path_io(&io.base, vs, SP_MSVC_VS_PATH_LIB), SP_OK);
    sp_must_eq(t, sp_io_write_str(&io.base, sp_str_lit("/vcruntime.lib"), SP_NULLPTR), SP_OK);
    sp_expect(t, sp_fs_exists(sp_io_mem_writer_as_str(&io)));
    if (it) sp_expect(t, !sp_msvc_version_gt(vs->version.build, msvc.installations[it - 1].version.build));
  }

  return SP_OK;
}

#else

sp_test(msvc, find_unsupported) {
  sp_msvc_t msvc = sp_zero;
  sp_expect_eq(t, sp_msvc_find(&msvc), SP_MSVC_ERR_UNSUPPORTED);
  sp_expect_eq(t, msvc.num_sdks, 0u);
  sp_expect_eq(t, msvc.num_installations, 0u);
  return SP_OK;
}

#endif
