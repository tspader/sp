#include "io.h"

UTEST_EMPTY_FIXTURE(io_limit)

UTEST_F(io_limit, clamps_reads) {
  io_result_t results[] = {
    { .bytes = 6, .err = SP_OK, .data = "abcdef" },
  };
  io_mock_reader_t inner = sp_zero;
  io_mock_reader_init(&inner, results, sp_carr_len(results));

  sp_io_limit_reader_t limit = sp_zero;
  sp_io_limit_reader_init(&limit, &inner.base, 4);

  u8 dest[16] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&limit.base, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, (u64)4);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)dest, 4), sp_str_lit("abcd")));
  EXPECT_EQ(limit.remaining, (u64)0);

  EXPECT_EQ(sp_io_read(&limit.base, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, (u64)0);
}

UTEST_F(io_limit, zero_is_immediate_eof) {
  io_mock_reader_t inner = sp_zero;
  io_mock_reader_init(&inner, SP_NULLPTR, 0);

  sp_io_limit_reader_t limit = sp_zero;
  sp_io_limit_reader_init(&limit, &inner.base, 0);

  u8 dest[4] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&limit.base, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, (u64)0);
  EXPECT_EQ(inner.cursor, (u64)0);
}

// Inner EOF passes through, and remaining exposes the shortfall. This is how a
// caller detects a truncated fixed-length body.
UTEST_F(io_limit, inner_eof_first) {
  io_result_t results[] = {
    { .bytes = 2, .err = SP_OK, .data = "ab" },
    { .bytes = 0, .err = SP_ERR_IO_EOF },
  };
  io_mock_reader_t inner = sp_zero;
  io_mock_reader_init(&inner, results, sp_carr_len(results));

  sp_io_limit_reader_t limit = sp_zero;
  sp_io_limit_reader_init(&limit, &inner.base, 5);

  u8 dest[16] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&limit.base, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, (u64)2);

  EXPECT_EQ(sp_io_read(&limit.base, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, (u64)0);
  EXPECT_EQ(limit.remaining, (u64)3);
}

// A limit over a buffered inner reader consumes exactly `limit` bytes from the
// inner cursor; surplus buffered bytes stay readable afterward.
UTEST_F(io_limit, surplus_stays_buffered) {
  io_result_t results[] = {
    { .bytes = 6, .err = SP_OK, .data = "abcdef" },
    { .bytes = 0, .err = SP_ERR_IO_EOF },
  };
  io_mock_reader_t inner = sp_zero;
  io_mock_reader_init(&inner, results, sp_carr_len(results));

  u8 inner_buf[16] = sp_zero;
  sp_io_reader_set_buffer(&inner.base, inner_buf, sizeof(inner_buf));

  sp_io_limit_reader_t limit = sp_zero;
  sp_io_limit_reader_init(&limit, &inner.base, 3);

  u8 dest[16] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&limit.base, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, (u64)3);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)dest, 3), sp_str_lit("abc")));

  EXPECT_EQ(sp_io_read(&inner.base, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, (u64)3);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)dest, 3), sp_str_lit("def")));
}

UTEST_F(io_limit, copy_stops_at_limit) {
  io_result_t results[] = {
    { .bytes = 8, .err = SP_OK, .data = "abcdefgh" },
  };
  io_mock_reader_t inner = sp_zero;
  io_mock_reader_init(&inner, results, sp_carr_len(results));

  u8 inner_buf[16] = sp_zero;
  sp_io_reader_set_buffer(&inner.base, inner_buf, sizeof(inner_buf));

  sp_io_limit_reader_t limit = sp_zero;
  sp_io_limit_reader_init(&limit, &inner.base, 5);

  u8 out_buf[16] = sp_zero;
  sp_io_mem_writer_t out = sp_zero;
  sp_io_mem_writer_from_buffer(&out, out_buf, sizeof(out_buf));

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&out.base, &limit.base, &copied), SP_OK);
  EXPECT_EQ(copied, (u64)5);
  EXPECT_EQ(limit.remaining, (u64)0);
  EXPECT_TRUE(sp_str_equal(sp_str((c8*)out_buf, 5), sp_str_lit("abcde")));
}
