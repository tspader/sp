#include "io.h"

typedef struct {
  const c8* source;
  io_step_t steps [IO_MAX_STEPS];
} io_read_test_t;

void run_io_read_test(int* utest_result, io_read_test_t t) {
  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    u8 dest [64] = sp_zero;
    u64 bytes = 0;
    sp_err_t err = sp_io_read(&r, dest, step->read.request, &bytes);
    EXPECT_EQ(err, step->read.err);

    u64 expect_bytes = sp_cstr_len(step->read.content);
    EXPECT_EQ(bytes, expect_bytes);
    sp_for(it, expect_bytes) {
      EXPECT_EQ((c8)dest[it], step->read.content[it]);
    }
  }
}

UTEST_F(io, read_exact_then_eof) {
  run_io_read_test(utest_result, (io_read_test_t){
    .source = "0123",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_OK, "0123" } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, read_chunked) {
  run_io_read_test(utest_result, (io_read_test_t){
    .source = "01234567",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "012" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "345" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "67" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, read_oversized_request) {
  run_io_read_test(utest_result, (io_read_test_t){
    .source = "abc",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_OK, "abc" } },
      { .kind = IO_STEP_READ, .read = { 16, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, read_empty_source) {
  run_io_read_test(utest_result, (io_read_test_t){
    .source = "",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, read_eof_idempotent) {
  run_io_read_test(utest_result, (io_read_test_t){
    .source = "x",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_OK, "x" } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, read_zero_request) {
  run_io_read_test(utest_result, (io_read_test_t){
    .source = "abc",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 0, SP_OK } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "abc" } },
    },
  });
}
