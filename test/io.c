#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()


struct io {
  sp_str_t test_file_path;
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(io) {
  sp_test_file_manager_init(&ut.file_manager);
  ut.test_file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io.file"));
}

UTEST_F_TEARDOWN(io) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(io, memory_open) {
  u8 buffer[64];
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  ASSERT_TRUE(true);
  sp_io_close(&stream);
}

UTEST_F(io, memory_close) {
  u8 buffer[64];
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  sp_io_close(&stream);
  ASSERT_TRUE(true);
}

UTEST_F(io, memory_size) {
  u8 buffer[128];
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, 128);
  sp_io_close(&stream);
}

UTEST_F(io, memory_read_full) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));
  u64 bytes = sp_io_read(&stream, dest, sizeof(dest));

  ASSERT_EQ(bytes, 16);
  for (u32 i = 0; i < 16; i++) {
    ASSERT_EQ(dest[i], source[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(io, memory_read_partial) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[8] = SP_ZERO_INITIALIZE();

  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));
  u64 bytes = sp_io_read(&stream, dest, sizeof(dest));

  ASSERT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    ASSERT_EQ(dest[i], source[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(io, memory_read_past_end) {
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));
  u64 bytes = sp_io_read(&stream, dest, sizeof(dest));

  ASSERT_EQ(bytes, 8);
  sp_io_close(&stream);
}

UTEST_F(io, memory_write_in_bounds) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  u8 source[8] = {1,2,3,4,5,6,7,8};

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&stream, source, sizeof(source));

  ASSERT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    ASSERT_EQ(buffer[i], source[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(io, memory_write_overflow) {
  u8 buffer[8] = SP_ZERO_INITIALIZE();
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&stream, source, sizeof(source));

  ASSERT_EQ(bytes, 8);
  sp_io_close(&stream);
}

UTEST_F(io, memory_seek_set) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&stream, 32, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 32);

  sp_io_close(&stream);
}

UTEST_F(io, memory_seek_bounds) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 0);

  pos = sp_io_seek(&stream, 64, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 64);

  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_END);
  ASSERT_EQ(pos, 64);

  sp_io_close(&stream);
}

UTEST_F(io, memory_seek_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&stream, 100, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);

  pos = sp_io_seek(&stream, -10, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);

  sp_io_close(&stream);
}

UTEST_F(io, file_open_read) {
  const char* test_content = "Hello, World!";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 13);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  ASSERT_TRUE(stream.file.fd >= 0);
  sp_io_close(&stream);
}

UTEST_F(io, file_open_write) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  ASSERT_TRUE(stream.file.fd >= 0);
  sp_io_close(&stream);
}

UTEST_F(io, file_open_nonexistent) {
  sp_str_t file_path = sp_test_file_path(&ut.file_manager, sp_str_lit("sp_io.file_open_nonexistent.file"));
  sp_io_stream_t stream = sp_io_from_file(file_path, SP_IO_MODE_READ);
  ASSERT_TRUE(stream.file.fd < 0);
}

UTEST_F(io, file_close) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);
  ASSERT_TRUE(true);
}

UTEST_F(io, file_read_full) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 16);
  sp_io_close(&write_stream);

  char buffer[16] = {0};
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&stream, buffer, 16);

  ASSERT_EQ(bytes, 16);
  for (u32 i = 0; i < 16; i++) {
    ASSERT_EQ(buffer[i], test_content[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(io, file_read_partial) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 16);
  sp_io_close(&write_stream);

  char buffer[8] = {0};
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&stream, buffer, 8);

  ASSERT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    ASSERT_EQ(buffer[i], test_content[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(io, file_read_past_eof) {
  const char* test_content = "0123";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 4);
  sp_io_close(&write_stream);

  char buffer[16] = {0};
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&stream, buffer, 16);

  ASSERT_EQ(bytes, 4);
  sp_io_close(&stream);
}

UTEST_F(io, file_write_new) {
  const char* test_content = "test data";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  u64 bytes = sp_io_write(&stream, test_content, 9);

  ASSERT_EQ(bytes, 9);
  ASSERT_TRUE(sp_fs_exists(utest_fixture->test_file_path));
  sp_io_close(&stream);
}

UTEST_F(io, file_write_overwrite) {
  const char* first = "XXXXXXXX";
  const char* second = "1234";

  sp_io_stream_t stream1 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream1, first, 8);
  sp_io_close(&stream1);

  sp_io_stream_t stream2 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream2, second, 4);
  sp_io_close(&stream2);

  char buffer[8] = {0};
  sp_io_stream_t read_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&read_stream, buffer, 8);

  ASSERT_EQ(bytes, 4);
  ASSERT_EQ(buffer[0], '1');
  ASSERT_EQ(buffer[1], '2');
  ASSERT_EQ(buffer[2], '3');
  ASSERT_EQ(buffer[3], '4');
  sp_io_close(&read_stream);
}

UTEST_F(io, file_seek_set) {
  const char* test_content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&stream, 5, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&stream, buffer, 5);
  ASSERT_EQ(buffer[0], '5');
  sp_io_close(&stream);
}

UTEST_F(io, file_seek_cur) {
  const char* test_content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  char buffer[5] = {0};
  sp_io_read(&stream, buffer, 3);

  s64 pos = sp_io_seek(&stream, 2, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 5);

  sp_io_read(&stream, buffer, 1);
  ASSERT_EQ(buffer[0], '5');
  sp_io_close(&stream);
}

UTEST_F(io, file_seek_end) {
  const char* test_content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&stream, -3, SP_IO_SEEK_END);
  ASSERT_EQ(pos, 7);

  char buffer[3] = {0};
  sp_io_read(&stream, buffer, 3);
  ASSERT_EQ(buffer[0], '7');
  sp_io_close(&stream);
}

UTEST_F(io, file_size_regular) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 16);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, 16);
  sp_io_close(&stream);
}

UTEST_F(io, file_size_empty) {
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, 0);
  sp_io_close(&stream);
}

UTEST_F(io, file_to_memory) {
  const char* test_content = "file to memory test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 19);
  sp_io_close(&write_stream);

  sp_io_stream_t file_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&file_stream);
  char buffer[32] = {0};
  sp_io_read(&file_stream, buffer, size);
  sp_io_close(&file_stream);

  sp_io_stream_t mem_stream = sp_io_from_memory(buffer, size);
  char result[32] = {0};
  sp_io_read(&mem_stream, result, size);

  ASSERT_EQ(size, 19);
  for (u32 i = 0; i < 19; i++) {
    ASSERT_EQ(result[i], test_content[i]);
  }
  sp_io_close(&mem_stream);
}

UTEST_F(io, memory_to_file) {
  const char* test_content = "memory to file test";
  char buffer[32] = {0};
  sp_mem_copy(test_content, buffer, 19);

  sp_io_stream_t mem_stream = sp_io_from_memory(buffer, 19);
  char temp[32] = {0};
  sp_io_read(&mem_stream, temp, 19);
  sp_io_close(&mem_stream);

  sp_io_stream_t file_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&file_stream, temp, 19);
  sp_io_close(&file_stream);

  sp_io_stream_t read_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  char result[32] = {0};
  sp_io_read(&read_stream, result, 19);
  sp_io_close(&read_stream);

  for (u32 i = 0; i < 19; i++) {
    ASSERT_EQ(result[i], test_content[i]);
  }
}

UTEST_F(io, load_file_helper) {
  const char* test_content = "load file helper test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 21);
  sp_io_close(&write_stream);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 21);
  for (u32 i = 0; i < 21; i++) {
    ASSERT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(io, file_write_append) {
  const char* first = "first";
  const char* second = "second";

  sp_io_stream_t stream1 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream1, first, 5);
  sp_io_close(&stream1);

  sp_io_stream_t stream2 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_APPEND);
  sp_io_write(&stream2, second, 6);
  sp_io_close(&stream2);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 11);
  ASSERT_EQ(loaded.data[0], 'f');
  ASSERT_EQ(loaded.data[4], 't');
  ASSERT_EQ(loaded.data[5], 's');
  ASSERT_EQ(loaded.data[10], 'd');
}

UTEST_F(io, file_read_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  char buffer[10] = {0};
  u64 bytes = sp_io_read(&stream, buffer, 10);
  ASSERT_EQ(bytes, 0);
}

UTEST_F(io, file_write_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  u64 bytes = sp_io_write(&stream, "test", 4);
  ASSERT_EQ(bytes, 0);
}

UTEST_F(io, file_seek_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);
}

UTEST_F(io, file_size_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, -1);
}

UTEST_F(io, file_close_autoclose_false) {
  const char* test_content = "autoclose test";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  stream.file.close_mode = SP_IO_FILE_CLOSE_MODE_NONE;
  sp_io_write(&stream, test_content, 14);
  s32 fd = stream.file.fd;
  sp_io_close(&stream);

  sp_io_stream_t stream2 = SP_ZERO_INITIALIZE();
  stream2.file.fd = fd;
  stream2.file.close_mode = SP_IO_FILE_CLOSE_MODE_AUTO;
  stream2.callbacks.close = sp_io_file_close;
  sp_io_close(&stream2);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 14);
  for (u32 i = 0; i < 14; i++) {
    ASSERT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(io, file_close_autoclose_true) {
  const char* test_content = "autoclose true";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream, test_content, 14);
  ASSERT_TRUE(stream.file.close_mode == SP_IO_FILE_CLOSE_MODE_AUTO);
  sp_io_close(&stream);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 14);
  for (u32 i = 0; i < 14; i++) {
    ASSERT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(io, file_seek_invalid_negative) {
  const char* test_content = "seek test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 9);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&stream, -100, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);
  sp_io_close(&stream);
}

UTEST_F(io, file_write_to_readonly) {
  const char* test_content = "initial";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 7);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_write(&stream, "data", 4);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(io, file_read_from_writeonly) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  char buffer[10] = {0};
  u64 bytes = sp_io_read(&stream, buffer, 10);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(io, file_open_read_write) {
  const char* initial = "initial data";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));
  ASSERT_TRUE(stream.file.fd >= 0);

  u64 written = sp_io_write(&stream, initial, 12);
  ASSERT_EQ(written, 12);

  sp_io_seek(&stream, 0, SP_IO_SEEK_SET);

  char buffer[12] = {0};
  u64 read = sp_io_read(&stream, buffer, 12);
  ASSERT_EQ(read, 12);

  for (u32 i = 0; i < 12; i++) {
    ASSERT_EQ(buffer[i], initial[i]);
  }

  sp_io_close(&stream);
}

UTEST_F(io, memory_seek_cur_forward) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 64; i++) buffer[i] = (u8)i;

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&stream, 5, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 15);

  u8 val;
  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 15);

  sp_io_close(&stream);
}

UTEST_F(io, memory_seek_cur_backward) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 64; i++) buffer[i] = (u8)i;

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  sp_io_seek(&stream, 30, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&stream, -10, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 20);

  u8 val;
  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 20);

  sp_io_close(&stream);
}

UTEST_F(io, memory_seek_cur_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&stream, -20, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, -1);

  pos = sp_io_seek(&stream, 100, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, -1);

  sp_io_close(&stream);
}

UTEST_F(io, memory_sequential_reads) {
  u8 source[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));

  u8 buf1[4], buf2[4], buf3[4], buf4[4];

  ASSERT_EQ(sp_io_read(&stream, buf1, 4), 4);
  ASSERT_EQ(sp_io_read(&stream, buf2, 4), 4);
  ASSERT_EQ(sp_io_read(&stream, buf3, 4), 4);
  ASSERT_EQ(sp_io_read(&stream, buf4, 4), 4);

  ASSERT_EQ(buf1[0], 0);
  ASSERT_EQ(buf2[0], 4);
  ASSERT_EQ(buf3[0], 8);
  ASSERT_EQ(buf4[0], 12);

  sp_io_close(&stream);
}

UTEST_F(io, memory_sequential_writes) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 data1[] = {1,2,3,4};
  u8 data2[] = {5,6,7,8};
  u8 data3[] = {9,10,11,12};

  ASSERT_EQ(sp_io_write(&stream, data1, 4), 4);
  ASSERT_EQ(sp_io_write(&stream, data2, 4), 4);
  ASSERT_EQ(sp_io_write(&stream, data3, 4), 4);

  ASSERT_EQ(buffer[0], 1);
  ASSERT_EQ(buffer[4], 5);
  ASSERT_EQ(buffer[8], 9);

  sp_io_close(&stream);
}

UTEST_F(io, memory_interleaved_operations) {
  u8 buffer[32] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 data[] = {1,2,3,4};
  sp_io_write(&stream, data, 4);

  sp_io_seek(&stream, 0, SP_IO_SEEK_SET);

  u8 read_buf[4];
  sp_io_read(&stream, read_buf, 4);
  ASSERT_EQ(read_buf[0], 1);

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);

  u8 data2[] = {5,6,7,8};
  sp_io_write(&stream, data2, 4);

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);
  sp_io_read(&stream, read_buf, 4);
  ASSERT_EQ(read_buf[0], 5);

  sp_io_close(&stream);
}

UTEST_F(io, memory_size_zero) {
  u8 dummy;
  sp_io_stream_t stream = sp_io_from_memory(&dummy, 0);

  ASSERT_EQ(sp_io_size(&stream), 0);

  u8 buffer[10];
  ASSERT_EQ(sp_io_read(&stream, buffer, 10), 0);
  ASSERT_EQ(sp_io_write(&stream, buffer, 10), 0);

  sp_io_close(&stream);
}

UTEST_F(io, file_read_zero_bytes) {
  const char* content = "test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, content, 4);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  char buffer[10];
  u64 bytes = sp_io_read(&stream, buffer, 0);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(io, memory_read_zero_bytes) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 dest[10];
  u64 bytes = sp_io_read(&stream, dest, 0);
  ASSERT_EQ(bytes, 0);

  sp_io_close(&stream);
}

UTEST_F(io, file_write_zero_bytes) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  u64 bytes = sp_io_write(&stream, "test", 0);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(io, sp_io_read_file_nonexistent) {
  sp_str_t path = SP_LIT("/tmp/sp_io_nonexistent_xyz_12345.txt");
  sp_str_t result = sp_io_read_file(path);

  ASSERT_EQ(result.len, 0);
  ASSERT_EQ(result.data, SP_NULLPTR);
}

UTEST_F(io, sp_io_read_file_empty) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  sp_str_t result = sp_io_read_file(utest_fixture->test_file_path);

  ASSERT_EQ(result.len, 0);
}

UTEST_F(io, file_seek_all_whence_zero_offset) {
  const char* content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);

  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 0);

  sp_io_seek(&stream, 5, SP_IO_SEEK_SET);
  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 5);

  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_END);
  ASSERT_EQ(pos, 10);

  sp_io_close(&stream);
}

UTEST_F(io, memory_position_tracking) {
  u8 buffer[16];
  for (u32 i = 0; i < 16; i++) buffer[i] = (u8)i;

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 val;
  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 0);

  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 1);

  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 1);

  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 2);

  sp_io_close(&stream);
}

UTEST_F(io, file_write_read_roundtrip) {
  const char* data = "roundtrip test data";

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));

  sp_io_write(&stream, data, 19);
  sp_io_seek(&stream, 0, SP_IO_SEEK_SET);

  char buffer[32] = {0};
  sp_io_read(&stream, buffer, 19);

  for (u32 i = 0; i < 19; i++) {
    ASSERT_EQ(buffer[i], data[i]);
  }

  sp_io_close(&stream);
}

