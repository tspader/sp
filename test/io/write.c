#include "io.h"

UTEST_EMPTY_FIXTURE(io_write)

typedef struct {
  io_mock_response_t responses[IO_MAX_RESPONSES];
  io_step_t steps[IO_MAX_STEPS];
  struct {
    const c8* received;
  } expect;
} io_mock_write_test_t;

////////////
// RUNNER //
////////////
void run_io_mock_write_test(int* utest_result, io_mock_write_test_t t) {
  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.responses, num_responses);

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
      case IO_STEP_NONE:
      case IO_STEP_READ: {
        sp_unreachable_case();
      }
    }
  }

  EXPECT_EQ(w.cursor, w.num_responses);

  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}



UTEST_F(io_write, smoke) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 3, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
    },
    .expect.received = "abc",
  });
}

// Partial write paired with NO_SPACE. Caller sees both bytes_written>0 and
// the error.
UTEST_F(io_write, partial_no_space) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 4, .err = SP_ERR_IO_NO_SPACE },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect.received = "abcd",
  });
}

// Hard error with zero bytes accepted.
UTEST_F(io_write, error_zero_bytes) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_ERR_IO_WRITE_FAILED, 0 } },
    },
  });
}

// Successful write, then a hard error on the next call.
UTEST_F(io_write, error_after_success) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 3, .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "xyz", SP_ERR_IO_WRITE_FAILED, 0 } },
    },
    .expect.received = "abc",
  });
}

// Orthogonal contract for writes: partial bytes AND error in one call.
UTEST_F(io_write, bytes_and_error) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 2, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_ERR_IO_WRITE_FAILED, 2 } },
    },
    .expect.received = "ab",
  });
}

// After a NO_SPACE, a smaller write that fits should still succeed: the
// wrapper does not latch the error.
UTEST_F(io_write, smaller_after_overflow) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_NO_SPACE },
      { .bytes = 2, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_ERR_IO_NO_SPACE, 0 } },
      { .kind = IO_STEP_WRITE, .write = { "xy", SP_OK, 2 } },
    },
    .expect.received = "xy",
  });
}
