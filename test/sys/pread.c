#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_pread)

UTEST_F(sys_pread, reads_at_offset) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_pread_reads_at_offset",
    .setup = {
      { .path = "file.bin", .content = "0123456789ABCDEF" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .flags = SP_O_RDONLY | SP_O_BINARY } },
      { .kind = SYS_STEP_PREAD, .pread = { .count = 4, .offset = 12, .expect = "CDEF" } },
    },
  });
}

UTEST_F(sys_pread, preserves_file_position) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_pread_preserves_file_position",
    .setup = {
      { .path = "file.bin", .content = "0123456789ABCDEF" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .flags = SP_O_RDONLY | SP_O_BINARY } },
      { .kind = SYS_STEP_READ, .read = { .count = 4, .expect = "0123" } },
      { .kind = SYS_STEP_PREAD, .pread = { .count = 4, .offset = 8, .expect = "89AB" } },
      { .kind = SYS_STEP_READ, .read = { .count = 4, .expect = "4567" } },
    },
  });
}
