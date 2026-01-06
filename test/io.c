#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()


struct io_rw {
  sp_str_t file_path;
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(io_rw) {
  sp_test_file_manager_init(&ut.file_manager);
  ut.file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io_rw.file"));
}

UTEST_F_TEARDOWN(io_rw) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(io_rw, reader_mem_read_full) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r = sp_io_reader_from_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&r, dest, sizeof(dest));

  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_read_partial) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[8] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r = sp_io_reader_from_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&r, dest, sizeof(dest));

  EXPECT_EQ(bytes, 8);
  sp_for(i, 8) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_read_past_end) {
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r = sp_io_reader_from_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&r, dest, sizeof(dest));

  EXPECT_EQ(bytes, 8);
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_size) {
  u8 buffer[128];
  sp_io_reader_t r = sp_io_reader_from_mem(buffer, sizeof(buffer));
  EXPECT_EQ(sp_io_reader_size(&r), 128);
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_seek) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_for(i, 64) buffer[i] = (u8)i;

  sp_io_reader_t r = sp_io_reader_from_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_reader_seek(&r, 32, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 32);

  u8 val;
  sp_io_read(&r, &val, 1);
  EXPECT_EQ(val, 32);

  pos = sp_io_reader_seek(&r, 0, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 64);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_seek_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_reader_t r = sp_io_reader_from_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_reader_seek(&r, 100, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);

  pos = sp_io_reader_seek(&r, -10, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_read) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  EXPECT_TRUE(r.file.fd >= 0);

  char buffer[16] = {0};
  u64 bytes = sp_io_read(&r, buffer, 16);
  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(buffer[i], content[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_seek) {
  const char* content = "0123456789";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 10);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  s64 pos = sp_io_reader_seek(&r, 5, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&r, buffer, 5);
  EXPECT_EQ(buffer[0], '5');
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_size) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  EXPECT_EQ(sp_io_reader_size(&r), 16);
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_nonexistent) {
  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("nonexistent.file"));
  sp_io_reader_t r = sp_io_reader_from_file(path);
  EXPECT_EQ(sp_err_get(), SP_ERR_IO_OPEN_FAILED);
}

UTEST_F(io_rw, writer_mem_write) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  u8 source[8] = {1,2,3,4,5,6,7,8};

  sp_io_writer_t w = sp_io_writer_from_mem(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&w, source, sizeof(source));

  EXPECT_EQ(bytes, 8);
  sp_for(i, 8) {
    EXPECT_EQ(buffer[i], source[i]);
  }
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_mem_write_overflow) {
  u8 buffer[8] = SP_ZERO_INITIALIZE();
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  sp_io_writer_t w = sp_io_writer_from_mem(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&w, source, sizeof(source));

  EXPECT_EQ(bytes, 8);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_mem_seek) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_writer_t w = sp_io_writer_from_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_writer_seek(&w, 32, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 32);

  pos = sp_io_writer_seek(&w, 0, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 64);

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_mem_size) {
  u8 buffer[128];
  sp_io_writer_t w = sp_io_writer_from_mem(buffer, sizeof(buffer));
  EXPECT_EQ(sp_io_writer_size(&w), 128);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_file_write) {
  const char* content = "test data";
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  EXPECT_TRUE(w.file.fd >= 0);

  u64 bytes = sp_io_write(&w, content, 9);
  EXPECT_EQ(bytes, 9);
  sp_io_writer_close(&w);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 9);
}

UTEST_F(io_rw, writer_file_overwrite) {
  const char* first = "XXXXXXXX";
  const char* second = "1234";

  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, first, 8);
    sp_io_writer_close(&w);
  }

  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, second, 4);
    sp_io_writer_close(&w);
  }

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 4);
  EXPECT_EQ(loaded.data[0], '1');
  EXPECT_EQ(loaded.data[3], '4');
}

UTEST_F(io_rw, writer_file_append) {
  const char* first = "first";
  const char* second = "second";

  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, first, 5);
    sp_io_writer_close(&w);
  }

  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_APPEND);
    sp_io_write(&w, second, 6);
    sp_io_writer_close(&w);
  }

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}

UTEST_F(io_rw, writer_file_seek) {
  const char* content = "0123456789";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 10);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  s64 pos = sp_io_reader_seek(&r, 5, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&r, buffer, 5);
  EXPECT_EQ(buffer[0], '5');
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_file_size) {
  const char* content = "0123456789ABCDEF";
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w, content, 16);
  EXPECT_EQ(sp_io_writer_size(&w), 16);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_file_pad) {
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w, "AA", 2);
  sp_io_pad(&w, 3);
  sp_io_write(&w, "BB", 2);
  sp_io_writer_close(&w);

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  u8 result[7] = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_read(&r, result, 7), 7);
  EXPECT_EQ(result[0], 'A');
  EXPECT_EQ(result[1], 'A');
  EXPECT_EQ(result[2], 0);
  EXPECT_EQ(result[3], 0);
  EXPECT_EQ(result[4], 0);
  EXPECT_EQ(result[5], 'B');
  EXPECT_EQ(result[6], 'B');
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_dyn_write) {
  sp_io_writer_t w = sp_io_writer_from_dyn_mem();
  u8 data[] = {1, 2, 3, 4};
  u64 written = sp_io_write(&w, data, 4);

  EXPECT_EQ(written, 4);
  EXPECT_EQ(sp_io_writer_size(&w), 4);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_grows) {
  sp_io_writer_t w = sp_io_writer_from_dyn_mem();

  u8 data[256];
  sp_for(i, 256) data[i] = (u8)i;

  u64 written = sp_io_write(&w, data, 256);
  EXPECT_EQ(written, 256);
  EXPECT_EQ(sp_io_writer_size(&w), 256);

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_to_str) {
  sp_io_writer_t w = sp_io_writer_from_dyn_mem();

  const char* text = "hello world";
  sp_io_write(&w, text, 11);

  sp_str_t str = sp_io_writer_as_str(&w);
  EXPECT_EQ(str.len, 11);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("hello world")));

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_seek) {
  sp_io_writer_t w = sp_io_writer_from_dyn_mem();

  u8 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  sp_io_write(&w, data, 8);

  s64 pos = sp_io_writer_seek(&w, 4, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 4);

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_multiple_writes) {
  sp_io_writer_t w = sp_io_writer_from_dyn_mem();

  sp_io_write(&w, "abc", 3);
  sp_io_write(&w, "def", 3);
  sp_io_write(&w, "ghi", 3);

  EXPECT_EQ(sp_io_writer_size(&w), 9);

  sp_str_t str = sp_io_writer_as_str(&w);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("abcdefghi")));

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_buffered_1000_bytes) {
  u8 write_buf[64];
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  sp_for(i, 1000) {
    u8 byte = (u8)(i & 0xFF);
    EXPECT_EQ(sp_io_write(&w, &byte, 1), 1);
  }
  sp_io_writer_close(&w);

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  EXPECT_EQ(sp_io_reader_size(&r), 1000);

  sp_for(i, 1000) {
    u8 byte;
    EXPECT_EQ(sp_io_read(&r, &byte, 1), 1);
    EXPECT_EQ(byte, (u8)(i & 0xFF));
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_buffered_larger_than_buffer) {
  u8 write_buf[32];
  u8 data[128];
  sp_for(i, 128) data[i] = (u8)i;

  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  EXPECT_EQ(sp_io_write(&w, data, 128), 128);
  sp_io_writer_close(&w);

  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  u8 result[128] = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_read(&r, result, 128), 128);
  sp_for(i, 128) {
    EXPECT_EQ(result[i], data[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_buffered_implicit_flush) {
  u8 write_buf[64];
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  sp_io_write(&w, "hello", 5);
  sp_io_writer_close(&w);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 5);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("hello")));
}

UTEST_F(io_rw, writer_buffered_flush_empty) {
  u8 write_buf[64];
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  sp_err_t err = sp_io_flush(&w);
  EXPECT_EQ(err, SP_ERR_OK);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_buffered_set_twice) {
  u8 write_buf1[64];
  u8 write_buf2[64];
  sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf1, sizeof(write_buf1));

  sp_io_write(&w, "first", 5);
  sp_io_writer_set_buffer(&w, write_buf2, sizeof(write_buf2));
  sp_io_write(&w, "second", 6);
  sp_io_writer_close(&w);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}

UTEST_F(io_rw, reader_buffered_read) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char result[16] = {0};
  u64 bytes = sp_io_read(&r, result, 16);
  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(result[i], content[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_small_reads) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  sp_for(i, 16) {
    char c;
    EXPECT_EQ(sp_io_read(&r, &c, 1), 1);
    EXPECT_EQ(c, content[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_seek_discards_buffer) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = sp_io_writer_from_file(ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = sp_io_reader_from_file(ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char c;
  sp_io_read(&r, &c, 1);
  EXPECT_EQ(c, '0');

  sp_io_reader_seek(&r, 10, SP_IO_SEEK_SET);
  sp_io_read(&r, &c, 1);
  EXPECT_EQ(c, 'A');

  sp_io_reader_close(&r);
}
