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
  SKIP_ON_WASM()
  ut.arena = sp_mem_arena_new(sp_mem_os_new());
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
  sp_test_file_manager_init(&ut.file_manager);
  ut.file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io_rw.file"));
}

UTEST_F_TEARDOWN(io_rw) {
  SKIP_ON_WASM()
  sp_test_file_manager_cleanup(&ut.file_manager);
  sp_mem_arena_destroy(ut.arena);
}

UTEST_F(io_rw, reader_mem_read_full) {
  SKIP_ON_WASM()
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = sp_zero;

  sp_io_reader_t io = sp_zero;
  sp_io_reader_from_mem(&io, source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_OK);

  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(dest[i], source[i]);
  }
}
UTEST_F(io_rw, reader_mem_read_partial) {
  SKIP_ON_WASM()
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[8] = sp_zero;

  sp_io_reader_t io = sp_zero;
  sp_io_reader_from_mem(&io, source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_OK);

  EXPECT_EQ(bytes, 8);
  sp_for(i, 8) {
    EXPECT_EQ(dest[i], source[i]);
  }
}
UTEST_F(io_rw, reader_mem_read_past_end) {
  SKIP_ON_WASM()
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[16] = sp_zero;

  sp_io_reader_t io = sp_zero;
  sp_io_reader_from_mem(&io, source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_OK);

  EXPECT_EQ(bytes, 8);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);
}
UTEST_F(io_rw, reader_mem_read_eof_exact) {
  SKIP_ON_WASM()
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[8] = sp_zero;

  sp_io_reader_t io = sp_zero;
  sp_io_reader_from_mem(&io, source, sizeof(source));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_OK);
  EXPECT_EQ(bytes, 8);

  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);
}
UTEST_F(io_rw, reader_mem_read_eof_empty) {
  SKIP_ON_WASM()
  u8 dest[4] = sp_zero;

  sp_io_reader_t io = sp_zero;
  sp_io_reader_from_mem(&io, SP_NULLPTR, 0);
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&io, dest, sizeof(dest), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);
}
UTEST_F(io_rw, reader_mem_seek) {
  SKIP_ON_WASM()
  u8 buffer[64] = sp_zero;
  sp_for(i, 64) buffer[i] = (u8)i;

  sp_io_reader_t backing = sp_zero;
  sp_io_seeking_reader_t io = sp_zero;
  sp_io_seeking_reader_from_mem(&io, &backing, buffer, sizeof(buffer));

  s64 pos = 0;
  EXPECT_EQ(sp_io_seeking_reader_seek(&io, 32, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 32);

  u8 val;
  sp_io_read(io.reader, &val, 1, SP_NULLPTR);
  EXPECT_EQ(val, 32);

  EXPECT_EQ(sp_io_seeking_reader_seek(&io, 0, SP_IO_SEEK_END, &pos), SP_OK);
  EXPECT_EQ(pos, 64);
}
UTEST_F(io_rw, reader_mem_seek_invalid) {
  SKIP_ON_WASM()
  u8 buffer[64] = sp_zero;
  sp_io_reader_t backing = sp_zero;
  sp_io_seeking_reader_t io = sp_zero;
  sp_io_seeking_reader_from_mem(&io, &backing, buffer, sizeof(buffer));

  s64 pos = 0;
  EXPECT_EQ(sp_io_seeking_reader_seek(&io, 100, SP_IO_SEEK_SET, &pos), SP_ERR_IO_SEEK_INVALID);
  EXPECT_EQ(pos, -1);

  EXPECT_EQ(sp_io_seeking_reader_seek(&io, -10, SP_IO_SEEK_SET, &pos), SP_ERR_IO_SEEK_INVALID);
  EXPECT_EQ(pos, -1);
}
UTEST_F(io_rw, seeking_reader_file_seek) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t fr = sp_zero;
  sp_io_file_reader_from_path(&fr, ut.file_path);

  sp_io_seeking_reader_t sr = sp_zero;
  sp_io_seeking_reader_from_file_reader(&sr, &fr);

  s64 pos = 0;
  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, 5, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(sr.reader, buffer, 5, &bytes), SP_OK);
  EXPECT_EQ(bytes, 5);
  EXPECT_EQ(buffer[0], '5');
  EXPECT_EQ(buffer[4], '9');

  sp_io_file_reader_close(&fr);
}
UTEST_F(io_rw, seeking_reader_file_seek_whence) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t fr = sp_zero;
  sp_io_file_reader_from_path(&fr, ut.file_path);

  sp_io_seeking_reader_t sr = sp_zero;
  sp_io_seeking_reader_from_file_reader(&sr, &fr);

  s64 pos = 0;
  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, 4, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 4);

  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, 3, SP_IO_SEEK_CUR, &pos), SP_OK);
  EXPECT_EQ(pos, 7);

  char c = 0;
  sp_io_read(sr.reader, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, '7');

  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, 0, SP_IO_SEEK_END, &pos), SP_OK);
  EXPECT_EQ(pos, 16);

  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, -2, SP_IO_SEEK_END, &pos), SP_OK);
  EXPECT_EQ(pos, 14);
  sp_io_read(sr.reader, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, 'E');

  sp_io_file_reader_close(&fr);
}
UTEST_F(io_rw, seeking_reader_file_seek_invalid) {
  SKIP_ON_WASM()
  const char* content = "0123456789";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 10, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t fr = sp_zero;
  sp_io_file_reader_from_path(&fr, ut.file_path);

  sp_io_seeking_reader_t sr = sp_zero;
  sp_io_seeking_reader_from_file_reader(&sr, &fr);

  s64 pos = 0;
  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, -10, SP_IO_SEEK_SET, &pos), SP_ERR_IO_SEEK_FAILED);

  sp_io_file_reader_close(&fr);
}
UTEST_F(io_rw, seeking_reader_file_seek_buffered) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t fr = sp_zero;
  sp_io_file_reader_from_path(&fr, ut.file_path);
  sp_io_reader_set_buffer(&fr.base, read_buf, sizeof(read_buf));

  sp_io_seeking_reader_t sr = sp_zero;
  sp_io_seeking_reader_from_file_reader(&sr, &fr);

  char c = 0;
  sp_io_read(sr.reader, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, '0');

  s64 pos = 0;
  EXPECT_EQ(sp_io_seeking_reader_seek(&sr, 10, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 10);

  sp_io_read(sr.reader, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, 'A');

  sp_io_file_reader_close(&fr);
}
UTEST_F(io_rw, reader_file_read) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  EXPECT_EQ(sp_io_file_reader_from_path(&r, ut.file_path), SP_OK);

  char buffer[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, buffer, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(buffer[i], content[i]);
  }
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_file_seek) {
  SKIP_ON_WASM()
  const char* content = "0123456789";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 10, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  s64 pos = 0;
  EXPECT_EQ(sp_io_file_reader_seek(&r, 5, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&r.base, buffer, 5, SP_NULLPTR);
  EXPECT_EQ(buffer[0], '5');
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_file_nonexistent) {
  SKIP_ON_WASM()
  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("nonexistent.file"));
  sp_io_file_reader_t r = sp_zero;
  EXPECT_EQ(sp_io_file_reader_from_path(&r, path), SP_ERR_IO_OPEN_FAILED);
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_file_read_eof_after_drain) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);

  char buffer[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, buffer, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, buffer, 16, &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_file_read_eof_short) {
  SKIP_ON_WASM()
  const char* content = "short";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 5, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);

  char buffer[32] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, buffer, sizeof(buffer), &bytes), SP_OK);
  EXPECT_EQ(bytes, 5);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, buffer, sizeof(buffer), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_file_read_eof_empty) {
  SKIP_ON_WASM()
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);

  char buffer[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, buffer, sizeof(buffer), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, writer_mem_write) {
  SKIP_ON_WASM()
  u8 buffer[16] = sp_zero;
  u8 source[8] = {1,2,3,4,5,6,7,8};

  sp_io_mem_writer_t w; sp_io_mem_writer_from_buffer(&w,buffer, sizeof(buffer));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w.base, source, sizeof(source), &bytes), SP_OK);

  EXPECT_EQ(bytes, 8);
  sp_for(i, 8) {
    EXPECT_EQ(buffer[i], source[i]);
  }
  /* mem writer has no close */;
}
UTEST_F(io_rw, writer_mem_write_overflow) {
  SKIP_ON_WASM()
  u8 buffer[8] = sp_zero;
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  sp_io_mem_writer_t w; sp_io_mem_writer_from_buffer(&w,buffer, sizeof(buffer));
  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w.base, source, sizeof(source), &bytes), SP_ERR_IO_NO_SPACE);

  EXPECT_EQ(bytes, 0);
  /* mem writer has no close */;
}
UTEST_F(io_rw, writer_mem_seek) {
  SKIP_ON_WASM()
  u8 buffer[64] = sp_zero;
  sp_io_mem_writer_t w; sp_io_mem_writer_from_buffer(&w,buffer, sizeof(buffer));

  s64 pos = 0;
  EXPECT_EQ(sp_io_mem_writer_seek(&w, 32, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 32);

  EXPECT_EQ(sp_io_mem_writer_seek(&w, 0, SP_IO_SEEK_END, &pos), SP_OK);
  EXPECT_EQ(pos, 64);

  /* mem writer has no close */;
}
UTEST_F(io_rw, writer_mem_size) {
  SKIP_ON_WASM()
  u8 buffer[128];
  sp_io_mem_writer_t w; sp_io_mem_writer_from_buffer(&w,buffer, sizeof(buffer));
  u64 size = 0;
  EXPECT_EQ(sp_io_mem_writer_size(&w, &size), SP_OK);
  EXPECT_EQ(size, 128);
  /* mem writer has no close */;
}
UTEST_F(io_rw, writer_file_write) {
  SKIP_ON_WASM()
  const char* content = "test data";
  sp_io_file_writer_t w = sp_zero;
  EXPECT_EQ(sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE), SP_OK);

  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w.base, content, 9, &bytes), SP_OK);
  EXPECT_EQ(bytes, 9);
  sp_io_file_writer_close(&w);

  sp_str_t loaded = sp_zero;
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 9);
}
UTEST_F(io_rw, writer_file_overwrite) {
  SKIP_ON_WASM()
  const char* first = "XXXXXXXX";
  const char* second = "1234";

  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, first, 8, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, second, 4, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_str_t loaded = sp_zero;
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 4);
  EXPECT_EQ(loaded.data[0], '1');
  EXPECT_EQ(loaded.data[3], '4');
}
UTEST_F(io_rw, writer_file_append) {
  SKIP_ON_WASM()
  const char* first = "first";
  const char* second = "second";

  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, first, 5, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_APPEND);
    sp_io_write(&w.base, second, 6, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_str_t loaded = sp_zero;
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}
UTEST_F(io_rw, writer_file_seek) {
  SKIP_ON_WASM()
  const char* content = "0123456789";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 10, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  s64 pos = 0;
  EXPECT_EQ(sp_io_file_reader_seek(&r, 5, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&r.base, buffer, 5, SP_NULLPTR);
  EXPECT_EQ(buffer[0], '5');
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, writer_file_size) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w.base, content, 16, SP_NULLPTR);
  u64 size = 0;
  EXPECT_EQ(sp_io_file_writer_size(&w, &size), SP_OK);
  EXPECT_EQ(size, 16);
  sp_io_file_writer_close(&w);
}
UTEST_F(io_rw, writer_file_pad) {
  SKIP_ON_WASM()
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w.base, "AA", 2, SP_NULLPTR);
  sp_io_pad(&w.base, 3, SP_NULLPTR);
  sp_io_write(&w.base, "BB", 2, SP_NULLPTR);
  sp_io_file_writer_close(&w);

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  u8 result[7] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 7, &bytes), SP_OK);
  EXPECT_EQ(bytes, 7);
  EXPECT_EQ(result[0], 'A');
  EXPECT_EQ(result[1], 'A');
  EXPECT_EQ(result[2], 0);
  EXPECT_EQ(result[3], 0);
  EXPECT_EQ(result[4], 0);
  EXPECT_EQ(result[5], 'B');
  EXPECT_EQ(result[6], 'B');
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, writer_dyn_write) {
  SKIP_ON_WASM()
  sp_io_dyn_mem_writer_t w; sp_io_dyn_mem_writer_init_a(ut.mem, &w);
  u8 data[] = {1, 2, 3, 4};
  u64 written = 0;
  EXPECT_EQ(sp_io_write(&w.base, data, 4, &written), SP_OK);

  EXPECT_EQ(written, 4);
  u64 size = 0;
  sp_io_dyn_mem_writer_size(&w, &size);
  EXPECT_EQ(size, 4);
  sp_io_dyn_mem_writer_close(&w);
}
UTEST_F(io_rw, writer_dyn_grows) {
  SKIP_ON_WASM()
  sp_io_dyn_mem_writer_t w; sp_io_dyn_mem_writer_init_a(ut.mem, &w);

  u8 data[256];
  sp_for(i, 256) data[i] = (u8)i;

  u64 written = 0;
  EXPECT_EQ(sp_io_write(&w.base, data, 256, &written), SP_OK);
  EXPECT_EQ(written, 256);
  u64 size = 0;
  sp_io_dyn_mem_writer_size(&w, &size);
  EXPECT_EQ(size, 256);

  sp_io_dyn_mem_writer_close(&w);
}
UTEST_F(io_rw, writer_dyn_to_str) {
  SKIP_ON_WASM()
  sp_io_dyn_mem_writer_t w; sp_io_dyn_mem_writer_init_a(ut.mem, &w);

  const char* text = "hello world";
  sp_io_write(&w.base, text, 11, SP_NULLPTR);

  sp_str_t str = sp_mem_buffer_as_str(&w.storage);
  EXPECT_EQ(str.len, 11);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("hello world")));

  sp_io_dyn_mem_writer_close(&w);
}
UTEST_F(io_rw, writer_dyn_seek) {
  SKIP_ON_WASM()
  sp_io_dyn_mem_writer_t w; sp_io_dyn_mem_writer_init_a(ut.mem, &w);

  u8 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  sp_io_write(&w.base, data, 8, SP_NULLPTR);

  s64 pos = 0;
  EXPECT_EQ(sp_io_dyn_mem_writer_seek(&w, 4, SP_IO_SEEK_SET, &pos), SP_OK);
  EXPECT_EQ(pos, 4);

  sp_io_dyn_mem_writer_close(&w);
}
UTEST_F(io_rw, writer_dyn_multiple_writes) {
  SKIP_ON_WASM()
  sp_io_dyn_mem_writer_t w; sp_io_dyn_mem_writer_init_a(ut.mem, &w);

  sp_io_write(&w.base, "abc", 3, SP_NULLPTR);
  sp_io_write(&w.base, "def", 3, SP_NULLPTR);
  sp_io_write(&w.base, "ghi", 3, SP_NULLPTR);

  u64 size = 0;
  sp_io_dyn_mem_writer_size(&w, &size);
  EXPECT_EQ(size, 9);

  sp_str_t str = sp_mem_buffer_as_str(&w.storage);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("abcdefghi")));

  sp_io_dyn_mem_writer_close(&w);
}
UTEST_F(io_rw, writer_buffered_1000_bytes) {
  SKIP_ON_WASM()
  u8 write_buf[64];
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w.base, write_buf, sizeof(write_buf));

  sp_for(i, 1000) {
    u8 byte = (u8)(i & 0xFF);
    u64 bytes = 0;
    EXPECT_EQ(sp_io_write(&w.base, &byte, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
  }
  sp_io_file_writer_close(&w);

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  s64 end = sp_sys_lseek(r.file, 0, SP_IO_SEEK_END);
  sp_sys_lseek(r.file, 0, SP_IO_SEEK_SET);
  EXPECT_EQ((u64)end, 1000);

  sp_for(i, 1000) {
    u8 byte;
    u64 bytes = 0;
    EXPECT_EQ(sp_io_read(&r.base, &byte, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
    EXPECT_EQ(byte, (u8)(i & 0xFF));
  }
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, writer_buffered_larger_than_buffer) {
  SKIP_ON_WASM()
  u8 write_buf[32];
  u8 data[128];
  sp_for(i, 128) data[i] = (u8)i;

  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w.base, write_buf, sizeof(write_buf));

  u64 bytes = 0;
  EXPECT_EQ(sp_io_write(&w.base, data, 128, &bytes), SP_OK);
  EXPECT_EQ(bytes, 128);
  sp_io_file_writer_close(&w);

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  u8 result[128] = sp_zero;
  u64 read_bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 128, &read_bytes), SP_OK);
  EXPECT_EQ(read_bytes, 128);
  sp_for(i, 128) {
    EXPECT_EQ(result[i], data[i]);
  }
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, writer_buffered_implicit_flush) {
  SKIP_ON_WASM()
  u8 write_buf[64];
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w.base, write_buf, sizeof(write_buf));

  sp_io_write(&w.base, "hello", 5, SP_NULLPTR);
  sp_io_file_writer_close(&w);

  sp_str_t loaded = sp_zero;
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 5);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("hello")));
}
UTEST_F(io_rw, writer_buffered_flush_empty) {
  SKIP_ON_WASM()
  u8 write_buf[64];
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w.base, write_buf, sizeof(write_buf));

  sp_err_t err = sp_io_flush(&w.base);
  EXPECT_EQ(err, SP_OK);
  sp_io_file_writer_close(&w);
}
UTEST_F(io_rw, writer_buffered_set_twice) {
  SKIP_ON_WASM()
  u8 write_buf1[64];
  u8 write_buf2[64];
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_writer_set_buffer(&w.base, write_buf1, sizeof(write_buf1));

  sp_io_write(&w.base, "first", 5, SP_NULLPTR);
  sp_io_writer_set_buffer(&w.base, write_buf2, sizeof(write_buf2));
  sp_io_write(&w.base, "second", 6, SP_NULLPTR);
  sp_io_file_writer_close(&w);

  sp_str_t loaded = sp_zero;
  sp_io_read_file_a(ut.mem, ut.file_path, &loaded);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}
UTEST_F(io_rw, reader_buffered_read) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  char result[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(result[i], content[i]);
  }
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_small_reads) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  sp_for(i, 16) {
    char c;
    u64 bytes = 0;
    EXPECT_EQ(sp_io_read(&r.base, &c, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
    EXPECT_EQ(c, content[i]);
  }
  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_eof_exact) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  char result[16] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 16, &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 1, &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_eof_partial) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  char result[32] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, sizeof(result), &bytes), SP_OK);
  EXPECT_EQ(bytes, 16);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, sizeof(result), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_fill_preserves_drained) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  char first[4] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, first, 4, &bytes), SP_OK);
  EXPECT_EQ(bytes, 4);

  char second[7] = {0};
  bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, second, 7, &bytes), SP_OK);
  EXPECT_EQ(bytes, 7);
  EXPECT_EQ(second[0], '4');
  EXPECT_EQ(second[1], '5');
  EXPECT_EQ(second[2], '6');
  EXPECT_EQ(second[3], '7');
  EXPECT_EQ(second[4], '8');
  EXPECT_EQ(second[5], '9');
  EXPECT_EQ(second[6], 'A');

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_eof_direct_path) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[4];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  char prime;
  EXPECT_EQ(sp_io_read(&r.base, &prime, 1, SP_NULLPTR), SP_OK);
  EXPECT_EQ(prime, '0');

  char result[32] = {0};
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, sizeof(result), &bytes), SP_OK);
  EXPECT_EQ(bytes, 15);

  bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, sizeof(result), &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_zero_size) {
  SKIP_ON_WASM()
  u8 source[1] = {0};
  u8 read_buf[8];
  sp_io_reader_t io = sp_zero;
  sp_io_reader_from_mem(&io, source, sizeof(source));
  sp_io_reader_set_buffer(&io, read_buf, sizeof(read_buf));

  u64 bytes = 1;
  EXPECT_EQ(sp_io_read(&io, source, 0, &bytes), SP_OK);
  EXPECT_EQ(bytes, 0);
}
UTEST_F(io_rw, reader_buffered_eof_byte_by_byte) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  sp_for(i, 16) {
    char c;
    u64 bytes = 0;
    EXPECT_EQ(sp_io_read(&r.base, &c, 1, &bytes), SP_OK);
    EXPECT_EQ(bytes, 1);
    EXPECT_EQ(c, content[i]);
  }

  char c;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, &c, 1, &bytes), SP_ERR_IO_EOF);
  EXPECT_EQ(bytes, 0);

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, reader_buffered_seek_discards_buffer) {
  SKIP_ON_WASM()
  const char* content = "0123456789ABCDEF";
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, content, 16, SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  u8 read_buf[8];
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  sp_io_reader_set_buffer(&r.base, read_buf, sizeof(read_buf));

  char c;
  sp_io_read(&r.base, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, '0');

  sp_io_file_reader_seek(&r, 10, SP_IO_SEEK_SET, SP_NULLPTR);
  sp_io_read(&r.base, &c, 1, SP_NULLPTR);
  EXPECT_EQ(c, 'A');

  sp_io_file_reader_close(&r);
}
UTEST_F(io_rw, seek_beyond_4gb) {
  SKIP_ON_WASM()
  s64 offset = (s64)5 * 1024 * 1024 * 1024;
  u8 marker[4] = {0xDE, 0xAD, 0xBE, 0xEF};

  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  s64 seek_pos = 0;
  EXPECT_EQ(sp_io_file_writer_seek(&w, offset, SP_IO_SEEK_SET, &seek_pos), SP_OK);
  EXPECT_EQ(seek_pos, offset);
  sp_io_write(&w.base, marker, 4, SP_NULLPTR);
  sp_io_file_writer_close(&w);

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  s64 end = sp_sys_lseek(r.file, 0, SP_IO_SEEK_END);
  sp_sys_lseek(r.file, 0, SP_IO_SEEK_SET);
  EXPECT_EQ((u64)end, (u64)offset + 4);

  s64 read_pos = 0;
  EXPECT_EQ(sp_io_file_reader_seek(&r, offset, SP_IO_SEEK_SET, &read_pos), SP_OK);
  EXPECT_EQ(read_pos, offset);

  u8 result[4] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 4, &bytes), SP_OK);
  EXPECT_EQ(bytes, 4);
  EXPECT_EQ(result[0], 0xDE);
  EXPECT_EQ(result[1], 0xAD);
  EXPECT_EQ(result[2], 0xBE);
  EXPECT_EQ(result[3], 0xEF);
  sp_io_file_reader_close(&r);
}
