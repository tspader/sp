#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_rename)

UTEST_F(sys_rename, replaces_existing_target) {
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

UTEST_F(sys_rename, refuses_missing_source) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rename_refuses_missing_source",
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "src.bin", .to = "dst.bin", .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "dst.bin" },
    },
  });
}

UTEST_F(sys_rename, renames_directory) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rename_renames_directory",
    .setup = {
      { .path = "olddir", .kind = SYS_SETUP_DIR },
      { .path = "olddir/file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "olddir", .to = "newdir" } },
    },
    .expect = {
      { .path = "olddir" },
      { .path = "newdir/file.bin", .exists = true, .content = "A" },
    },
  });
}

UTEST_F(sys_rename, honors_dirfds) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_rename_honors_dirfds",
    .setup = {
      { .path = "a", .kind = SYS_SETUP_DIR },
      { .path = "b", .kind = SYS_SETUP_DIR },
      { .path = "a/src.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .slot = 0, .path = "a" } },
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .slot = 1, .path = "b" } },
      { .kind = SYS_STEP_RENAME, .rename = { .from = "src.bin", .from_dir = SYS_DIR_SLOT_0, .to = "dst.bin", .to_dir = SYS_DIR_SLOT_1 } },
    },
    .expect = {
      { .path = "a/src.bin" },
      { .path = "b/dst.bin", .exists = true, .content = "A" },
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
