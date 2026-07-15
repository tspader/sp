#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_symlink)

UTEST_F(sys_symlink, honors_dirfd) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_symlink_honors_dirfd",
    .steps = {
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "target", .alias = "lnk" } },
      { .kind = SYS_STEP_LSTAT, .lstat = { .path = "lnk" } },
    },
  });
}

UTEST_F(sys_symlink, resolves_target_relative_to_link) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_symlink_resolves_target_relative_to_link",
    .setup = {
      { .path = "target.bin", .content = "hello" },
    },
    .steps = {
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "target.bin", .alias = "lnk" } },
      { .kind = SYS_STEP_OPEN, .open = { .path = "lnk", .flags = SP_O_RDONLY | SP_O_BINARY } },
      { .kind = SYS_STEP_READ, .read = { .count = 8, .expect = "hello" } },
    },
  });
}
