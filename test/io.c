#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()


struct io {
  sp_str_t file_path;
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(io) {
  sp_test_file_manager_init(&ut.file_manager);
  ut.file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io.file"));
}

UTEST_F_TEARDOWN(io) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(io, memory_open) {
  u8 buffer[64];
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));
  EXPECT_TRUE(true);
  sp_io_close(&io);
}

UTEST_F(io, memory_close) {
  u8 buffer[64];
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));
  sp_io_close(&io);
  EXPECT_TRUE(true);
}

UTEST_F(io, memory_size) {
  u8 buffer[128];
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));
  s64 size = sp_io_size(&io);
  EXPECT_EQ(size, 128);
  sp_io_close(&io);
}

UTEST_F(io, memory_read_full) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_t io = sp_io_from_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&io, dest, sizeof(dest));

  EXPECT_EQ(bytes, 16);
  for (u32 i = 0; i < 16; i++) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, memory_read_partial) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[8] = SP_ZERO_INITIALIZE();

  sp_io_t io = sp_io_from_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&io, dest, sizeof(dest));

  EXPECT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, memory_read_past_end) {
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_t io = sp_io_from_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&io, dest, sizeof(dest));

  EXPECT_EQ(bytes, 8);
  sp_io_close(&io);
}

UTEST_F(io, memory_write_in_bounds) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  u8 source[8] = {1,2,3,4,5,6,7,8};

  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&io, source, sizeof(source));

  EXPECT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    EXPECT_EQ(buffer[i], source[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, memory_write_overflow) {
  u8 buffer[8] = SP_ZERO_INITIALIZE();
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&io, source, sizeof(source));

  EXPECT_EQ(bytes, 8);
  sp_io_close(&io);
}

UTEST_F(io, memory_seek_set) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&io, 32, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 32);

  sp_io_close(&io);
}

UTEST_F(io, memory_seek_bounds) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&io, 0, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 0);

  pos = sp_io_seek(&io, 64, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 64);

  pos = sp_io_seek(&io, 0, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 64);

  sp_io_close(&io);
}

UTEST_F(io, memory_seek_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&io, 100, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);

  pos = sp_io_seek(&io, -10, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);

  sp_io_close(&io);
}

UTEST_F(io, file_open_read) {
  const char* test_content = "Hello, World!";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 13);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  EXPECT_TRUE(io.file.fd >= 0);
  sp_io_close(&io);
}

UTEST_F(io, file_open_write) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  EXPECT_TRUE(io.file.fd >= 0);
  sp_io_close(&io);
}

UTEST_F(io, file_open_nonexistent) {
  sp_str_t file_path = sp_test_file_path(&ut.file_manager, sp_str_lit("sp_io.file_open_nonexistent.file"));
  sp_io_t io = sp_io_from_file(file_path, SP_IO_MODE_READ);
  EXPECT_EQ(sp_err_get(), SP_ERR_IO_OPEN_FAILED);
}

UTEST_F(io, file_close) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&io);
  EXPECT_TRUE(true);
}

UTEST_F(io, file_read_full) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 16);
  sp_io_close(&writer);

  char buffer[16] = {0};
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&io, buffer, 16);

  EXPECT_EQ(bytes, 16);
  for (u32 i = 0; i < 16; i++) {
    EXPECT_EQ(buffer[i], test_content[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, file_read_partial) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 16);
  sp_io_close(&writer);

  char buffer[8] = {0};
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&io, buffer, 8);

  EXPECT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    EXPECT_EQ(buffer[i], test_content[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, file_read_past_eof) {
  const char* test_content = "0123";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 4);
  sp_io_close(&writer);

  char buffer[16] = {0};
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&io, buffer, 16);

  EXPECT_EQ(bytes, 4);
  sp_io_close(&io);
}

UTEST_F(io, file_write_new) {
  const char* test_content = "test data";
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  u64 bytes = sp_io_write(&io, test_content, 9);

  EXPECT_EQ(bytes, 9);
  EXPECT_TRUE(sp_fs_exists(utest_fixture->file_path));
  sp_io_close(&io);
}

UTEST_F(io, file_write_overwrite) {
  const char* first = "XXXXXXXX";
  const char* second = "1234";

  sp_io_t io1 = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&io1, first, 8);
  sp_io_close(&io1);

  sp_io_t io2 = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&io2, second, 4);
  sp_io_close(&io2);

  char buffer[8] = {0};
  sp_io_t reader = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&reader, buffer, 8);

  EXPECT_EQ(bytes, 4);
  EXPECT_EQ(buffer[0], '1');
  EXPECT_EQ(buffer[1], '2');
  EXPECT_EQ(buffer[2], '3');
  EXPECT_EQ(buffer[3], '4');
  sp_io_close(&reader);
}

UTEST_F(io, file_seek_set) {
  const char* test_content = "0123456789";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 10);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&io, 5, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&io, buffer, 5);
  EXPECT_EQ(buffer[0], '5');
  sp_io_close(&io);
}

UTEST_F(io, file_seek_cur) {
  const char* test_content = "0123456789";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 10);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  char buffer[5] = {0};
  sp_io_read(&io, buffer, 3);

  s64 pos = sp_io_seek(&io, 2, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 5);

  sp_io_read(&io, buffer, 1);
  EXPECT_EQ(buffer[0], '5');
  sp_io_close(&io);
}

UTEST_F(io, file_seek_end) {
  const char* test_content = "0123456789";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 10);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&io, -3, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 7);

  char buffer[3] = {0};
  sp_io_read(&io, buffer, 3);
  EXPECT_EQ(buffer[0], '7');
  sp_io_close(&io);
}

UTEST_F(io, file_size_regular) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 16);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&io);
  EXPECT_EQ(size, 16);
  sp_io_close(&io);
}

UTEST_F(io, file_size_empty) {
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&io);
  EXPECT_EQ(size, 0);
  sp_io_close(&io);
}

UTEST_F(io, file_to_memory) {
  const char* test_content = "file to memory test";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 19);
  sp_io_close(&writer);

  sp_io_t file_io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&file_io);
  char buffer[32] = {0};
  sp_io_read(&file_io, buffer, size);
  sp_io_close(&file_io);

  sp_io_t mem_io = sp_io_from_mem(buffer, size);
  char result[32] = {0};
  sp_io_read(&mem_io, result, size);

  EXPECT_EQ(size, 19);
  for (u32 i = 0; i < 19; i++) {
    EXPECT_EQ(result[i], test_content[i]);
  }
  sp_io_close(&mem_io);
}

UTEST_F(io, memory_to_file) {
  const char* test_content = "memory to file test";
  char buffer[32] = {0};
  sp_mem_copy(test_content, buffer, 19);

  sp_io_t mem_io = sp_io_from_mem(buffer, 19);
  char temp[32] = {0};
  sp_io_read(&mem_io, temp, 19);
  sp_io_close(&mem_io);

  sp_io_t file_io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&file_io, temp, 19);
  sp_io_close(&file_io);

  sp_io_t reader = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  char result[32] = {0};
  sp_io_read(&reader, result, 19);
  sp_io_close(&reader);

  for (u32 i = 0; i < 19; i++) {
    EXPECT_EQ(result[i], test_content[i]);
  }
}

UTEST_F(io, load_file_helper) {
  const char* test_content = "load file helper test";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 21);
  sp_io_close(&writer);

  sp_str_t loaded = sp_io_read_file(utest_fixture->file_path);
  EXPECT_EQ(loaded.len, 21);
  for (u32 i = 0; i < 21; i++) {
    EXPECT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(io, file_write_append) {
  const char* first = "first";
  const char* second = "second";

  sp_io_t io1 = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&io1, first, 5);
  sp_io_close(&io1);

  sp_io_t io2 = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_APPEND);
  sp_io_write(&io2, second, 6);
  sp_io_close(&io2);

  sp_str_t loaded = sp_io_read_file(utest_fixture->file_path);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_EQ(loaded.data[0], 'f');
  EXPECT_EQ(loaded.data[4], 't');
  EXPECT_EQ(loaded.data[5], 's');
  EXPECT_EQ(loaded.data[10], 'd');
}

UTEST_F(io, file_read_invalid_fd) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&io);

  io.file.fd = -1;
  char buffer[10] = {0};
  u64 bytes = sp_io_read(&io, buffer, 10);
  EXPECT_EQ(bytes, 0);
}

UTEST_F(io, file_write_invalid_fd) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&io);

  io.file.fd = -1;
  u64 bytes = sp_io_write(&io, "test", 4);
  EXPECT_EQ(bytes, 0);
}

UTEST_F(io, file_seek_invalid_fd) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&io);

  io.file.fd = -1;
  s64 pos = sp_io_seek(&io, 0, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);
}

UTEST_F(io, file_size_invalid_fd) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&io);

  io.file.fd = -1;
  s64 size = sp_io_size(&io);
  EXPECT_EQ(size, -1);
}

UTEST_F(io, file_close_autoclose_false) {
  const char* test_content = "autoclose test";
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  io.file.close_mode = SP_IO_FILE_CLOSE_MODE_NONE;
  sp_io_write(&io, test_content, 14);
  s32 fd = io.file.fd;
  sp_io_close(&io);

  sp_io_t io2 = SP_ZERO_INITIALIZE();
  io2.file.fd = fd;
  io2.file.close_mode = SP_IO_FILE_CLOSE_MODE_AUTO;
  io2.callbacks.close = sp_io_file_close;
  sp_io_close(&io2);

  sp_str_t loaded = sp_io_read_file(utest_fixture->file_path);
  EXPECT_EQ(loaded.len, 14);
  for (u32 i = 0; i < 14; i++) {
    EXPECT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(io, file_close_autoclose_true) {
  const char* test_content = "autoclose true";
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&io, test_content, 14);
  EXPECT_TRUE(io.file.close_mode == SP_IO_FILE_CLOSE_MODE_AUTO);
  sp_io_close(&io);

  sp_str_t loaded = sp_io_read_file(utest_fixture->file_path);
  EXPECT_EQ(loaded.len, 14);
  for (u32 i = 0; i < 14; i++) {
    EXPECT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(io, file_seek_invalid_negative) {
  const char* test_content = "seek test";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 9);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&io, -100, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);
  sp_io_close(&io);
}

UTEST_F(io, file_write_to_readonly) {
  const char* test_content = "initial";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, test_content, 7);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_write(&io, "data", 4);
  EXPECT_EQ(bytes, 0);
  sp_io_close(&io);
}

UTEST_F(io, file_read_from_writeonly) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  char buffer[10] = {0};
  u64 bytes = sp_io_read(&io, buffer, 10);
  EXPECT_EQ(bytes, 0);
  sp_io_close(&io);
}

UTEST_F(io, file_open_read_write) {
  const char* initial = "initial data";
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));
  EXPECT_TRUE(io.file.fd >= 0);

  u64 written = sp_io_write(&io, initial, 12);
  EXPECT_EQ(written, 12);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  char buffer[12] = {0};
  u64 read = sp_io_read(&io, buffer, 12);
  EXPECT_EQ(read, 12);

  for (u32 i = 0; i < 12; i++) {
    EXPECT_EQ(buffer[i], initial[i]);
  }

  sp_io_close(&io);
}

UTEST_F(io, memory_seek_cur_forward) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 64; i++) buffer[i] = (u8)i;

  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  sp_io_seek(&io, 10, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&io, 5, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 15);

  u8 val;
  sp_io_read(&io, &val, 1);
  EXPECT_EQ(val, 15);

  sp_io_close(&io);
}

UTEST_F(io, memory_seek_cur_backward) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 64; i++) buffer[i] = (u8)i;

  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  sp_io_seek(&io, 30, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&io, -10, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 20);

  u8 val;
  sp_io_read(&io, &val, 1);
  EXPECT_EQ(val, 20);

  sp_io_close(&io);
}

UTEST_F(io, memory_seek_cur_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  sp_io_seek(&io, 10, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&io, -20, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, -1);

  pos = sp_io_seek(&io, 100, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, -1);

  sp_io_close(&io);
}

UTEST_F(io, memory_sequential_reads) {
  u8 source[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  sp_io_t io = sp_io_from_mem(source, sizeof(source));

  u8 buf1[4], buf2[4], buf3[4], buf4[4];

  EXPECT_EQ(sp_io_read(&io, buf1, 4), 4);
  EXPECT_EQ(sp_io_read(&io, buf2, 4), 4);
  EXPECT_EQ(sp_io_read(&io, buf3, 4), 4);
  EXPECT_EQ(sp_io_read(&io, buf4, 4), 4);

  EXPECT_EQ(buf1[0], 0);
  EXPECT_EQ(buf2[0], 4);
  EXPECT_EQ(buf3[0], 8);
  EXPECT_EQ(buf4[0], 12);

  sp_io_close(&io);
}

UTEST_F(io, memory_sequential_writes) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  u8 data1[] = {1,2,3,4};
  u8 data2[] = {5,6,7,8};
  u8 data3[] = {9,10,11,12};

  EXPECT_EQ(sp_io_write(&io, data1, 4), 4);
  EXPECT_EQ(sp_io_write(&io, data2, 4), 4);
  EXPECT_EQ(sp_io_write(&io, data3, 4), 4);

  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[4], 5);
  EXPECT_EQ(buffer[8], 9);

  sp_io_close(&io);
}

UTEST_F(io, memory_interleaved_operations) {
  u8 buffer[32] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  u8 data[] = {1,2,3,4};
  sp_io_write(&io, data, 4);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  u8 read_buf[4];
  sp_io_read(&io, read_buf, 4);
  EXPECT_EQ(read_buf[0], 1);

  sp_io_seek(&io, 10, SP_IO_SEEK_SET);

  u8 data2[] = {5,6,7,8};
  sp_io_write(&io, data2, 4);

  sp_io_seek(&io, 10, SP_IO_SEEK_SET);
  sp_io_read(&io, read_buf, 4);
  EXPECT_EQ(read_buf[0], 5);

  sp_io_close(&io);
}

UTEST_F(io, memory_size_zero) {
  u8 dummy;
  sp_io_t io = sp_io_from_mem(&dummy, 0);

  EXPECT_EQ(sp_io_size(&io), 0);

  u8 buffer[10];
  EXPECT_EQ(sp_io_read(&io, buffer, 10), 0);
  EXPECT_EQ(sp_io_write(&io, buffer, 10), 0);

  sp_io_close(&io);
}

UTEST_F(io, file_read_zero_bytes) {
  const char* content = "test";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, content, 4);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);
  char buffer[10];
  u64 bytes = sp_io_read(&io, buffer, 0);
  EXPECT_EQ(bytes, 0);
  sp_io_close(&io);
}

UTEST_F(io, memory_read_zero_bytes) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  u8 dest[10];
  u64 bytes = sp_io_read(&io, dest, 0);
  EXPECT_EQ(bytes, 0);

  sp_io_close(&io);
}

UTEST_F(io, file_write_zero_bytes) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  u64 bytes = sp_io_write(&io, "test", 0);
  EXPECT_EQ(bytes, 0);
  sp_io_close(&io);
}

UTEST_F(io, sp_io_read_file_nonexistent) {
  sp_str_t path = SP_LIT("/tmp/sp_io_nonexistent_xyz_12345.txt");
  sp_str_t result = sp_io_read_file(path);

  SP_EXPECT_ERR(SP_ERR_IO_OPEN_FAILED);
  EXPECT_EQ(result.len, 0);
  EXPECT_EQ(result.data, SP_NULLPTR);
}

UTEST_F(io, sp_io_read_file_empty) {
  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_close(&io);

  sp_str_t result = sp_io_read_file(utest_fixture->file_path);

  EXPECT_EQ(result.len, 0);
}

UTEST_F(io, buffer_open) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  EXPECT_EQ(sp_io_size(&io), 0);
  sp_io_close(&io);
}

UTEST_F(io, buffer_write_small) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  u8 data[] = {1, 2, 3, 4};
  u64 written = sp_io_write(&io, data, 4);

  EXPECT_EQ(written, 4);
  EXPECT_EQ(sp_io_size(&io), 4);
  sp_io_close(&io);
}

UTEST_F(io, buffer_write_grows) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[256];
  for (u32 i = 0; i < 256; i++) data[i] = (u8)i;

  u64 written = sp_io_write(&io, data, 256);
  EXPECT_EQ(written, 256);
  EXPECT_EQ(sp_io_size(&io), 256);

  sp_io_close(&io);
}

UTEST_F(io, buffer_write_read_roundtrip) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[] = {10, 20, 30, 40, 50};
  sp_io_write(&io, data, 5);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  u8 result[5] = SP_ZERO_INITIALIZE();
  u64 read = sp_io_read(&io, result, 5);

  EXPECT_EQ(read, 5);
  for (u32 i = 0; i < 5; i++) {
    EXPECT_EQ(result[i], data[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, buffer_seek_set) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  sp_io_write(&io, data, 8);

  s64 pos = sp_io_seek(&io, 4, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 4);

  u8 val;
  sp_io_read(&io, &val, 1);
  EXPECT_EQ(val, 5);

  sp_io_close(&io);
}

UTEST_F(io, buffer_seek_cur) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  sp_io_write(&io, data, 8);
  sp_io_seek(&io, 2, SP_IO_SEEK_SET);

  s64 pos = sp_io_seek(&io, 3, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 5);

  sp_io_close(&io);
}

UTEST_F(io, buffer_seek_end) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  sp_io_write(&io, data, 8);

  s64 pos = sp_io_seek(&io, -2, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 6);

  u8 val;
  sp_io_read(&io, &val, 1);
  EXPECT_EQ(val, 7);

  sp_io_close(&io);
}

UTEST_F(io, buffer_seek_invalid) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[] = {1, 2, 3, 4};
  sp_io_write(&io, data, 4);

  s64 pos = sp_io_seek(&io, -1, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);

  pos = sp_io_seek(&io, 100, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, -1);

  sp_io_close(&io);
}

UTEST_F(io, buffer_to_str) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  const char* text = "hello world";
  sp_io_write(&io, text, 11);

  sp_str_t str = sp_io_buffer_to_str(&io);
  EXPECT_EQ(str.len, 11);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("hello world")));

  sp_io_close(&io);
}

UTEST_F(io, buffer_multiple_writes) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  sp_io_write(&io, "abc", 3);
  sp_io_write(&io, "def", 3);
  sp_io_write(&io, "ghi", 3);

  EXPECT_EQ(sp_io_size(&io), 9);

  sp_str_t str = sp_io_buffer_to_str(&io);
  EXPECT_TRUE(sp_str_equal(str, sp_str_lit("abcdefghi")));

  sp_io_close(&io);
}

UTEST_F(io, buffer_write_at_position) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  sp_io_write(&io, "XXXX", 4);
  sp_io_seek(&io, 1, SP_IO_SEEK_SET);
  sp_io_write(&io, "YY", 2);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);
  u8 result[4];
  sp_io_read(&io, result, 4);

  EXPECT_EQ(result[0], 'X');
  EXPECT_EQ(result[1], 'Y');
  EXPECT_EQ(result[2], 'Y');
  EXPECT_EQ(result[3], 'X');

  sp_io_close(&io);
}

UTEST_F(io, buffer_write_extends_size) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  sp_io_write(&io, "abc", 3);
  EXPECT_EQ(sp_io_size(&io), 3);

  sp_io_seek(&io, 1, SP_IO_SEEK_SET);
  sp_io_write(&io, "XXXXX", 5);

  EXPECT_EQ(sp_io_size(&io), 6);

  sp_io_close(&io);
}

UTEST_F(io, buffer_read_past_end) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  sp_io_write(&io, "abc", 3);
  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  u8 result[10] = SP_ZERO_INITIALIZE();
  u64 read = sp_io_read(&io, result, 10);

  EXPECT_EQ(read, 3);
  sp_io_close(&io);
}

UTEST_F(io, buffer_large_write) {
  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u8 data[4096];
  for (u32 i = 0; i < 4096; i++) data[i] = (u8)(i & 0xFF);

  u64 written = sp_io_write(&io, data, 4096);
  EXPECT_EQ(written, 4096);
  EXPECT_EQ(sp_io_size(&io), 4096);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  u8 result[4096];
  u64 read = sp_io_read(&io, result, 4096);
  EXPECT_EQ(read, 4096);

  for (u32 i = 0; i < 4096; i++) {
    EXPECT_EQ(result[i], data[i]);
  }

  sp_io_close(&io);
}

UTEST_F(io, file_seek_all_whence_zero_offset) {
  const char* content = "0123456789";
  sp_io_t writer = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_WRITE);
  sp_io_write(&writer, content, 10);
  sp_io_close(&writer);

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, SP_IO_MODE_READ);

  s64 pos = sp_io_seek(&io, 0, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 0);

  sp_io_seek(&io, 5, SP_IO_SEEK_SET);
  pos = sp_io_seek(&io, 0, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 5);

  pos = sp_io_seek(&io, 0, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 10);

  sp_io_close(&io);
}

UTEST_F(io, memory_position_tracking) {
  u8 buffer[16];
  for (u32 i = 0; i < 16; i++) buffer[i] = (u8)i;

  sp_io_t io = sp_io_from_mem(buffer, sizeof(buffer));

  u8 val;
  sp_io_read(&io, &val, 1);
  EXPECT_EQ(val, 0);

  s64 pos = sp_io_seek(&io, 0, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 1);

  sp_io_read(&io, &val, 1);
  EXPECT_EQ(val, 1);

  pos = sp_io_seek(&io, 0, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 2);

  sp_io_close(&io);
}

UTEST_F(io, file_write_read_roundtrip) {
  const char* data = "roundtrip test data";

  sp_io_t io = sp_io_from_file(utest_fixture->file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));

  sp_io_write(&io, data, 19);
  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  char buffer[32] = {0};
  sp_io_read(&io, buffer, 19);

  for (u32 i = 0; i < 19; i++) {
    EXPECT_EQ(buffer[i], data[i]);
  }

  sp_io_close(&io);
}

UTEST_F(io, memory_pad_basic) {
  u32 buffer_size = 32;
  u32 pad_size = 8;

  u8 buffer[32] = SP_ZERO_INITIALIZE();
  sp_mem_fill_u8(buffer, buffer_size, 0xFF);

  sp_io_t io = sp_io_from_mem(buffer, buffer_size);
  EXPECT_EQ(sp_io_pad(&io, pad_size), pad_size);

  sp_for(it, 8) {
    EXPECT_EQ(buffer[it], 0);
  }

  sp_for_range(it, 8, 32) {
    EXPECT_EQ(buffer[it], 0xFF);
  }

  sp_io_close(&io);
}

UTEST_F(io, memory_pad_advances_position) {
  u32 buffer_size = 32;
  u32 pad_size = 10;

  u8 buffer[32] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, buffer_size);

  sp_io_pad(&io, pad_size);
  s64 pos = sp_io_seek(&io, 0, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, pad_size);

  sp_io_close(&io);
}

UTEST_F(io, const_memory_open) {
  const u8 buffer[64] = {0};
  sp_io_t io = sp_io_from_const_mem(buffer, sizeof(buffer));
  EXPECT_TRUE(true);
  sp_io_close(&io);
}

UTEST_F(io, const_memory_size) {
  const u8 buffer[128] = {0};
  sp_io_t io = sp_io_from_const_mem(buffer, sizeof(buffer));
  s64 size = sp_io_size(&io);
  EXPECT_EQ(size, 128);
  sp_io_close(&io);
}

UTEST_F(io, const_memory_read) {
  const u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_t io = sp_io_from_const_mem(source, sizeof(source));
  u64 bytes = sp_io_read(&io, dest, sizeof(dest));

  EXPECT_EQ(bytes, 16);
  sp_for(i, 16) {
    EXPECT_EQ(dest[i], source[i]);
  }
  sp_io_close(&io);
}

UTEST_F(io, const_memory_seek) {
  const u8 buffer[64] = {0};
  sp_io_t io = sp_io_from_const_mem(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&io, 32, SP_IO_SEEK_SET);
  EXPECT_EQ(pos, 32);

  pos = sp_io_seek(&io, 10, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 42);

  pos = sp_io_seek(&io, -4, SP_IO_SEEK_END);
  EXPECT_EQ(pos, 60);

  sp_io_close(&io);
}

UTEST_F(io, const_memory_write_fails) {
  const u8 buffer[64] = {0};
  sp_io_t io = sp_io_from_const_mem(buffer, sizeof(buffer));

  u8 data[] = {1, 2, 3, 4};
  u64 bytes = sp_io_write(&io, data, sizeof(data));

  EXPECT_EQ(bytes, 0);

  sp_io_close(&io);
}

UTEST_F(io, const_memory_pad_fails) {
  const u8 buffer[64] = {0};
  sp_io_t io = sp_io_from_const_mem(buffer, sizeof(buffer));

  u64 bytes = sp_io_pad(&io, 16);

  EXPECT_EQ(bytes, 0);

  sp_io_close(&io);
}


UTEST_F(io, memory_pad_overflow) {
  u32 buffer_size = 16;
  u32 pad_size = 32;

  u8 buffer[16] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, buffer_size);

  u64 padded = sp_io_pad(&io, pad_size);
  EXPECT_EQ(padded, buffer_size);

  sp_io_close(&io);
}

UTEST_F(io, memory_pad_zero) {
  u32 buffer_size = 16;
  u32 pad_size = 0;

  u8 buffer[16] = SP_ZERO_INITIALIZE();
  sp_io_t io = sp_io_from_mem(buffer, buffer_size);

  u64 padded = sp_io_pad(&io, pad_size);
  EXPECT_EQ(padded, 0);

  s64 pos = sp_io_seek(&io, 0, SP_IO_SEEK_CUR);
  EXPECT_EQ(pos, 0);

  sp_io_close(&io);
}

UTEST_F(io, buffer_pad_basic) {
  u32 pad_size = 16;

  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u64 padded = sp_io_pad(&io, pad_size);
  EXPECT_EQ(padded, pad_size);
  EXPECT_EQ(sp_io_size(&io), pad_size);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);
  u8 result[16] = SP_ZERO_INITIALIZE();
  sp_io_read(&io, result, pad_size);

  sp_for(it, pad_size) {
    EXPECT_EQ(result[it], 0);
  }

  sp_io_close(&io);
}

UTEST_F(io, buffer_pad_grows) {
  u32 pad_size = 256;

  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  u64 padded = sp_io_pad(&io, pad_size);
  EXPECT_EQ(padded, pad_size);
  EXPECT_EQ(sp_io_size(&io), pad_size);

  sp_io_close(&io);
}

UTEST_F(io, buffer_pad_after_write) {
  u32 write_size = 3;
  u32 pad_size = 5;
  u32 total_size = write_size + pad_size;

  sp_io_t io = sp_io_from_dyn_mem(SP_NULLPTR, 0);

  sp_io_write(&io, "abc", write_size);
  u64 padded = sp_io_pad(&io, pad_size);

  EXPECT_EQ(padded, pad_size);
  EXPECT_EQ(sp_io_size(&io), total_size);

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);
  u8 result[8] = SP_ZERO_INITIALIZE();
  sp_io_read(&io, result, total_size);

  EXPECT_EQ(result[0], 'a');
  EXPECT_EQ(result[1], 'b');
  EXPECT_EQ(result[2], 'c');

  sp_for_range(it, write_size, total_size) {
    EXPECT_EQ(result[it], 0);
  }

  sp_io_close(&io);
}

UTEST_F(io, file_pad_basic) {
  u32 pad_size = 1972;
  u8* buffer = sp_alloc_n(u8, pad_size);

  {
    sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
    EXPECT_EQ(sp_io_pad(&io, pad_size), pad_size);
    sp_io_close(&io);
  }

  {
    sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_READ);
    EXPECT_EQ(sp_io_size(&io), pad_size);
    sp_io_read(&io, buffer, pad_size);

    sp_for(it, pad_size) {
      EXPECT_EQ(buffer[it], 0);
    }

    sp_io_close(&io);
  }
}

UTEST_F(io, file_pad_after_write) {
  u32 write_size = 5;
  u32 pad_size = 11;
  u32 total_size = write_size + pad_size;
  u8 buffer [16] = SP_ZERO_INITIALIZE();

  {
    sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);

    sp_io_write(&io, "hello", write_size);
    EXPECT_EQ(sp_io_pad(&io, pad_size), pad_size);

    sp_io_close(&io);
  }

  {
    sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_READ);

    EXPECT_EQ(sp_io_size(&io), total_size);
    EXPECT_EQ(sp_io_read(&io, buffer, total_size), total_size);
    EXPECT_EQ(buffer[0], 'h');
    EXPECT_EQ(buffer[4], 'o');

    sp_for_range(it, write_size, total_size) {
      EXPECT_EQ(buffer[it], 0);
    }

    sp_io_close(&io);
  }
}

UTEST_F(io, buffered_write_1000_single_bytes) {
  u8 write_buf[64];
  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  sp_for(i, 1000) {
    u8 byte = (u8)(i & 0xFF);
    EXPECT_EQ(sp_io_write(&io, &byte, 1), 1);
  }
  sp_io_close(&io);

  sp_io_t reader = sp_io_from_file(ut.file_path, SP_IO_MODE_READ);
  EXPECT_EQ(sp_io_size(&reader), 1000);

  sp_for(i, 1000) {
    u8 byte;
    EXPECT_EQ(sp_io_read(&reader, &byte, 1), 1);
    EXPECT_EQ(byte, (u8)(i & 0xFF));
  }
  sp_io_close(&reader);
}

UTEST_F(io, buffered_write_larger_than_buffer) {
  u8 write_buf[32];
  u8 data[128];
  sp_for(i, 128) data[i] = (u8)i;

  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  EXPECT_EQ(sp_io_write(&io, data, 128), 128);
  sp_io_close(&io);

  sp_io_t reader = sp_io_from_file(ut.file_path, SP_IO_MODE_READ);
  u8 result[128] = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_read(&reader, result, 128), 128);
  sp_for(i, 128) {
    EXPECT_EQ(result[i], data[i]);
  }
  sp_io_close(&reader);
}

UTEST_F(io, buffered_write_implicit_flush_on_close) {
  u8 write_buf[64];
  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  sp_io_write(&io, "hello", 5);
  sp_io_close(&io);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 5);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("hello")));
}

UTEST_F(io, buffered_flush_empty_buffer_no_error) {
  u8 write_buf[64];
  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  sp_err_t err = sp_io_flush(&io);
  EXPECT_EQ(err, SP_ERR_OK);
  sp_io_close(&io);
}

UTEST_F(io, buffered_set_buffer_twice_flushes_first) {
  u8 write_buf1[64];
  u8 write_buf2[64];
  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
  sp_io_set_buffer(&io, write_buf1, sizeof(write_buf1));

  sp_io_write(&io, "first", 5);
  sp_io_set_buffer(&io, write_buf2, sizeof(write_buf2));
  sp_io_write(&io, "second", 6);
  sp_io_close(&io);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 11);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("firstsecond")));
}

UTEST_F(io, unbuffered_stream_still_works) {
  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);

  sp_io_write(&io, "test", 4);
  sp_io_close(&io);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 4);
  EXPECT_TRUE(sp_str_equal(loaded, sp_str_lit("test")));
}

UTEST_F(io, buffered_write_seek_back_write) {
  u8 write_buf[64];
  sp_io_t io = sp_io_from_file(ut.file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  sp_io_write(&io, "AAAA", 4);
  sp_io_seek(&io, 1, SP_IO_SEEK_SET);
  sp_io_write(&io, "BB", 2);
  sp_io_close(&io);

  sp_str_t loaded = sp_io_read_file(ut.file_path);
  EXPECT_EQ(loaded.len, 4);
  EXPECT_EQ(loaded.data[0], 'A');
  EXPECT_EQ(loaded.data[1], 'B');
  EXPECT_EQ(loaded.data[2], 'B');
  EXPECT_EQ(loaded.data[3], 'A');
}

UTEST_F(io, buffered_write_read_back) {
  u8 write_buf[64];
  sp_io_t io = sp_io_from_file(ut.file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  sp_io_write(&io, "hello", 5);
  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  c8 result[5] = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_read(&io, result, 5), 5);
  EXPECT_EQ(result[0], 'h');
  EXPECT_EQ(result[4], 'o');
  sp_io_close(&io);
}

UTEST_F(io, buffered_write_pad_write) {
  u8 write_buf[64];
  sp_io_t io = sp_io_from_file(ut.file_path, SP_IO_MODE_WRITE);
  sp_io_set_buffer(&io, write_buf, sizeof(write_buf));

  sp_io_write(&io, "AA", 2);
  sp_io_pad(&io, 3);
  sp_io_write(&io, "BB", 2);
  sp_io_close(&io);

  sp_io_t reader = sp_io_from_file(ut.file_path, SP_IO_MODE_READ);
  u8 result[7] = SP_ZERO_INITIALIZE();
  EXPECT_EQ(sp_io_read(&reader, result, 7), 7);
  EXPECT_EQ(result[0], 'A');
  EXPECT_EQ(result[1], 'A');
  EXPECT_EQ(result[2], 0);
  EXPECT_EQ(result[3], 0);
  EXPECT_EQ(result[4], 0);
  EXPECT_EQ(result[5], 'B');
  EXPECT_EQ(result[6], 'B');
  sp_io_close(&reader);
}
