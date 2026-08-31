#include "msvc.h"

typedef struct {
  const c8* lib;
  const c8* include;
  const c8* bin;
} expect_t;

typedef struct {
  const c8* name;
  sp_msvc_arch_t host;
  sp_msvc_arch_t target;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "x64_to_x64",
    .expect = {
      .lib     = "C:/V/VC/Tools/MSVC/1.2.3/Lib/x64",
      .include = "C:/V/VC/Tools/MSVC/1.2.3/include",
      .bin     = "C:/V/VC/Tools/MSVC/1.2.3/bin/Hostx64/x64",
    },
  },
  {
    .name = "x64_to_arm64",
    .target = SP_MSVC_ARCH_ARM64,
    .expect = {
      .lib     = "C:/V/VC/Tools/MSVC/1.2.3/Lib/arm64",
      .include = "C:/V/VC/Tools/MSVC/1.2.3/include",
      .bin     = "C:/V/VC/Tools/MSVC/1.2.3/bin/Hostx64/arm64",
    },
  },
  {
    .name = "arm64_to_x64",
    .host = SP_MSVC_ARCH_ARM64,
    .expect = {
      .lib     = "C:/V/VC/Tools/MSVC/1.2.3/Lib/x64",
      .include = "C:/V/VC/Tools/MSVC/1.2.3/include",
      .bin     = "C:/V/VC/Tools/MSVC/1.2.3/bin/Hostarm64/x64",
    },
  },
};

sp_test_each(msvc, vs_new, test_t, tests) {
  sp_msvc_state_t state = {
    .install_path = sp_str_lit("C:/V"),
    .build_version = sp_str_lit("17.14.6"),
    .product_line = sp_str_lit("2022"),
  };
  sp_msvc_vs_t vs = sp_msvc_vs_new(sp_test_arena(t), it->host, it->target, state, sp_str_lit("1.2.3"));

  sp_expect_str_eq_c(t, vs.install_path, "C:/V");
  sp_expect_str_eq_c(t, vs.version.product, "2022");
  sp_expect_str_eq_c(t, vs.version.build.str, "17.14.6");
  sp_expect_str_eq_c(t, vs.version.tools.str, "1.2.3");
  sp_expect_str_eq_c(t, vs.lib, it->expect.lib);
  sp_expect_str_eq_c(t, vs.include, it->expect.include);
  sp_expect_str_eq_c(t, vs.bin, it->expect.bin);
  return SP_OK;
}
