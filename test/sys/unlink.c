#include "harness.h"

static const sys_case_t tests [] = {
  {
    .name = "removes_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "A" } },
    },
    .expect = {
      { .path = "A" },
    },
  },
  {
    .name = "removes_read_only_file",
    .setup = {
      { .path = "A", .kind = SYS_SETUP_READ_ONLY_FILE, .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "A" } },
    },
    .expect = {
      { .path = "A" },
    },
  },
  // disabled: unlinking a directory maps to SP_ERR_SYS_IS_DIR on linux/windows
  // (EISDIR, FILE_NON_DIRECTORY_FILE) but SP_ERR_SYS_ACCESS_DENIED on macos (EPERM)
  // {
  //   .name = "refuses_directory",
  //   .setup = {
  //     { .path = "D", .kind = SYS_SETUP_DIR },
  //   },
  //   .steps = {
  //     { .kind = SYS_STEP_UNLINK, .unlink = { .path = "D", .err = SP_ERR_SYS_IS_DIR } },
  //   },
  //   .expect = {
  //     { .path = "D", .exists = true },
  //   },
  // },
  {
    .name = "refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "A", .err = SP_ERR_SYS_NOT_FOUND } },
    },
  },
  {
    .name = "removes_symlink_not_target",
    .setup = {
      { .path = "T", .content = "A" },
      { .path = "L", .kind = SYS_SETUP_SYMLINK, .target = "T" },
    },
    .steps = {
      { .kind = SYS_STEP_UNLINK, .unlink = { .path = "L" } },
    },
    .expect = {
      { .path = "L" },
      { .path = "T", .exists = true, .content = "A" },
    },
  },
};

sp_test_each_fn(sys, unlink, sys_case_t, tests, sys_case_run);
