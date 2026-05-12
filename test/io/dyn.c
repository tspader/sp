#include "io.h"

typedef struct {
  io_step_t steps [IO_MAX_STEPS];
  struct {
    const c8* content;
  } expect;
} io_dyn_test_t;

////////////
// RUNNER //
////////////
void run_io_dyn_test(int* utest_result, sp_mem_t mem, io_dyn_test_t t) {
  sp_io_dyn_mem_writer_t w;
  sp_io_dyn_mem_writer_init_a(mem, &w);

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
      case IO_STEP_SEEK: {
        s64 pos = 0;
        sp_err_t err = sp_io_dyn_mem_writer_seek(&w, step->seek.offset, step->seek.whence, &pos);
        EXPECT_EQ(err, step->seek.err);
        EXPECT_EQ(pos, step->seek.pos);
        break;
      }
      case IO_STEP_SIZE: {
        u64 size = 0;
        sp_err_t err = sp_io_dyn_mem_writer_size(&w, &size);
        EXPECT_EQ(err, step->size.err);
        EXPECT_EQ(size, step->size.size);
        break;
      }
      case IO_STEP_NONE:
      case IO_STEP_READ:
      case IO_STEP_FLUSH:
      case IO_STEP_COPY:
      case IO_STEP_PAD: {
        sp_unreachable_case();
      }
    }
  }

  if (t.expect.content) {
    sp_str_t str = sp_mem_buffer_as_str(&w.storage);
    u64 n = sp_cstr_len(t.expect.content);
    EXPECT_EQ(str.len, n);
    sp_for(it, n) {
      EXPECT_EQ((c8)str.data[it], t.expect.content[it]);
    }
  }

  sp_io_dyn_mem_writer_close(&w);
}

UTEST_F(io, dyn_write) {
  run_io_dyn_test(utest_result, ut.mem, (io_dyn_test_t){
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_OK, 4 } },
      { .kind = IO_STEP_SIZE,  .size  = { SP_OK, 4 } },
    },
    .expect.content = "abcd",
  });
}

UTEST_F(io, dyn_multiple_writes) {
  run_io_dyn_test(utest_result, ut.mem, (io_dyn_test_t){
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "def", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "ghi", SP_OK, 3 } },
      { .kind = IO_STEP_SIZE,  .size  = { SP_OK, 9 } },
    },
    .expect.content = "abcdefghi",
  });
}

UTEST_F(io, dyn_seek) {
  run_io_dyn_test(utest_result, ut.mem, (io_dyn_test_t){
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "12345678", SP_OK, 8 } },
      { .kind = IO_STEP_SEEK,  .seek  = { 4, SP_IO_SEEK_SET, SP_OK, 4 } },
    },
    .expect.content = "12345678",
  });
}

// Growth past any reasonable initial capacity.
UTEST_F(io, dyn_grows) {
  SKIP_ON_WASM()
  sp_io_dyn_mem_writer_t w;
  sp_io_dyn_mem_writer_init_a(ut.mem, &w);

  u8 data [256];
  sp_for(i, 256) data[i] = (u8)i;

  u64 written = 0;
  EXPECT_EQ(sp_io_write(&w.base, data, 256, &written), SP_OK);
  EXPECT_EQ(written, 256);
  u64 size = 0;
  sp_io_dyn_mem_writer_size(&w, &size);
  EXPECT_EQ(size, 256);

  sp_io_dyn_mem_writer_close(&w);
}
