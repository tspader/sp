#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()


struct io_rw {
  sp_str_t file_path;
  sp_test_file_manager_t file_manager;
  sp_mem_arena_t* arena;
  sp_mem_t mem;
};

UTEST_F_SETUP(io_rw) {
  ut.arena = sp_mem_arena_new();
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
  sp_test_file_manager_init(&ut.file_manager);
  ut.file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io_rw.file"));
}

UTEST_F_TEARDOWN(io_rw) {
  sp_test_file_manager_cleanup(&ut.file_manager);
  sp_mem_arena_destroy(ut.arena);
}

UTEST_F(io_rw, reader_mem_read_full) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r; sp_io_reader_from_mem(&r,source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_OK);

  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_read_partial) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[8] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r; sp_io_reader_from_mem(&r,source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_OK);

  EXPECT_EQ(bytes, 8);
  sp_for(i, 8) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_read_past_end) {
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r; sp_io_reader_from_mem(&r,source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_OK);

  EXPECT_EQ(bytes, 8);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_read_eof_exact) {
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[8] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r; sp_io_reader_from_mem(&r, source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, 8);

  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_read_eof_empty) {
  u8 dest[4] = SP_ZERO_INITIALIZE();

  sp_io_reader_t r; sp_io_reader_from_mem(&r, SP_NULLPTR, 0);
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_size) {
  u8 buffer[128] = SP_ZERO_INITIALIZE();
  sp_io_reader_t r; sp_io_reader_from_mem(&r,buffer, sizeof(buffer));
  u64 size = 0;
  EXPECT_EQ(sp_io_reader_size(&r, &size), SP_OK);
  EXPECT_EQ(size, 128);
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_seek) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_for(i, 64) buffer[i] = (u8)i;

  sp_io_reader_t r; sp_io_reader_from_mem(&r,buffer, sizeof(buffer));

  s64 pos = 0;
  EXPECT_EQ(sp_io_reader_seek(&r, 32, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 32);

  u8 val;
  sp_io_read(&r, &val, 1, SP_NULLPTR);
  EXPECT_EQ(val, 32);

  EXPECT_EQ(sp_io_reader_seek(&r, 0, SP_IO_SEEK_END, &pos), SP_OK);
  EXPECT_EQ(pos, 64);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_mem_seek_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_reader_t r; sp_io_reader_from_mem(&r,buffer, sizeof(buffer));

  s64 pos = 0;
  EXPECT_EQ(sp_io_reader_seek(&r, 100, SP_IO_SEEK_SET, &pos), SP_ERR_IO_SEEK_INVALID);
  EXPECT_EQ(pos, -1);

  EXPECT_EQ(sp_io_reader_seek(&r, -10, SP_IO_SEEK_SET, &pos), SP_ERR_IO_SEEK_INVALID);
  EXPECT_EQ(pos, -1);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_read) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_reader_from_file(&r, ut.file_path), SP_OK);

  char buffer[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, buffer, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(buffer[i], content[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_seek) {
  const char* content = "0123456789";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 10, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  s64 pos = 0;
  EXPECT_EQ(sp_io_reader_seek(&r, 5, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&r, buffer, 5, SP_NULLPTR);
  EXPECT_EQ(buffer[0], '5');
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_size) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  u64 size = 0;
  EXPECT_EQ(sp_io_reader_size(&r, &size), SP_OK);
  EXPECT_EQ(size, 16);
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_nonexistent) {
  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("nonexistent.file"));
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_reader_from_file(&r, path), SP_ERR_IO_OPEN_FAILED);
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_read_eof_after_drain) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);

  char buffer[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, buffer, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, buffer, 16, &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_read_eof_short) {
  const char* content = "short";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 5, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);

  char buffer[32] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, buffer, sizeof(buffer), &bytes), SP_OK);
  EXPECT_EQ(bytes, 5);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, buffer, sizeof(buffer), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_file_read_eof_empty) {
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);

  char buffer[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, buffer, sizeof(buffer), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_mem_write) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  u8 source[8] = {1,2,3,4,5,6,7,8};

  sp_io_writer_t w; sp_io_writer_from_mem(&w,buffer, sizeof(buffer));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w, source, sizeof(source), &bytes), SP_OK);

  EXPECT_EQ(bytes, 8);
  sp_for(i, 8) {
    EXPECT_EQ(buffer[i], source[i]);
  }
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_mem_write_overflow) {
  u8 buffer[8] = SP_ZERO_INITIALIZE();
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  sp_io_writer_t w; sp_io_writer_from_mem(&w,buffer, sizeof(buffer));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w, source, sizeof(source), &bytes), SP_ERR_IO_NO_SPACE);

  EXPECT_EQ(bytes, 0);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_mem_seek) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_writer_t w; sp_io_writer_from_mem(&w,buffer, sizeof(buffer));

  s64 pos = 0;
  EXPECT_EQ(sp_io_writer_seek(&w, 32, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 32);

  EXPECT_EQ(sp_io_writer_seek(&w, 0, SP_IO_SEEK_END, &pos), SP_OK);
  EXPECT_EQ(pos, 64);

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_mem_size) {
  u8 buffer[128];
  sp_io_writer_t w; sp_io_writer_from_mem(&w,buffer, sizeof(buffer));
  u64 size = 0;
  EXPECT_EQ(sp_io_writer_size(&w, &size), SP_OK);
  EXPECT_EQ(size, 128);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_file_write) {
  const char* content = "test data";
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE), SP_OK);

  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w, content, 9, &bytes), SP_OK);
  EXPECT_EQ(bytes, 9);
  sp_io_writer_close(&w);

  sp_str_t loaded = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 9);
}

UTEST_F(io_rw, writer_file_overwrite) {
  const char* first = "XXXXXXXX";
  const char* second = "1234";

  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, first, 8, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, second, 4, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_str_t loaded = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 4);
  EXPECT_EQ(loaded.data[0], '1');
  EXPECT_EQ(loaded.data[3], '4');
}

UTEST_F(io_rw, writer_file_append) {
  const char* first = "first";
  const char* second = "second";

  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, first, 5, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_APPEND);
    sp_io_write(&w, second, 6, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_str_t loaded = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}

UTEST_F(io_rw, writer_file_seek) {
  const char* content = "0123456789";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 10, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  s64 pos = 0;
  EXPECT_EQ(sp_io_reader_seek(&r, 5, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&r, buffer, 5, SP_NULLPTR);
  EXPECT_EQ(buffer[0], '5');
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_file_size) {
  const char* content = "0123456789ABCDEF";
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w, content, 16, SP_NULLPTR);
  u64 size = 0;
  EXPECT_EQ(sp_io_writer_size(&w, &size), SP_OK);
  EXPECT_EQ(size, 16);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_file_pad) {
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w, "AA", 2, SP_NULLPTR);
  sp_io_pad(&w, 3, SP_NULLPTR);
  sp_io_write(&w, "BB", 2, SP_NULLPTR);
  sp_io_writer_close(&w);

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  u8 result[7] = SP_ZERO_INITIALIZE();
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, 7, &bytes), SP_OK);
  EXPECT_EQ(bytes, 7);
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
  sp_io_writer_t w; sp_io_writer_from_dyn_mem_a(ut.mem, &w);
  u8 data[] = {1, 2, 3, 4};
  u64 written = 0;
  EXPECT_EQ(sp_io_write(&w, data, 4, &written), SP_OK);

  EXPECT_EQ(written, 4);
  u64 size = 0;
  sp_io_writer_size(&w, &size);
  EXPECT_EQ(size, 4);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_grows) {
  sp_io_writer_t w; sp_io_writer_from_dyn_mem_a(ut.mem, &w);

  u8 data[256];
  sp_for(i, 256) data[i] = (u8)i;

  u64 written = 0;
  EXPECT_EQ(sp_io_write(&w, data, 256, &written), SP_OK);
  EXPECT_EQ(written, 256);
  u64 size = 0;
  sp_io_writer_size(&w, &size);
  EXPECT_EQ(size, 256);

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_to_str) {
  sp_io_writer_t w; sp_io_writer_from_dyn_mem_a(ut.mem, &w);

  const char* text = "hello world";
  sp_io_write(&w, text, 11, SP_NULLPTR);

  sp_str_t str = sp_mem_buffer_as_str(&w.dyn_mem.buffer);
  EXPECT_EQ(str.len, 11);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("hello world")));

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_seek) {
  sp_io_writer_t w; sp_io_writer_from_dyn_mem_a(ut.mem, &w);

  u8 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  sp_io_write(&w, data, 8, SP_NULLPTR);

  s64 pos = 0;
  EXPECT_EQ(sp_io_writer_seek(&w, 4, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 4);

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_dyn_multiple_writes) {
  sp_io_writer_t w; sp_io_writer_from_dyn_mem_a(ut.mem, &w);

  sp_io_write(&w, "abc", 3, SP_NULLPTR);
  sp_io_write(&w, "def", 3, SP_NULLPTR);
  sp_io_write(&w, "ghi", 3, SP_NULLPTR);

  u64 size = 0;
  sp_io_writer_size(&w, &size);
  EXPECT_EQ(size, 9);

  sp_str_t str = sp_mem_buffer_as_str(&w.dyn_mem.buffer);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("abcdefghi")));

  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_buffered_1000_bytes) {
  u8 write_buf[64];
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  sp_for(i, 1000) {
    u8 byte = (u8)(i & 0xFF);
    u64 bytes = 0;
    EXPECT_EQ(sp_io_write(&w, &byte, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
  }
  sp_io_writer_close(&w);

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  u64 size = 0;
  sp_io_reader_size(&r, &size);
  EXPECT_EQ(size, 1000);

  sp_for(i, 1000) {
    u8 byte;
    u64 bytes = 0;
    EXPECT_EQ(sp_io_read(&r, &byte, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
    EXPECT_EQ(byte, (u8)(i & 0xFF));
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_buffered_larger_than_buffer) {
  u8 write_buf[32];
  u8 data[128];
  sp_for(i, 128) data[i] = (u8)i;

  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w, data, 128, &bytes), SP_OK);
  EXPECT_EQ(bytes, 128);
  sp_io_writer_close(&w);

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  u8 result[128] = SP_ZERO_INITIALIZE();
  u64 read_bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, 128, &read_bytes), SP_OK);
  EXPECT_EQ(read_bytes, 128);
  sp_for(i, 128) {
    EXPECT_EQ(result[i], data[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, writer_buffered_implicit_flush) {
  u8 write_buf[64];
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  sp_io_write(&w, "hello", 5, SP_NULLPTR);
  sp_io_writer_close(&w);

  sp_str_t loaded = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 5);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("hello")));
}

UTEST_F(io_rw, writer_buffered_flush_empty) {
  u8 write_buf[64];
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf, sizeof(write_buf));

  sp_err_t err = sp_io_flush(&w);
  EXPECT_EQ(err, SP_OK);
  sp_io_writer_close(&w);
}

UTEST_F(io_rw, writer_buffered_set_twice) {
  u8 write_buf1[64];
  u8 write_buf2[64];
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w, write_buf1, sizeof(write_buf1));

  sp_io_write(&w, "first", 5, SP_NULLPTR);
  sp_io_writer_set_buffer(&w, write_buf2, sizeof(write_buf2));
  sp_io_write(&w, "second", 6, SP_NULLPTR);
  sp_io_writer_close(&w);

  sp_str_t loaded = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}

UTEST_F(io_rw, reader_buffered_read) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char result[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(result[i], content[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_small_reads) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  sp_for(i, 16) {
    char c;
    u64 bytes = 0;
    EXPECT_EQ(sp_io_read(&r, &c, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
    EXPECT_EQ(c, content[i]);
  }
  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_eof_exact) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char result[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, 1, &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_eof_partial) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char result[32] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, sizeof(result), &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, sizeof(result), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_fill_preserves_drained) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char first[4] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, first, 4, &bytes), SP_OK);
  EXPECT_EQ(bytes, 4);

  char second[7] = {0};
  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, second, 7, &bytes), SP_OK);
  EXPECT_EQ(bytes, 7);
  EXPECT_EQ(second[0], '4');
  EXPECT_EQ(second[1], '5');
  EXPECT_EQ(second[2], '6');
  EXPECT_EQ(second[3], '7');
  EXPECT_EQ(second[4], '8');
  EXPECT_EQ(second[5], '9');
  EXPECT_EQ(second[6], 'A');

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_eof_direct_path) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[4];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char prime;
  EXPECT_EQ(sp_io_read(&r, &prime, 1, SP_NULLPTR), SP_OK);
  EXPECT_EQ(prime, '0');

  char result[32] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, sizeof(result), &bytes), SP_OK);
  EXPECT_EQ(bytes, 15);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, sizeof(result), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_zero_size) {
  u8 source[1] = {0};
  u8 read_buf[8];
  sp_io_reader_t r; sp_io_reader_from_mem(&r, source, sizeof(source));
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  u64 bytes = 1;
  EXPECT_EQ(sp_io_read(&r, source, 0, &bytes), SP_OK);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_eof_byte_by_byte) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  sp_for(i, 16) {
    char c;
    u64 bytes = 0;
    EXPECT_EQ(sp_io_read(&r, &c, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
    EXPECT_EQ(c, content[i]);
  }

  char c;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, &c, 1, &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, reader_buffered_seek_discards_buffer) {
  const char* content = "0123456789ABCDEF";
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w, content, 16, SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  sp_io_reader_set_buffer(&r, read_buf, sizeof(read_buf));

  char c;
  sp_io_read(&r, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, '0');

  sp_io_reader_seek(&r, 10, SP_IO_SEEK_SET, SP_NULLPTR);
  sp_io_read(&r, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, 'A');

  sp_io_reader_close(&r);
}

UTEST_F(io_rw, seek_beyond_4gb) {
  s64 offset = (s64)5 * 1024 * 1024 * 1024;
  u8 marker[4] = {0xDE, 0xAD, 0xBE, 0xEF};

  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  s64 seek_pos = 0;
  EXPECT_EQ(sp_io_writer_seek(&w, offset, SP_IO_SEEK_SET, &seek_pos), SP_OK);
  EXPECT_EQ(seek_pos, offset);
  sp_io_write(&w, marker, 4, SP_NULLPTR);
  sp_io_writer_close(&w);

  sp_io_reader_t r = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&r, ut.file_path);
  u64 size = 0;
  sp_io_reader_size(&r, &size);
  EXPECT_EQ(size, (u64)offset + 4);

  s64 read_pos = 0;
  EXPECT_EQ(sp_io_reader_seek(&r, offset, SP_IO_SEEK_SET, &read_pos), SP_OK);
  EXPECT_EQ(read_pos, offset);

  u8 result[4] = SP_ZERO_INITIALIZE();
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r, result, 4, &bytes), SP_OK);
  EXPECT_EQ(bytes, 4);
  EXPECT_EQ(result[0], 0xDE);
  EXPECT_EQ(result[1], 0xAD);
  EXPECT_EQ(result[2], 0xBE);
  EXPECT_EQ(result[3], 0xEF);
  sp_io_reader_close(&r);
}
