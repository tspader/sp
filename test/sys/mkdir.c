#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_mkdir)

UTEST_F(sys_mkdir, creates_directory) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_mkdir_creates_directory",
    .steps = {
      { .kind = SYS_STEP_MKDIR, .mkdir = { .path = "dir" } },
      { .kind = SYS_STEP_STAT, .stat = { .path = "dir", .kind = SP_FS_KIND_DIR } },
    },
  });
}

UTEST_F(sys_mkdir, refuses_existing_path) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_mkdir_refuses_existing_path",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_MKDIR, .mkdir = { .path = "file.bin", .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  });
}

UTEST_F(sys_mkdir, refuses_missing_parent) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_mkdir_refuses_missing_parent",
    .steps = {
      { .kind = SYS_STEP_MKDIR, .mkdir = { .path = "a/b", .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "a" },
    },
  });
}
