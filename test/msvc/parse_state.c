#include "msvc.h"

typedef struct {
  bool ok;
  const c8* install_path;
  const c8* build_version;
  const c8* product_line;
} expect_t;

typedef struct {
  const c8* name;
  const c8* json;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "full",
    .json = "{\"installationPath\":\"C:\\\\A\\\\B\",\"buildVersion\":\"17.14.3\",\"productLineVersion\":\"2022\"}",
    .expect = { .ok = true, .install_path = "C:/A/B", .build_version = "17.14.3", .product_line = "2022" },
  },
  {
    .name = "forward_slashes",
    .json = "{\"installationPath\":\"C:/A\",\"buildVersion\":\"17.0.0\",\"productLineVersion\":\"2022\"}",
    .expect = { .ok = true, .install_path = "C:/A", .build_version = "17.0.0", .product_line = "2022" },
  },
  {
    .name = "no_product_line",
    .json = "{\"installationPath\":\"C:\\\\A\",\"buildVersion\":\"17.0.0\"}",
    .expect = { .ok = true, .install_path = "C:/A", .build_version = "17.0.0", .product_line = "" },
  },
  {
    .name = "escaped_quote",
    .json = "{\"installationPath\":\"C:\\\\A\\\"B\",\"buildVersion\":\"17.0.0\"}",
    .expect = { .ok = true, .install_path = "C:/A\"B", .build_version = "17.0.0", .product_line = "" },
  },
  {
    .name = "no_install_path",
    .json = "{\"buildVersion\":\"17.0.0\"}",
  },
  {
    .name = "no_build_version",
    .json = "{\"installationPath\":\"C:\\\\A\"}",
  },
  {
    .name = "install_path_not_a_string",
    .json = "{\"installationPath\":3,\"buildVersion\":\"17.0.0\"}",
  },
  {
    .name = "truncated_value",
    .json = "{\"installationPath\":\"C:\\\\A\",\"buildVersion\":\"17.0",
    .expect = { .ok = true, .install_path = "C:/A", .build_version = "17.0", .product_line = "" },
  },
  {
    .name = "whitespace",
    .json = "{\n  \"installationPath\" : \"C:\\\\A\",\n  \"buildVersion\":\n\"17.0.0\"\n}",
    .expect = { .ok = true, .install_path = "C:/A", .build_version = "17.0.0", .product_line = "" },
  },
  {
    .name = "key_in_value",
    .json = "{\"A\":\"installationPath\",\"installationPath\":\"C:\\\\A\",\"buildVersion\":\"17.0.0\"}",
    .expect = { .ok = true, .install_path = "C:/A", .build_version = "17.0.0", .product_line = "" },
  },
  {
    .name = "empty",
    .json = "",
  },
};

sp_test_each(msvc, parse_state, test_t, tests) {
  sp_msvc_state_t state = sp_zero;
  bool ok = sp_msvc_parse_state(sp_test_arena(t), sp_cstr_as_str(it->json), &state);
  sp_must_eq(t, ok, it->expect.ok);
  if (!ok) return SP_OK;

  sp_expect_str_eq_c(t, state.install_path, it->expect.install_path);
  sp_expect_str_eq_c(t, state.build_version, it->expect.build_version);
  sp_expect_str_eq_c(t, state.product_line, it->expect.product_line);
  return SP_OK;
}
