#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_write)

UTEST_F(sys_write, advances_file_position) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_write_advances_file_position",
    .setup = {
      { .path = "file.bin", .content = "0123" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "xx" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "yy" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "xxyy" },
    },
  });
}

UTEST_F(sys_write, append_writes_at_current_eof) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_write_append_writes_at_current_eof",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_OPEN, .open = { .slot = 1, .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_PWRITE, .pwrite = { .slot = 1, .data = "BBBB", .offset = 4 } },
      { .kind = SYS_STEP_CLOSE, .close = { .slot = 1 } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "CC" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAABBBBCC" },
    },
  });
}
