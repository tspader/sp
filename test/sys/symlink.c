#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_symlink)

UTEST_F(sys_symlink, honors_dirfd) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_symlink_honors_dirfd",
    .steps = {
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "target", .alias = "lnk", .kind = SP_FS_KIND_FILE } },
      { .kind = SYS_STEP_LSTAT, .lstat = { .path = "lnk" } },
    },
  });
}

UTEST_F(sys_symlink, resolves_target_relative_to_link) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_symlink_resolves_target_relative_to_link",
    .setup = {
      { .path = "target.bin", .content = "hello" },
    },
    .steps = {
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "target.bin", .alias = "lnk", .kind = SP_FS_KIND_FILE } },
      { .kind = SYS_STEP_OPEN, .open = { .path = "lnk" } },
      { .kind = SYS_STEP_READ, .read = { .count = 8, .expect = "hello" } },
    },
  });
}

UTEST_F(sys_symlink, dir_kind_resolves_to_dir) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_symlink_dir_kind_resolves_to_dir",
    .setup = {
      { .path = "D", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "D", .alias = "L", .kind = SP_FS_KIND_DIR } },
      { .kind = SYS_STEP_STAT, .stat = { .path = "L", .kind = SP_FS_KIND_DIR } },
    },
  });
}

UTEST_F(sys_symlink, refuses_untyped_link) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_symlink_refuses_untyped_link",
    .steps = {
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "T", .alias = "L", .err = SP_ERR_SYS_INVALID } },
      { .kind = SYS_STEP_SYMLINK, .symlink = { .target = "T", .alias = "L", .kind = SP_FS_KIND_SYMLINK, .err = SP_ERR_SYS_INVALID } },
      { .kind = SYS_STEP_LSTAT, .lstat = { .path = "L", .err = SP_ERR_SYS_NOT_FOUND } },
    },
  });
}
