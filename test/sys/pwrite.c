#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_pwrite)

UTEST_F(sys_pwrite, writes_at_offset) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_pwrite_writes_at_offset",
    .setup = {
      { .path = "file.bin", .content = "0123456789ABCDEF" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .flags = SP_O_WRONLY | SP_O_BINARY } },
      { .kind = SYS_STEP_PWRITE, .pwrite = { .data = "xxxx", .offset = 8 } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "01234567xxxxCDEF" },
    },
  });
}

UTEST_F(sys_pwrite, preserves_file_position) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_pwrite_preserves_file_position",
    .setup = {
      { .path = "file.bin", .content = "0123456789ABCDEF" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .flags = SP_O_RDWR | SP_O_BINARY } },
      { .kind = SYS_STEP_READ, .read = { .count = 4, .expect = "0123" } },
      { .kind = SYS_STEP_PWRITE, .pwrite = { .data = "xx", .offset = 8 } },
      { .kind = SYS_STEP_READ, .read = { .count = 4, .expect = "4567" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "01234567xxABCDEF" },
    },
  });
}
