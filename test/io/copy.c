#include "io.h"

UTEST_EMPTY_FIXTURE(io_copy)

typedef struct {
  io_mock_response_t responses[IO_MAX_RESPONSES];
  u64 writer_capacity;
  u64 buffer;
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
  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.responses, num_responses);

  u8 backing[64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, backing, t.writer_capacity);

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r.base, copy_buf, t.buffer, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.final);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.final[it]);
  }
}

typedef struct {
  const c8* source;
  io_mock_response_t responses[IO_MAX_RESPONSES];
  u64 buffer;
  struct {
    sp_err_t err;
    u64 copied;
    const c8* received;
  } expect;
} io_mock_copy_writer_test_t;

void run_io_mock_copy_writer_test(int* utest_result, io_mock_copy_writer_test_t t) {
  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));

  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.responses, num_responses);

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r, copy_buf, t.buffer, &copied);

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
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .writer_capacity = 32, .buffer = 8,
    .expect = { .err = SP_ERR_IO_READ_FAILED, .copied = 3, .final = "abc" },
  });
}

// Source returns bytes AND error in the same call. Per the orthogonal
// contract, copy should still commit those bytes to the destination.
UTEST_F(io_copy, reader_bytes_and_error) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_ERR_IO_READ_FAILED },
    },
    .writer_capacity = 32, .buffer = 8,
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
    .buffer = 8,
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
    .buffer = 8,
    .expect = { .err = SP_ERR_IO_WRITE_FAILED, .copied = 0, .received = "" },
  });
}
