#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_link)

UTEST_F(sys_link, honors_dirfds) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_link_honors_dirfds",
    .setup = {
      { .path = "a", .kind = SYS_SETUP_DIR },
      { .path = "b", .kind = SYS_SETUP_DIR },
      { .path = "a/src.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .slot = 0, .path = "a" } },
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .slot = 1, .path = "b" } },
      { .kind = SYS_STEP_LINK, .link = { .existing = "src.bin", .existing_dir = SYS_DIR_SLOT_0, .alias = "dst.bin", .alias_dir = SYS_DIR_SLOT_1 } },
    },
    .expect = {
      { .path = "a/src.bin", .exists = true, .content = "A" },
      { .path = "b/dst.bin", .exists = true, .content = "A" },
    },
  });
}
