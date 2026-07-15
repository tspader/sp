#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_open)

UTEST_F(sys_open, excl_creates_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_excl_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .flags = SP_O_CREAT | SP_O_EXCL | SP_O_WRONLY | SP_O_BINARY, .mode = 0644 } },
    },
    .expect = {
      { .path = "file.bin", .exists = true },
    },
  });
}

UTEST_F(sys_open, excl_refuses_existing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_excl_refuses_existing_file",
    .setup = {
      { .path = "file.bin", .content = "hello" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .flags = SP_O_CREAT | SP_O_EXCL | SP_O_WRONLY | SP_O_BINARY, .mode = 0644, .fail = true } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "hello" },
    },
  });
}

UTEST_F(sys_open, excl_refuses_symlink) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_excl_refuses_symlink",
    .setup = {
      { .path = "lnk", .kind = SYS_SETUP_SYMLINK, .target = "victim" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "lnk", .flags = SP_O_CREAT | SP_O_EXCL | SP_O_WRONLY | SP_O_BINARY, .mode = 0644, .fail = true } },
    },
    .expect = {
      { .path = "victim" },
    },
  });
}
