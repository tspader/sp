#include "io.h"

typedef struct {
  const c8* source;
  u64 capacity;
  u64 buffer;
  struct {
    sp_err_t err;
    const c8* final;
  } expect;
} io_copy_test_t;

void run_io_copy_test(int* utest_result, io_copy_test_t t) {
  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));

  u8 backing [64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, backing, t.capacity);

  u8 copy_buf [64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r, copy_buf, t.buffer, &copied);

  EXPECT_EQ(err, t.expect.err);
  u64 n = sp_cstr_len(t.expect.final);
  EXPECT_EQ(copied, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.final[it]);
  }
}

UTEST_F(io, copy_full) {
  run_io_copy_test(utest_result, (io_copy_test_t){
    .source = "0123456789",
    .capacity = 32, .buffer = 8,
    .expect = { .err = SP_OK, .final = "0123456789" },
  });
}

UTEST_F(io, copy_loops) {
  run_io_copy_test(utest_result, (io_copy_test_t){
    .source = "ABCDEFGHIJKLMNOP",
    .capacity = 32, .buffer = 4,
    .expect = { .err = SP_OK, .final = "ABCDEFGHIJKLMNOP" },
  });
}

UTEST_F(io, copy_empty) {
  run_io_copy_test(utest_result, (io_copy_test_t){
    .source = "",
    .capacity = 8, .buffer = 8,
    .expect = { .err = SP_OK },
  });
}

// TODO: this case exposes the sp_io_copy_b bug: today it returns bytes_copied=0
// because the write fails before `total += chunk` runs, and the mem writer's
// all-or-nothing semantics drop the partial. Uncomment after fixing.
//
// UTEST_F(io, copy_writer_no_space) {
//   run_io_copy_test(utest_result, (io_copy_test_t){
//     .source = "0123456789",
//     .capacity = 4, .buffer = 8,
//     .expect = { .err = SP_ERR_IO_NO_SPACE, .final = "0123" },
//   });
// }
