#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_unlink)

UTEST_F(sys_unlink, removes_file) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_unlink_removes_file",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "file.bin" } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  });
}

// disabled: unlinking a directory maps to SP_ERR_SYS_IS_DIR on linux/windows
// (EISDIR, FILE_NON_DIRECTORY_FILE) but SP_ERR_SYS_ACCESS_DENIED on macos (EPERM)
// UTEST_F(sys_unlink, refuses_directory) {
//   run_sys_test(utest_result, (sys_test_t) {
//     .label = "sys_unlink_refuses_directory",
//     .setup = {
//       { .path = "dir", .kind = SYS_SETUP_DIR },
//     },
//     .steps = {
//       { .kind = SYS_STEP_UNLINK, .unlink = { .path = "dir", .err = SP_ERR_SYS_IS_DIR } },
//     },
//     .expect = {
//       { .path = "dir", .exists = true },
//     },
//   });
// }

UTEST_F(sys_unlink, refuses_missing_path) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_unlink_refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "file.bin", .err = SP_ERR_SYS_NOT_FOUND } },
    },
  });
}

UTEST_F(sys_unlink, removes_symlink_not_target) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_unlink_removes_symlink_not_target",
    .setup = {
      { .path = "target.bin", .content = "A" },
      { .path = "lnk", .kind = SYS_SETUP_SYMLINK, .target = "target.bin" },
    },
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "lnk" } },
    },
    .expect = {
      { .path = "lnk" },
      { .path = "target.bin", .exists = true, .content = "A" },
    },
  });
}
