#include "io.h"

UTEST_EMPTY_FIXTURE(io_discard)

// Buffered bytes are tossed by advancing the cursor; the backend is not
// involved. The second script entry is only consumed by the read afterward.
UTEST_F(io_discard, tosses_buffered_bytes_without_backend) {
  io_result_t results[] = {
    { .bytes = 5, .err = SP_OK, .data = "hello" },
    { .bytes = 3, .err = SP_OK, .data = "end" },
  };
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, results, sp_carr_len(results));
  u8 buf[64] = sp_zero;
  sp_io_reader_set_buffer(&r.base, buf, sizeof(buf));

  u8 tmp[8] = sp_zero;
  u64 n = 0;
  EXPECT_EQ(sp_io_read(&r.base, tmp, 2, &n), SP_OK);
  EXPECT_EQ(n, (u64)2);

  u64 discarded = 0;
  EXPECT_EQ(sp_io_discard(&r.base, 3, &discarded), SP_OK);
  EXPECT_EQ(discarded, (u64)3);

  EXPECT_EQ(sp_io_read(&r.base, tmp, 3, &n), SP_OK);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)tmp, (u32)n), sp_str_lit("end")));
}

// A discard larger than the buffer refills and tosses until satisfied,
// leaving any overshoot from the final fill readable.
UTEST_F(io_discard, spans_multiple_fills) {
  io_result_t results[] = {
    { .bytes = 4, .err = SP_OK, .data = "aaaa" },
    { .bytes = 4, .err = SP_OK, .data = "bbbb" },
    { .bytes = 2, .err = SP_OK, .data = "cc" },
  };
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, results, sp_carr_len(results));
  u8 buf[4] = sp_zero;
  sp_io_reader_set_buffer(&r.base, buf, sizeof(buf));

  u64 discarded = 0;
  EXPECT_EQ(sp_io_discard(&r.base, 9, &discarded), SP_OK);
  EXPECT_EQ(discarded, (u64)9);

  u8 tmp[8] = sp_zero;
  u64 n = 0;
  EXPECT_EQ(sp_io_read(&r.base, tmp, sizeof(tmp), &n), SP_OK);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)tmp, (u32)n), sp_str_lit("c")));
}

// EOF before n bytes propagates, with discarded reporting the shortfall.
UTEST_F(io_discard, eof_reports_shortfall) {
  io_result_t results[] = {
    { .bytes = 3, .err = SP_OK, .data = "abc" },
    { .bytes = 0, .err = SP_ERR_IO_EOF },
  };
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, results, sp_carr_len(results));

  u64 discarded = 0;
  EXPECT_EQ(sp_io_discard(&r.base, 5, &discarded), SP_ERR_IO_EOF);
  EXPECT_EQ(discarded, (u64)3);
}

// EOF alongside the bytes that complete the request is normalized to OK,
// same as sp_io_read.
UTEST_F(io_discard, eof_with_final_bytes_is_ok) {
  io_result_t results[] = {
    { .bytes = 3, .err = SP_ERR_IO_EOF, .data = "abc" },
  };
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, results, sp_carr_len(results));

  u64 discarded = 0;
  EXPECT_EQ(sp_io_discard(&r.base, 3, &discarded), SP_OK);
  EXPECT_EQ(discarded, (u64)3);
}

// The file reader's discard is a position bump: skipping never reads, and
// skipping past the end reports EOF with the shortfall.
UTEST_F(io, discard_file_skips_by_position) {
  {
    sp_io_file_writer_t w = sp_zero;
    ASSERT_EQ(sp_io_file_writer_from_path(&w, ut.file_path), SP_OK);
    sp_io_write(&w.base, "hello world", 11, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  ASSERT_EQ(sp_io_file_reader_from_path(&r, ut.file_path), SP_OK);

  u64 discarded = 0;
  EXPECT_EQ(sp_io_discard(&r.base, 6, &discarded), SP_OK);
  EXPECT_EQ(discarded, (u64)6);

  u8 tmp[8] = sp_zero;
  u64 n = 0;
  EXPECT_EQ(sp_io_read(&r.base, tmp, sizeof(tmp), &n), SP_OK);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)tmp, (u32)n), sp_str_lit("world")));

  EXPECT_EQ(sp_io_discard(&r.base, 1, &discarded), SP_ERR_IO_EOF);
  EXPECT_EQ(discarded, (u64)0);

  sp_io_file_reader_close(&r);
}
