#include "io.h"

UTEST_EMPTY_FIXTURE(io_read)

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  u64 buffer;
  io_step_t steps[IO_MAX_STEPS];
} io_mock_read_test_t;

////////////
// RUNNER //
////////////
void run_io_mock_read_test(int* utest_result, io_mock_read_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.results, num_responses);

  u8 wrapper_buf[64] = sp_zero;
  if (t.buffer) {
    sp_io_reader_set_buffer(&r.base, wrapper_buf, t.buffer);
  }

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

  EXPECT_EQ(r.cursor, r.num_results);
}


UTEST_F(io_read, smoke) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
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
    .results = {
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
    .results = {
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
    .results = {
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
    .results = {
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
    .results = {
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
    .results = {
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
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}


//////////////
// BUFFERED //
//////////////

// First read with empty wrapper buffer: backend is called once to fill it,
// user gets the bytes they asked for, the rest remain in the buffer.
UTEST_F(io_read, buffered_first_read_fills) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 8, .data = "01234567", .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "012" } },
    },
  });
}

// Two reads of a buffer-sized prefill: the second read must NOT call the
// backend (script has only one entry; assertion would fail if it did).
UTEST_F(io_read, buffered_drains_without_backend_call) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 8, .data = "01234567", .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "012" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "345" } },
    },
  });
}

// Drain the buffer completely, then read again. The second physical read
// must trigger a fresh backend call to refill.
UTEST_F(io_read, buffered_refill_after_exhaustion) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 4, .data = "abcd", .err = SP_OK },
      { .bytes = 4, .data = "efgh", .err = SP_OK },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_OK, "abcd" } },
      { .kind = IO_STEP_READ, .read = { 4, SP_OK, "efgh" } },
    },
  });
}

// Request exceeds wrapper buffer capacity: wrapper bypasses its buffer and
// passes the request directly to the backend.
UTEST_F(io_read, buffered_large_request_bypasses) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 16, .data = "0123456789ABCDEF", .err = SP_OK },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_OK, "0123456789ABCDEF" } },
    },
  });
}

// Buffer has some bytes, request is larger than capacity: drain what's
// buffered, then bypass for the remainder.
UTEST_F(io_read, buffered_drain_then_bypass) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 4, .data = "abcd", .err = SP_OK },
      { .bytes = 12, .data = "EFGHIJKLMNOP", .err = SP_OK },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 2, SP_OK, "ab" } },
      { .kind = IO_STEP_READ, .read = { 14, SP_OK, "cdEFGHIJKLMNOP" } },
    },
  });
}

// Empty buffer, request 8: backend returns EOF with no bytes.
UTEST_F(io_read, buffered_eof_immediate) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

// Buffer has some bytes. Request exceeds them. Backend EOFs on the refill.
// Wrapper normalizes "EOF + bytes" to OK; the next call sees the EOF.
UTEST_F(io_read, buffered_eof_after_partial_drain) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 4, .data = "abcd", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 2, SP_OK, "ab" } },
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "cd" } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

// Empty buffer, backend errors immediately. Error propagates.
UTEST_F(io_read, buffered_error_immediate) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}

// Backend short-fills the wrapper buffer. User asked for 3 but backend
// only delivered 2. Wrapper does a single backend call per sp_io_read so
// the user sees a short read.
UTEST_F(io_read, buffered_short_fill) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {
      { .bytes = 2, .data = "ab", .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "ab" } },
    },
  });
}

// Zero-byte request must not call the backend.
UTEST_F(io_read, buffered_zero_request) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .results = {0},
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 0, SP_OK } },
    },
  });
}

