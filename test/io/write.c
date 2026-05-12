#include "io.h"

typedef struct {
  u64 capacity;
  io_step_t steps [IO_MAX_STEPS];
  struct {
    const c8* content;
  } expect;
} io_write_test_t;

void run_io_write_test(int* utest_result, io_write_test_t t) {
  u8 backing [64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, backing, t.capacity);

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    switch (step->kind) {
      case IO_STEP_WRITE: {
        u64 bytes = 0;
        sp_err_t err = sp_io_write(&w.base, step->write.data, sp_cstr_len(step->write.data), &bytes);
        EXPECT_EQ(err, step->write.err);
        EXPECT_EQ(bytes, step->write.bytes);
        break;
      }
      case IO_STEP_FLUSH: {
        sp_err_t err = sp_io_flush(&w.base);
        EXPECT_EQ(err, step->flush.err);
        break;
      }
      default: {
        sp_unreachable_case();
      }
    }
  }

  u64 n = sp_cstr_len(t.expect.content);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.content[it]);
  }
}

UTEST_F(io, write_fits) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 16,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "hello", SP_OK, 5 } },
    },
    .expect.content = "hello",
  });
}

UTEST_F(io, write_exact_fit) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
    },
    .expect.content = "abcd",
  });
}

UTEST_F(io, write_overflow) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect.content = "abcd",
  });
}

UTEST_F(io, write_barely_overflows) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcde", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect.content = "abcd",
  });
}

UTEST_F(io, write_smaller_after_overflow) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
      { .kind = IO_STEP_WRITE, .write = { "x", SP_ERR_IO_NO_SPACE, 0 } },
    },
    .expect.content = "abcd",
  });
}

UTEST_F(io, write_appends) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 16,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "def", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "ghi", SP_OK, 3 } },
    },
    .expect.content = "abcdefghi",
  });
}

UTEST_F(io, write_zero) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "", SP_OK, 0 } },
    },
  });
}

UTEST_F(io, write_flush_empty) {
  run_io_write_test(utest_result, (io_write_test_t){
    .capacity = 8,
    .steps = {
      { .kind = IO_STEP_FLUSH, .flush = { SP_OK } },
    },
  });
}
