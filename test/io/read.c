#include "io.h"

UTEST_EMPTY_FIXTURE(io_read)

typedef struct {
  io_mock_response_t responses[IO_MAX_RESPONSES];
  io_step_t steps[IO_MAX_STEPS];
} io_mock_read_test_t;

////////////
// RUNNER //
////////////
void run_io_mock_read_test(int* utest_result, io_mock_read_test_t t) {
  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.responses, num_responses);

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    u8 dest[64] = sp_zero;
    u64 bytes = 0;
    sp_err_t err = sp_io_read(&r.base, dest, step->read.request, &bytes);
    EXPECT_EQ(err, step->read.err);

    u64 expect_bytes = sp_cstr_len(step->read.content);
    EXPECT_EQ(bytes, expect_bytes);
    sp_for(it, expect_bytes) {
      EXPECT_EQ((c8)dest[it], step->read.content[it]);
    }
  }

  EXPECT_EQ(r.cursor, r.num_responses);
}


UTEST_F(io_read, smoke) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "abc" } },
    },
  });
}

// Short read with OK, then EOF. Pins that short reads do NOT signal EOF.
UTEST_F(io_read, short_then_eof) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "abc" } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

// EOF returned at offset 0 (empty stream).
UTEST_F(io_read, eof_empty) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

// EOF is idempotent: each repeated read returns the same terminal state.
UTEST_F(io_read, eof_idempotent) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_EOF },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

// Hard error with zero bytes: not-done -> broken.
UTEST_F(io_read, error_zero_bytes) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}

// Successful read, then a hard error on the next call.
UTEST_F(io_read, error_after_success) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 4, .data = "abcd", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "abcd" } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}

// Orthogonal contract: backend returns bytes AND error in the same call.
// Caller must see the bytes AND the error.
UTEST_F(io_read, bytes_and_error) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED, "abc" } },
    },
  });
}

// After a hard error, subsequent reads continue to report the error if the
// backend continues to surface it. Pins that the wrapper does NOT latch
// errors itself: it is the backend's job to decide whether to retry.
UTEST_F(io_read, error_then_error) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}

