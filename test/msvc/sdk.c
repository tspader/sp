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
  sp_msvc_sdk_t sdk = sp_msvc_sdk_new(
    sp_test_arena(t), it->arch, sp_str_lit("C:/K"), sp_str_lit("1.2.3.4")
  );
  sp_expect_str_eq_c(t, sdk.root, "C:/K");
  sp_expect_str_eq_c(t, sdk.version.str, "1.2.3.4");
  sp_expect_str_eq_c(t, sdk.lib_um, it->expect.lib_um);
  sp_expect_str_eq_c(t, sdk.lib_ucrt, it->expect.lib_ucrt);
  sp_expect_str_eq_c(t, sdk.include_ucrt, it->expect.include_ucrt);
  sp_expect_str_eq_c(t, sdk.include_um, it->expect.include_um);
  sp_expect_str_eq_c(t, sdk.include_shared, it->expect.include_shared);
  return SP_OK;
}
