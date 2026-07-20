#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_read)

UTEST_F(sys_read, reads_sequentially) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_read_reads_sequentially",
    .setup = {
      { .path = "file.bin", .content = "0123456789ABCDEF" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin" } },
      { .kind = SYS_STEP_READ, .read = { .count = 8, .expect = "01234567" } },
      { .kind = SYS_STEP_READ, .read = { .count = 8, .expect = "89ABCDEF" } },
      { .kind = SYS_STEP_READ, .read = { .count = 8, .expect = "" } },
    },
  });
}

UTEST_F(sys_read, clamps_count_beyond_4gib) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_read_clamps_count_beyond_4gib",
    .setup = {
      { .path = "file.bin", .content = "0123456789ABCDEF" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin" } },
      { .kind = SYS_STEP_READ, .read = { .count = 0x100000000ULL, .expect = "0123456789ABCDEF" } },
    },
  });
}
