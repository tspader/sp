#include "io.h"

UTEST_EMPTY_FIXTURE(io_copy)

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  struct {
    u64 copy;
    u64 r;
  } buffer;
  struct {
    u64 writer;
  } capacity;
  struct {
    sp_err_t err;
    u64 copied;
    const c8* final;
  } expect;
} io_mock_copy_reader_test_t;

////////////
// RUNNER //
////////////
void run_io_mock_copy_reader_test(int* utest_result, io_mock_copy_reader_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.results, num_responses);

  u8 reader_buf[64] = sp_zero;
  if (t.buffer.r) {
    sp_io_reader_set_buffer(&r.base, reader_buf, t.buffer.r);
  }

  u8 backing[64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, backing, t.capacity.writer);

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r.base, copy_buf, t.buffer.copy, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.final);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.final[it]);
  }
}

typedef struct {
  const c8* source;
  io_result_t responses[IO_MAX_RESPONSES];
  struct {
    u64 copy;
    u64 write;
  } buffer;

  struct {
    sp_err_t err;
    u64 copied;
    const c8* received;
  } expect;
} io_mock_copy_writer_test_t;

void run_io_mock_copy_writer_test(int* utest_result, io_mock_copy_writer_test_t t) {
  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));

  u64 num_responses = io_get_num_results(t.responses, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.responses, num_responses);

  u8 writer_buf[64] = sp_zero;
  if (t.buffer.write) {
    sp_io_writer_set_buffer(&w.base, writer_buf, t.buffer.write);
  }

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r, copy_buf, t.buffer.copy, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}


// Source fails AFTER returning some bytes in a prior call. Copy should
// preserve the bytes that did transfer.
UTEST_F(io_copy, reader_fails_after_success) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .capacity.writer = 32, .buffer.copy = 8,
    .expect = { .err = SP_ERR_IO_READ_FAILED, .copied = 3, .final = "abc" },
  });
}

// Source returns bytes AND error in the same call. Per the orthogonal
// contract, copy should still commit those bytes to the destination.
UTEST_F(io_copy, reader_bytes_and_error) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 3, .data = "abc", .err = SP_ERR_IO_READ_FAILED },
    },
    .capacity.writer = 32, .buffer.copy = 8,
    .expect = { .err = SP_ERR_IO_READ_FAILED, .copied = 3, .final = "abc" },
  });
}

// Writer accepts a partial prefix then signals NO_SPACE on a single call.
// Copy should report the prefix as copied and surface the error.
UTEST_F(io_copy, writer_partial_in_call) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 4, .err = SP_ERR_IO_NO_SPACE },
    },
    .buffer.copy = 8,
    .expect = { .err = SP_ERR_IO_NO_SPACE, .copied = 4, .received = "0123" },
  });
}

// Writer rejects everything on the first call with zero bytes accepted.
// Copy should report copied=0 and the destination should be untouched.
UTEST_F(io_copy, writer_fails_immediately) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .buffer.copy = 8,
    .expect = { .err = SP_ERR_IO_WRITE_FAILED, .copied = 0, .received = "" },
  });
}


//////////////
// BUFFERED //
//////////////

// Reader has a wrapper buffer smaller than the copy buffer. Each backend
// read fills the wrapper buffer; user-facing reads come from there until
// the wrapper buffer drains and the backend is called again. All source
// bytes still reach the destination.
UTEST_F(io_copy, buffered_reader_drains_through_buffer) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 4, .data = "0123", .err = SP_OK },
      { .bytes = 4, .data = "4567", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = { .r = 2, .copy = 2 },
    .capacity.writer = 32,
    .expect = { .err = SP_OK, .copied = 8, .final = "01234567" },
  });
}

// Reader wrapper buffer is larger than the copy buffer; one backend read
// fills it and the copy drains it across multiple small reads.
UTEST_F(io_copy, buffered_reader_one_fill_many_drains) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 8, .data = "ABCDEFGH", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = { .r = 8, .copy = 2 },
    .capacity.writer = 32,
    .expect = { .err = SP_OK, .copied = 8, .final = "ABCDEFGH" },
  });
}

// Writer wrapper buffer is large enough to hold the entire copy. The
// backend must NOT be called: bytes stay buffered. Copy reports the full
// count, but received_len at the backend is 0.
UTEST_F(io_copy, buffered_writer_absorbs_entire_copy) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "01234567",
    .responses = {0},
    .buffer = { .write = 16, .copy = 4 },
    .expect = { .err = SP_OK, .copied = 8, .received = "" },
  });
}

// Writer wrapper buffer smaller than copy size. As the buffer fills, the
// wrapper flushes to the backend. The tail of the data is left in the
// buffer when copy returns because sp_io_copy_b does not flush at end.
// Pins this behavior explicitly.
UTEST_F(io_copy, buffered_writer_overflow_flushes_partial) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "01234567",
    .responses = {
      { .bytes = 4, .err = SP_OK },
    },
    .buffer = { .write = 4, .copy = 2 },
    .expect = { .err = SP_OK, .copied = 8, .received = "0123" },
  });
}

// Writer buffer attached; backend errors on the flush that the wrapper
// triggers when the buffer overflows. Copy surfaces the error.
UTEST_F(io_copy, buffered_writer_flush_error) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "01234567",
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .buffer = { .write = 4, .copy = 2 },
    .expect = { .err = SP_ERR_IO_WRITE_FAILED, .copied = 4, .received = "" },
  });
}
