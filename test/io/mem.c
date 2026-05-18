#include "io.h"

UTEST_EMPTY_FIXTURE(io_mem)

typedef struct {
  const c8* source;
  u64 writer_capacity;
  io_step_t steps [IO_MAX_STEPS];
  struct {
    const c8* content;
  } expect;
} io_mem_test_t;

////////////
// RUNNER //
////////////
void run_io_mem_test(int* utest_result, io_mem_test_t t) {
  sp_io_reader_t r = sp_zero;
  if (t.source) {
    sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));
  }

  u8 backing [64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  if (t.writer_capacity) {
    sp_io_mem_writer_from_buffer(&w, backing, t.writer_capacity);
  }

  u8 copy_buf [64] = sp_zero;

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    switch (step->kind) {
      case IO_STEP_READ: {
        u8 dest [64] = sp_zero;
        u64 bytes = 0;
        sp_err_t err = sp_io_read(&r, dest, step->read.request, &bytes);
        EXPECT_EQ(err, step->read.err);
        u64 expect_bytes = sp_cstr_len(step->read.content);
        EXPECT_EQ(bytes, expect_bytes);
        sp_for(it, expect_bytes) {
          EXPECT_EQ((c8)dest[it], step->read.content[it]);
        }
        break;
      }
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
        sp_err_t err = sp_io_mem_writer_seek(&w, step->seek.offset, step->seek.whence, &pos);
        EXPECT_EQ(err, step->seek.err);
        EXPECT_EQ(pos, step->seek.pos);
        break;
      }
      case IO_STEP_SIZE: {
        u64 size = 0;
        sp_err_t err = sp_io_mem_writer_size(&w, &size);
        EXPECT_EQ(err, step->size.err);
        EXPECT_EQ(size, step->size.size);
        break;
      }
      case IO_STEP_COPY: {
        u64 copied = 0;
        sp_err_t err = sp_io_copy_b(&w.base, &r, copy_buf, step->copy.buffer, &copied);
        EXPECT_EQ(err, step->copy.err);
        EXPECT_EQ(copied, step->copy.copied);
        break;
      }
      case IO_STEP_NONE:
      case IO_STEP_PAD: {
        sp_unreachable_case();
      }
    }
  }

  u64 n = sp_cstr_len(t.expect.content);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.content[it]);
  }
}


//////////
// READ //
//////////
UTEST_F(io_mem, read_exact_then_eof) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "0123",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_OK, "0123" } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io_mem, read_chunked) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "01234567",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "012" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "345" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "67" } },
      { .kind = IO_STEP_READ, .read = { 3, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io_mem, read_oversized_request) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "abc",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 16, SP_OK, "abc" } },
      { .kind = IO_STEP_READ, .read = { 16, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io_mem, read_empty_source) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io_mem, read_eof_idempotent) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "x",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_OK, "x" } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

UTEST_F(io_mem, read_zero_request) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "abc",
    .steps = {
      { .kind = IO_STEP_READ, .read = { 0, SP_OK } },
      { .kind = IO_STEP_READ, .read = { 3, SP_OK, "abc" } },
    },
  });
}


///////////
// WRITE //
///////////
UTEST_F(io_mem, write_fits) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 16,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "hello", SP_OK, 5 } },
    },
    .expect = { .content = "hello" },
  });
}

UTEST_F(io_mem, write_exact_fit) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
    },
    .expect = { .content = "abcd" },
  });
}

UTEST_F(io_mem, write_overflow) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect = { .content = "abcd" },
  });
}

UTEST_F(io_mem, write_barely_overflows) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcde", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect = { .content = "abcd" },
  });
}

UTEST_F(io_mem, write_smaller_after_overflow) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 4,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
      { .kind = IO_STEP_WRITE, .write = { "x", SP_ERR_IO_NO_SPACE, 0 } },
    },
    .expect = { .content = "abcd" },
  });
}

UTEST_F(io_mem, write_appends) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 16,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "def", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "ghi", SP_OK, 3 } },
    },
    .expect = { .content = "abcdefghi" },
  });
}

UTEST_F(io_mem, write_zero) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 8,
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "", SP_OK, 0 } },
    },
  });
}

UTEST_F(io_mem, write_flush_empty) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 8,
    .steps = {
      { .kind = IO_STEP_FLUSH, .flush = { SP_OK } },
    },
  });
}


//////////
// COPY //
//////////
UTEST_F(io_mem, copy_full) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "0123456789",
    .writer_capacity = 32,
    .steps = {
      { .kind = IO_STEP_COPY, .copy = { 8, SP_OK, 10 } },
    },
    .expect = { .content = "0123456789" },
  });
}

UTEST_F(io_mem, copy_loops) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "ABCDEFGHIJKLMNOP",
    .writer_capacity = 32,
    .steps = {
      { .kind = IO_STEP_COPY, .copy = { 4, SP_OK, 16 } },
    },
    .expect = { .content = "ABCDEFGHIJKLMNOP" },
  });
}

UTEST_F(io_mem, copy_empty) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "",
    .writer_capacity = 8,
    .steps = {
      { .kind = IO_STEP_COPY, .copy = { 8, SP_OK, 0 } },
    },
  });
}

UTEST_F(io_mem, copy_writer_no_space) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .source = "0123456789",
    .writer_capacity = 4,
    .steps = {
      { .kind = IO_STEP_COPY, .copy = { 8, SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect = { .content = "0123" },
  });
}


///////////////
// SEEK/SIZE //
///////////////
UTEST_F(io_mem, writer_seek) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 64,
    .steps = {
      { .kind = IO_STEP_SEEK, .seek = { 32, SP_IO_SEEK_SET, SP_OK, 32 } },
      { .kind = IO_STEP_SEEK, .seek = { 0,  SP_IO_SEEK_END, SP_OK, 64 } },
    },
  });
}

UTEST_F(io_mem, writer_size) {
  run_io_mem_test(utest_result, (io_mem_test_t){
    .writer_capacity = 128,
    .steps = {
      { .kind = IO_STEP_SIZE, .size = { SP_OK, 128 } },
    },
  });
}
