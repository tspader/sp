#include "msvc.h"

typedef struct {
  const c8* lib_um;
  const c8* lib_ucrt;
  const c8* include_ucrt;
  const c8* include_um;
  const c8* include_shared;
} expect_t;

typedef struct {
  const c8* name;
  sp_msvc_arch_t arch;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "x64",
    .expect = {
      .lib_um         = "C:/K/Lib/1.2.3.4/um/x64",
      .lib_ucrt       = "C:/K/Lib/1.2.3.4/ucrt/x64",
      .include_ucrt   = "C:/K/Include/1.2.3.4/ucrt",
      .include_um     = "C:/K/Include/1.2.3.4/um",
      .include_shared = "C:/K/Include/1.2.3.4/shared",
    },
  },
  {
    .name = "arm64",
    .arch = SP_MSVC_ARCH_ARM64,
    .expect = {
      .lib_um         = "C:/K/Lib/1.2.3.4/um/arm64",
      .lib_ucrt       = "C:/K/Lib/1.2.3.4/ucrt/arm64",
      .include_ucrt   = "C:/K/Include/1.2.3.4/ucrt",
      .include_um     = "C:/K/Include/1.2.3.4/um",
      .include_shared = "C:/K/Include/1.2.3.4/shared",
    },
  },
};

sp_test_each(msvc, sdk_new, test_t, tests) {
  sp_msvc_sdk_t sdk = sp_msvc_sdk_new(it->arch, sp_str_lit("C:/K"), sp_str_lit("1.2.3.4"));
  sp_expect_str_eq_c(t, sp_msvc_path_str(&sdk.root), "C:/K");
  sp_expect_str_eq_c(t, sp_msvc_version_str(&sdk.version), "1.2.3.4");
  sp_expect_eq(t, sdk.arch, it->arch);

  sp_msvc_sdk_paths_t paths = sp_msvc_sdk_render(sp_test_arena(t), &sdk);
  sp_expect_str_eq_c(t, paths.lib_um, it->expect.lib_um);
  sp_expect_str_eq_c(t, paths.lib_ucrt, it->expect.lib_ucrt);
  sp_expect_str_eq_c(t, paths.include_ucrt, it->expect.include_ucrt);
  sp_expect_str_eq_c(t, paths.include_um, it->expect.include_um);
  sp_expect_str_eq_c(t, paths.include_shared, it->expect.include_shared);

  c8 buf [SP_MSVC_PATH_MAX] = sp_zero;
  sp_expect_str_eq_c(t, sp_msvc_sdk_path_buf(buf, sizeof(buf), &sdk, SP_MSVC_SDK_PATH_LIB_UM).value, it->expect.lib_um);

  c8 small [8] = sp_zero;
  sp_expect_eq(t, sp_msvc_sdk_path_buf(small, sizeof(small), &sdk, SP_MSVC_SDK_PATH_LIB_UM).err, SP_ERR_IO_NO_SPACE);
  return SP_OK;
}
