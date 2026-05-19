#include "io.h"

UTEST_EMPTY_FIXTURE(io_write)

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  u64 buffer;
  io_step_t steps[IO_MAX_STEPS];
  struct {
    const c8* received;
  } expect;
} io_mock_write_test_t;

////////////
// RUNNER //
////////////
void run_io_mock_write_test(int* utest_result, io_mock_write_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.results, num_responses);

  u8 wrapper_buf[64] = sp_zero;
  if (t.buffer) {
    sp_io_writer_set_buffer(&w.base, wrapper_buf, t.buffer);
  }

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
      case IO_STEP_READ:
      case IO_STEP_SEEK:
      case IO_STEP_SIZE:
      case IO_STEP_COPY:
      case IO_STEP_PAD: {
        sp_unreachable_case();
      }
    }
  }

  EXPECT_EQ(w.cursor, w.num_results);

  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}



UTEST_F(io_write, smoke) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
    },
    .expect = { .received = "abc" },
  });
}

// Partial write paired with NO_SPACE. Caller sees both bytes_written>0 and
// the error.
UTEST_F(io_write, partial_no_space) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 4, .err = SP_ERR_IO_NO_SPACE },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect = { .received = "abcd" },
  });
}

// Hard error with zero bytes accepted.
UTEST_F(io_write, error_zero_bytes) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
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
    .results = {
      { .bytes = 3, .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "xyz", SP_ERR_IO_WRITE_FAILED, 0 } },
    },
    .expect = { .received = "abc" },
  });
}

// Orthogonal contract for writes: partial bytes AND error in one call.
UTEST_F(io_write, bytes_and_error) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 2, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_ERR_IO_WRITE_FAILED, 2 } },
    },
    .expect = { .received = "ab" },
  });
}

// After a NO_SPACE, a smaller write that fits should still succeed: the
// wrapper does not latch the error.
UTEST_F(io_write, smaller_after_overflow) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_NO_SPACE },
      { .bytes = 2, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_ERR_IO_NO_SPACE, 0 } },
      { .kind = IO_STEP_WRITE, .write = { "xy", SP_OK, 2 } },
    },
    .expect = { .received = "xy" },
  });
}

// One backend call returns fewer bytes than requested with no error.
// sp_io_write surfaces that short count and does NOT loop to finish it.
UTEST_F(io_write, single_attempt_short_unbuffered) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 4, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_OK, 4 } },
    },
    .expect = { .received = "abcd" },
  });
}

// A write larger than the buffer bypasses it and issues a single backend
// write. A short backend write surfaces directly; no looping to completion.
UTEST_F(io_write, single_attempt_short_bypass) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 5, .err = SP_OK },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_OK, 5 } },
    },
    .expect = { .received = "abcde" },
  });
}


//////////////
// BUFFERED //
//////////////

// Small write fits in wrapper buffer: backend must NOT be called.
UTEST_F(io_write, buffered_small_no_backend_call) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {0},
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
    },
  });
}

// Multiple small writes accumulate in the buffer; backend stays untouched.
UTEST_F(io_write, buffered_writes_accumulate) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {0},
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "de",  SP_OK, 2 } },
    },
  });
}

// Write that exactly fills the buffer: still no backend call (no overflow).
UTEST_F(io_write, buffered_exact_fit) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {0},
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
    },
  });
}

// Explicit flush of a buffer with data calls the backend exactly once.
UTEST_F(io_write, buffered_flush_drains_buffer) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_FLUSH, .flush = { SP_OK } },
    },
    .expect = { .received = "abc" },
  });
}

// Multiple writes then a single flush: one backend call with the
// concatenation.
UTEST_F(io_write, buffered_multiple_writes_one_flush) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 6, .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "def", SP_OK, 3 } },
      { .kind = IO_STEP_FLUSH, .flush = { SP_OK } },
    },
    .expect = { .received = "abcdef" },
  });
}

// Flushing an empty buffer must NOT call the backend.
UTEST_F(io_write, buffered_flush_empty) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {0},
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_FLUSH, .flush = { SP_OK } },
    },
  });
}

// Buffer has data; next write does not fit in remaining space. Wrapper
// flushes first, then buffers the new write. Backend called once for the
// flush.
UTEST_F(io_write, buffered_overflow_flushes_then_buffers) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 5, .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcde", SP_OK, 5 } },
      { .kind = IO_STEP_WRITE, .write = { "fghij", SP_OK, 5 } },
    },
    .expect = { .received = "abcde" },
  });
}

// Write that is larger than the wrapper buffer bypasses the buffer
// entirely. Backend gets the bytes directly.
UTEST_F(io_write, buffered_large_write_bypasses) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 16, .err = SP_OK },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "0123456789ABCDEF", SP_OK, 16 } },
    },
    .expect = { .received = "0123456789ABCDEF" },
  });
}

// Buffer has some bytes, then a large write arrives. Wrapper flushes the
// buffered prefix and then writes the large payload directly. Two backend
// calls.
UTEST_F(io_write, buffered_drain_then_bypass) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 3,  .err = SP_OK },
      { .bytes = 16, .err = SP_OK },
    },
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc",              SP_OK, 3  } },
      { .kind = IO_STEP_WRITE, .write = { "0123456789ABCDEF", SP_OK, 16 } },
    },
    .expect = { .received = "abc0123456789ABCDEF" },
  });
}

// Backend fails when flush tries to drain. Flush surfaces the error.
UTEST_F(io_write, buffered_flush_backend_error) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_FLUSH, .flush = { SP_ERR_IO_WRITE_FAILED } },
    },
  });
}

// When flush hits a backend that accepts a partial prefix then errors, the
// bytes that didn't make it are dropped: the wrapper buffer is cleared
// regardless of flush outcome. A subsequent write does NOT include the lost
// suffix. Pins "flush failure is non-recoverable; tail is lost" as deliberate.
UTEST_F(io_write, buffered_flush_partial_drops_tail) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {
      { .bytes = 2, .err = SP_ERR_IO_WRITE_FAILED },
      { .bytes = 2, .err = SP_OK },
    },
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
      { .kind = IO_STEP_FLUSH, .flush = { SP_ERR_IO_WRITE_FAILED } },
      { .kind = IO_STEP_WRITE, .write = { "xy", SP_OK, 2 } },
      { .kind = IO_STEP_FLUSH, .flush = { SP_OK } },
    },
    .expect = { .received = "abxy" },
  });
}

// Zero-length write is a no-op: no buffer change, no backend call.
UTEST_F(io_write, buffered_zero_write) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .results = {0},
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "", SP_OK, 0 } },
    },
  });
}


///////////////
// WRITE_ALL  //
///////////////

UTEST_EMPTY_FIXTURE(io_write_all)

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  u64 buffer;
  struct {
    const c8* data;
    sp_err_t err;
    u64 bytes;
  } call;
  struct {
    const c8* received;
  } expect;
} io_mock_write_all_test_t;

void run_io_mock_write_all_test(int* utest_result, io_mock_write_all_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.results, num_responses);

  u8 wrapper_buf[64] = sp_zero;
  if (t.buffer) {
    sp_io_writer_set_buffer(&w.base, wrapper_buf, t.buffer);
  }

  u64 bytes = 0;
  sp_err_t err = sp_io_write_all(&w.base, t.call.data, sp_cstr_len(t.call.data), &bytes);
  EXPECT_EQ(err, t.call.err);
  EXPECT_EQ(bytes, t.call.bytes);

  EXPECT_EQ(w.cursor, w.num_results);

  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}

// write_all keeps calling the backend until every byte is written, even when
// each call accepts only part of what remains.
UTEST_F(io_write_all, loops_until_complete) {
  run_io_mock_write_all_test(utest_result, (io_mock_write_all_test_t){
    .results = {
      { .bytes = 4, .err = SP_OK },
      { .bytes = 4, .err = SP_OK },
    },
    .call = { "abcdefgh", SP_OK, 8 },
    .expect = { .received = "abcdefgh" },
  });
}

// A short first write is retried until the payload is fully accepted. This is
// the contrast with sp_io_write, which would stop after the short call.
UTEST_F(io_write_all, completes_after_short) {
  run_io_mock_write_all_test(utest_result, (io_mock_write_all_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK },
      { .bytes = 5, .err = SP_OK },
    },
    .call = { "abcdefgh", SP_OK, 8 },
    .expect = { .received = "abcdefgh" },
  });
}

// A hard error mid-loop stops write_all; bytes already accepted are reported.
UTEST_F(io_write_all, stops_on_error) {
  run_io_mock_write_all_test(utest_result, (io_mock_write_all_test_t){
    .results = {
      { .bytes = 4, .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .call = { "abcdefgh", SP_ERR_IO_WRITE_FAILED, 4 },
    .expect = { .received = "abcd" },
  });
}

// Orthogonal contract: a call that accepts bytes AND errors contributes those
// bytes to the total before write_all surfaces the error.
UTEST_F(io_write_all, bytes_and_error) {
  run_io_mock_write_all_test(utest_result, (io_mock_write_all_test_t){
    .results = {
      { .bytes = 4, .err = SP_OK },
      { .bytes = 2, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .call = { "abcdefgh", SP_ERR_IO_WRITE_FAILED, 6 },
    .expect = { .received = "abcdef" },
  });
}
