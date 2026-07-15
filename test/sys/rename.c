#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_rename)

UTEST_F(sys_rename, replaces_existing_target) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rename_replaces_existing_target",
    .setup = {
      { .path = "src.bin", .content = "new!" },
      { .path = "dst.bin", .content = "old!" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "src.bin", .to = "dst.bin" } },
    },
    .expect = {
      { .path = "src.bin" },
      { .path = "dst.bin", .exists = true, .content = "new!" },
    },
  });
}

UTEST_F(sys_rename, replaces_readonly_target) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rename_replaces_readonly_target",
    .setup = {
      { .path = "src.bin", .content = "new!" },
      { .path = "dst.bin", .content = "old!" },
      { .path = "dst.bin", .kind = SYS_SETUP_READONLY },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "src.bin", .to = "dst.bin" } },
    },
    .expect = {
      { .path = "src.bin" },
      { .path = "dst.bin", .exists = true, .content = "new!" },
    },
  });
}
