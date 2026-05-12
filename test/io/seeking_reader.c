#include "io.h"

UTEST_EMPTY_FIXTURE(io_seeking_reader)

typedef struct {
  const c8* source;
  io_step_t steps [IO_MAX_STEPS];
} io_seeking_reader_test_t;

////////////
// RUNNER //
////////////
void run_io_seeking_reader_test(int* utest_result, io_seeking_reader_test_t t) {
  sp_io_reader_t backing = sp_zero;
  sp_io_seeking_reader_t r = sp_zero;
  sp_io_seeking_reader_from_mem(&r, &backing, t.source, sp_cstr_len(t.source));

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    switch (step->kind) {
      case IO_STEP_READ: {
        u8 dest [64] = sp_zero;
        u64 bytes = 0;
        sp_err_t err = sp_io_read(r.reader, dest, step->read.request, &bytes);
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
        sp_err_t err = sp_io_seeking_reader_seek(&r, step->seek.offset, step->seek.whence, &pos);
        EXPECT_EQ(err, step->seek.err);
        EXPECT_EQ(pos, step->seek.pos);
        break;
      }
      case IO_STEP_NONE:
      case IO_STEP_WRITE:
      case IO_STEP_FLUSH:
      case IO_STEP_SIZE:
      case IO_STEP_COPY: {
        sp_unreachable_case();
      }
    }
  }
}


UTEST_F(io_seeking_reader, seek) {
  run_io_seeking_reader_test(utest_result, (io_seeking_reader_test_t){
    .source = "0123456789ABCDEF",
    .steps = {
      { .kind = IO_STEP_SEEK, .seek = { 8, SP_IO_SEEK_SET, SP_OK, 8  } },
      { .kind = IO_STEP_READ, .read = { 1, SP_OK, "8" } },
      { .kind = IO_STEP_SEEK, .seek = { 0, SP_IO_SEEK_END, SP_OK, 16 } },
    },
  });
}

UTEST_F(io_seeking_reader, seek_invalid) {
  run_io_seeking_reader_test(utest_result, (io_seeking_reader_test_t){
    .source = "0123456789",
    .steps = {
      { .kind = IO_STEP_SEEK, .seek = { 100, SP_IO_SEEK_SET, SP_ERR_IO_SEEK_INVALID, -1 } },
      { .kind = IO_STEP_SEEK, .seek = { -10, SP_IO_SEEK_SET, SP_ERR_IO_SEEK_INVALID, -1 } },
    },
  });
}
