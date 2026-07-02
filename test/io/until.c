#include "io.h"

UTEST_EMPTY_FIXTURE(io_until)

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  u64         buffer;      // wrapper buffer capacity; 0 leaves the reader unbuffered
  const c8*   delim;
  u64         max;
  sp_err_t    err;
  const c8*   expect;      // bytes written to out, including the delimiter
  const c8*   leftover;    // bytes still readable from the reader afterward
} io_until_test_t;

void run_io_until_test(int* utest_result, io_until_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.results, num_responses);

  u8 wrapper_buf[64] = sp_zero;
  if (t.buffer) {
    sp_io_reader_set_buffer(&r.base, wrapper_buf, t.buffer);
  }

  u8 out_buf[64] = sp_zero;
  sp_io_mem_writer_t out = sp_zero;
  sp_io_mem_writer_from_buffer(&out, out_buf, sizeof(out_buf));

  u64 max = t.max ? t.max : sizeof(out_buf);
  u64 bytes = 0;
  sp_err_t err = sp_io_read_until(&r.base, sp_cstr_as_str(t.delim), &out.base, max, &bytes);
  EXPECT_EQ(err, t.err);

  u64 expect_bytes = sp_cstr_len(t.expect);
  EXPECT_EQ(bytes, expect_bytes);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)out_buf, expect_bytes), sp_cstr_as_str(t.expect)));

  if (t.leftover) {
    u8 rest[64] = sp_zero;
    u64 rest_bytes = 0;
    EXPECT_EQ(sp_io_read(&r.base, rest, sizeof(rest), &rest_bytes), SP_OK);
    EXPECT_EQ(rest_bytes, sp_cstr_len(t.leftover));
    EXPECT_TRUE(sp_str_equal(sp_str((c8*)rest, rest_bytes), sp_cstr_as_str(t.leftover)));
  }
}

UTEST_F(io_until, delim_in_one_read) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 15, .err = SP_OK, .data = "HTTP: x\r\n\r\nrest" },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 64,
    .delim = "\r\n\r\n",
    .err = SP_OK,
    .expect = "HTTP: x\r\n\r\n",
    .leftover = "rest",
  });
}

// A tiny wrapper buffer forces the delimiter to straddle three separate
// fills, so the carried tail from one chunk must combine with only part of
// the next chunk to find the match.
UTEST_F(io_until, delim_split_across_many_reads) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 2, .err = SP_OK, .data = "hi" },
      { .bytes = 2, .err = SP_OK, .data = "\r\n" },
      { .bytes = 2, .err = SP_OK, .data = "\r\n" },
    },
    .buffer = 2,
    .delim = "\r\n\r\n",
    .err = SP_OK,
    .expect = "hi\r\n\r\n",
  });
}

UTEST_F(io_until, delim_split_across_reads) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK, .data = "ab\r" },
      { .bytes = 3, .err = SP_OK, .data = "\ncd" },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 64,
    .delim = "\r\n",
    .err = SP_OK,
    .expect = "ab\r\n",
    .leftover = "cd",
  });
}

// A delimiter prefix that dead-ends must not eat the real match that follows.
UTEST_F(io_until, false_start) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 7, .err = SP_OK, .data = "\r\n\r\r\n\r\n" },
    },
    .buffer = 64,
    .delim = "\r\n\r\n",
    .err = SP_OK,
    .expect = "\r\n\r\r\n\r\n",
  });
}

UTEST_F(io_until, eof_before_delim) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK, .data = "abc" },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 64,
    .delim = "\r\n",
    .err = SP_ERR_IO_EOF,
    .expect = "abc",
  });
}

UTEST_F(io_until, max_exceeded) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 8, .err = SP_OK, .data = "aaaaaaaa" },
    },
    .buffer = 64,
    .delim = "\r\n",
    .max = 4,
    .err = SP_ERR_IO_NO_SPACE,
    .expect = "aaaa",
  });
}

// Unbuffered readers get byte-sized backend reads, so nothing past the
// delimiter is consumed. The script has no results to spare: an overread
// would surface as SP_ERR_IO_READ_FAILED.
UTEST_F(io_until, unbuffered_does_not_overread) {
  run_io_until_test(utest_result, (io_until_test_t){
    .results = {
      { .bytes = 1, .err = SP_OK, .data = "a" },
      { .bytes = 1, .err = SP_OK, .data = "\r" },
      { .bytes = 1, .err = SP_OK, .data = "\n" },
    },
    .delim = "\r\n",
    .err = SP_OK,
    .expect = "a\r\n",
  });
}
