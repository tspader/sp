#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_rmdir)

UTEST_F(sys_rmdir, removes_empty_directory) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rmdir_removes_empty_directory",
    .setup = {
      { .path = "dir", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_RMDIR, .rmdir = { .path = "dir" } },
    },
    .expect = {
      { .path = "dir" },
    },
  });
}

UTEST_F(sys_rmdir, refuses_nonempty_directory) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rmdir_refuses_nonempty_directory",
    .setup = {
      { .path = "dir", .kind = SYS_SETUP_DIR },
      { .path = "dir/file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_RMDIR, .rmdir = { .path = "dir", .err = SP_ERR_SYS_NOT_EMPTY } },
    },
    .expect = {
      { .path = "dir/file.bin", .exists = true, .content = "A" },
    },
  });
}

UTEST_F(sys_rmdir, refuses_file) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rmdir_refuses_file",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_RMDIR, .rmdir = { .path = "file.bin", .err = SP_ERR_SYS_NOT_DIR } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  });
}

UTEST_F(sys_rmdir, refuses_missing_path) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rmdir_refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_RMDIR, .rmdir = { .path = "dir", .err = SP_ERR_SYS_NOT_FOUND } },
    },
  });
}
