#include "io.h"

////////////
// READER //
////////////
typedef struct {
  const c8* content;
  u64 buffer;
  io_step_t steps [IO_MAX_STEPS];
} io_file_reader_test_t;

void run_io_file_reader_test(int* utest_result, sp_str_t path, io_file_reader_test_t t) {
  {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, path, SP_IO_WRITE_MODE_OVERWRITE);
    if (t.content) sp_io_write(&w.base, t.content, sp_cstr_len(t.content), SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, path);

  u8 wrapper_buf [64] = sp_zero;
  if (t.buffer) sp_io_reader_set_buffer(&r.base, wrapper_buf, t.buffer);

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    switch (step->kind) {
      case IO_STEP_READ: {
        u8 dest [64] = sp_zero;
        u64 bytes = 0;
        sp_err_t err = sp_io_read(&r.base, dest, step->read.request, &bytes);
        EXPECT_EQ(err, step->read.err);
        u64 expect_bytes = sp_cstr_len(step->read.content);
        EXPECT_EQ(bytes, expect_bytes);
        sp_for(it, expect_bytes) {
          EXPECT_EQ((c8)dest[it], step->read.content[it]);
        }
        break;
      }
      case IO_STEP_SEEK: {
        s64 pos = 0;
        sp_err_t err = sp_io_file_reader_seek(&r, step->seek.offset, step->seek.whence, &pos);
        EXPECT_EQ(err, step->seek.err);
        EXPECT_EQ(pos, step->seek.pos);
        break;
      }
      case IO_STEP_NONE:
      case IO_STEP_WRITE:
      case IO_STEP_FLUSH:
      case IO_STEP_SIZE:
      case IO_STEP_COPY:
      case IO_STEP_PAD: {
        sp_unreachable_case();
      }
    }
  }

  sp_io_file_reader_close(&r);
}

UTEST_F(io, file_reader_read_full) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "0123456789ABCDEF",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_OK, "0123456789ABCDEF" } },
    },
  });
}

UTEST_F(io, file_reader_eof_after_drain) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "0123456789ABCDEF",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_OK, "0123456789ABCDEF" } },
      { .kind = IO_STEP_READ, .read = { 16, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, file_reader_eof_short) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "short",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 32, SP_OK, "short" } },
      { .kind = IO_STEP_READ, .read = { 32, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, file_reader_eof_empty) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io, file_reader_seek) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "0123456789",
    .steps = {
      { .kind = IO_STEP_SEEK, .seek = { 5, SP_IO_SEEK_SET, SP_OK, 5 } },
      { .kind = IO_STEP_READ, .read = { 5, SP_OK, "56789" } },
    },
  });
}

UTEST_F(io, file_reader_buffered_read) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "0123456789ABCDEF",
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_OK, "0123456789ABCDEF" } },
    },
  });
}

UTEST_F(io, file_reader_buffered_seek_discards_buffer) {
  run_io_file_reader_test(utest_result, ut.file_path, (io_file_reader_test_t){
    .content = "0123456789ABCDEF",
    .buffer = 8,
    .steps = {
      { .kind = IO_STEP_READ, .read = { 1,  SP_OK, "0" } },
      { .kind = IO_STEP_SEEK, .seek = { 10, SP_IO_SEEK_SET, SP_OK, 10 } },
      { .kind = IO_STEP_READ, .read = { 1,  SP_OK, "A" } },
    },
  });
}

UTEST_F(io, file_reader_nonexistent) {
  SKIP_ON_WASM()
  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("nonexistent.file"));
  sp_io_file_reader_t r = sp_zero;
  EXPECT_EQ(sp_io_file_reader_from_path(&r, path), SP_ERR_IO_OPEN_FAILED);
  sp_io_file_reader_close(&r);
}


////////////
// WRITER //
////////////
typedef struct {
  const c8* pre_content;
  sp_io_write_mode_t mode;
  u64 buffer;
  io_step_t steps [IO_MAX_STEPS];
  struct {
    const c8* content;
  } expect;
} io_file_writer_test_t;

void run_io_file_writer_test(int* utest_result, sp_mem_t mem, sp_str_t path, io_file_writer_test_t t) {
  if (t.pre_content) {
    sp_io_file_writer_t w = sp_zero;
    sp_io_file_writer_from_path(&w, path, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write(&w.base, t.pre_content, sp_cstr_len(t.pre_content), SP_NULLPTR);
    sp_io_file_writer_close(&w);
  }

  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, path, t.mode);

  u8 wrapper_buf [64] = sp_zero;
  if (t.buffer) sp_io_writer_set_buffer(&w.base, wrapper_buf, t.buffer);

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    switch (step->kind) {
      case IO_STEP_WRITE: {
        u64 bytes = 0;
        sp_err_t err = sp_io_write(&w.base, step->write.data, sp_cstr_len(step->write.data), &bytes);
        EXPECT_EQ(err, step->write.err);
        EXPECT_EQ(bytes, step->write.bytes);
        break;
      }
      case IO_STEP_FLUSH: {
        sp_err_t err = sp_io_flush(&w.base);
        EXPECT_EQ(err, step->flush.err);
        break;
      }
      case IO_STEP_SEEK: {
        s64 pos = 0;
        sp_err_t err = sp_io_file_writer_seek(&w, step->seek.offset, step->seek.whence, &pos);
        EXPECT_EQ(err, step->seek.err);
        EXPECT_EQ(pos, step->seek.pos);
        break;
      }
      case IO_STEP_SIZE: {
        u64 size = 0;
        sp_err_t err = sp_io_file_writer_size(&w, &size);
        EXPECT_EQ(err, step->size.err);
        EXPECT_EQ(size, step->size.size);
        break;
      }
      case IO_STEP_NONE:
      case IO_STEP_READ:
      case IO_STEP_COPY:
      case IO_STEP_PAD: {
        sp_unreachable_case();
      }
    }
  }

  sp_io_file_writer_close(&w);

  if (t.expect.content) {
    sp_str_t loaded = sp_zero;
    sp_io_read_file_a(mem, path, &loaded);
    u64 n = sp_cstr_len(t.expect.content);
    EXPECT_EQ(loaded.len, n);
    sp_for(it, n) {
      EXPECT_EQ((c8)loaded.data[it], t.expect.content[it]);
    }
  }
}

UTEST_F(io, file_writer_write) {
  run_io_file_writer_test(utest_result, ut.mem, ut.file_path, (io_file_writer_test_t){
    .mode = SP_IO_WRITE_MODE_OVERWRITE,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "test data", SP_OK, 9 } },
    },
    .expect.content = "test data",
  });
}

UTEST_F(io, file_writer_overwrite) {
  run_io_file_writer_test(utest_result, ut.mem, ut.file_path, (io_file_writer_test_t){
    .pre_content = "XXXXXXXX",
    .mode = SP_IO_WRITE_MODE_OVERWRITE,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "1234", SP_OK, 4 } },
    },
    .expect.content = "1234",
  });
}

UTEST_F(io, file_writer_append) {
  run_io_file_writer_test(utest_result, ut.mem, ut.file_path, (io_file_writer_test_t){
    .pre_content = "first",
    .mode = SP_IO_WRITE_MODE_APPEND,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "second", SP_OK, 6 } },
    },
    .expect.content = "firstsecond",
  });
}

UTEST_F(io, file_writer_size) {
  run_io_file_writer_test(utest_result, ut.mem, ut.file_path, (io_file_writer_test_t){
    .mode = SP_IO_WRITE_MODE_OVERWRITE,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "0123456789ABCDEF", SP_OK, 16 } },
      { .kind = IO_STEP_SIZE,  .size = { SP_OK, 16 } },
    },
    .expect.content = "0123456789ABCDEF",
  });
}

UTEST_F(io, file_writer_buffered_implicit_flush) {
  run_io_file_writer_test(utest_result, ut.mem, ut.file_path, (io_file_writer_test_t){
    .mode = SP_IO_WRITE_MODE_OVERWRITE,
    .buffer = 64,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "hello", SP_OK, 5 } },
    },
    .expect.content = "hello",
  });
}

UTEST_F(io, file_writer_buffered_larger_than_buffer) {
  run_io_file_writer_test(utest_result, ut.mem, ut.file_path, (io_file_writer_test_t){
    .mode = SP_IO_WRITE_MODE_OVERWRITE,
    .buffer = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "0123456789ABCDEF", SP_OK, 16 } },
    },
    .expect.content = "0123456789ABCDEF",
  });
}


//////////////
// SPECIALS //
//////////////

// sp_io_pad writes a sequence of zero bytes; the declarative runner can't
// express the resulting content (c-string verification stops at the first
// nul). Kept imperative.
UTEST_F(io, file_writer_pad) {
  SKIP_ON_WASM()
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_writer_from_path(&w, ut.file_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write(&w.base, "AA", 2, SP_NULLPTR);
  sp_io_pad(&w.base, 3, SP_NULLPTR);
  sp_io_write(&w.base, "BB", 2, SP_NULLPTR);
  sp_io_file_writer_close(&w);

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, ut.file_path);
  u8 result [7] = sp_zero;
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

// Two file handles (writer then reader) operating on the same large offset;
// doesn't fit the single-subject runner pattern.
UTEST_F(io, file_seek_beyond_4gb) {
  SKIP_ON_WASM()
  s64 offset = (s64)5 * 1024 * 1024 * 1024;
  u8 marker [4] = {0xDE, 0xAD, 0xBE, 0xEF};

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

  u8 result [4] = sp_zero;
  u64 bytes = 0;
  EXPECT_EQ(sp_io_read(&r.base, result, 4, &bytes), SP_OK);
  EXPECT_EQ(bytes, 4);
  EXPECT_EQ(result[0], 0xDE);
  EXPECT_EQ(result[1], 0xAD);
  EXPECT_EQ(result[2], 0xBE);
  EXPECT_EQ(result[3], 0xEF);
  sp_io_file_reader_close(&r);
}
