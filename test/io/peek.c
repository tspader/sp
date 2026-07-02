#include "io.h"

UTEST_EMPTY_FIXTURE(io_peek)

typedef enum {
  IO_PEEK_STEP_NONE,
  IO_PEEK_STEP_PEEK,
  IO_PEEK_STEP_FILL,
  IO_PEEK_STEP_CONSUME,
} io_peek_step_kind_t;

typedef struct {
  io_peek_step_kind_t kind;
  union {
    struct { sp_err_t err; const c8* avail; } peek;
    struct { sp_err_t err; u64 len; } fill;
    struct { u64 n; } consume;
  };
} io_peek_step_t;

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  u64 buffer;
  io_peek_step_t steps [IO_MAX_STEPS];
} io_peek_test_t;

void run_io_peek_test(int* utest_result, io_peek_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.results, num_responses);

  u8 wrapper_buf[64] = sp_zero;
  sp_io_reader_set_buffer(&r.base, wrapper_buf, t.buffer ? t.buffer : sizeof(wrapper_buf));

  sp_carr_for(t.steps, j) {
    const io_peek_step_t* step = &t.steps[j];
    if (step->kind == IO_PEEK_STEP_NONE) break;

    switch (step->kind) {
      case IO_PEEK_STEP_NONE: break;
      case IO_PEEK_STEP_PEEK: {
        sp_str_t out = sp_zero;
        sp_err_t err = sp_io_peek(&r.base, &out);
        EXPECT_EQ(err, step->peek.err);
        u64 expect_len = sp_cstr_len(step->peek.avail);
        EXPECT_EQ((u64)out.len, expect_len);
        EXPECT_TRUE(sp_str_equal(out, sp_cstr_as_str(step->peek.avail)));
        break;
      }
      case IO_PEEK_STEP_FILL: {
        sp_err_t err = sp_io_fill(&r.base);
        EXPECT_EQ(err, step->fill.err);
        EXPECT_EQ(r.base.buffer.len, step->fill.len);
        break;
      }
      case IO_PEEK_STEP_CONSUME: {
        sp_io_consume(&r.base, step->consume.n);
        break;
      }
    }
  }
}

// First peek on an empty buffer triggers exactly one backend fill; the
// script has only one entry, so a second call would surface as a failure.
UTEST_F(io_peek, fills_once_then_returns_slice) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 5, .err = SP_OK, .data = "hello" },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "hello" } },
    },
  });
}

// Peek does not consume: repeated peeks against a still-buffered reader
// return the same slice without another backend call.
UTEST_F(io_peek, repeated_peek_is_idempotent) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 5, .err = SP_OK, .data = "hello" },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "hello" } },
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "hello" } },
    },
  });
}

// Consume advances the cursor without copying; a subsequent peek exposes
// only what remains, still without a second backend call.
UTEST_F(io_peek, consume_shrinks_the_peeked_slice) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 5, .err = SP_OK, .data = "hello" },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "hello" } },
      { .kind = IO_PEEK_STEP_CONSUME, .consume = { 2 } },
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "llo" } },
    },
  });
}

// Consuming the whole buffered slice exhausts it; the next peek must
// refill from the backend.
UTEST_F(io_peek, peek_refills_after_full_consume) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK, .data = "abc" },
      { .bytes = 3, .err = SP_OK, .data = "def" },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "abc" } },
      { .kind = IO_PEEK_STEP_CONSUME, .consume = { 3 } },
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "def" } },
    },
  });
}

// fill() is a single, unconditional backend call: it can be driven directly
// without peek, and resets the cursor so the freshly filled bytes are all
// unread.
UTEST_F(io_peek, fill_is_a_single_backend_call) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 4, .err = SP_OK, .data = "abcd" },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_FILL, .fill = { SP_OK, 4 } },
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "abcd" } },
    },
  });
}

// EOF with zero bytes propagates through peek as-is.
UTEST_F(io_peek, peek_propagates_immediate_eof) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_ERR_IO_EOF, "" } },
    },
  });
}

// EOF alongside bytes is normalized to OK, same as sp_io_read: the bytes
// are real and peekable. The next peek, once those bytes are consumed,
// hits the terminal EOF.
UTEST_F(io_peek, peek_normalizes_eof_with_bytes) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 3, .err = SP_ERR_IO_EOF, .data = "abc" },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_OK, "abc" } },
      { .kind = IO_PEEK_STEP_CONSUME, .consume = { 3 } },
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_ERR_IO_EOF, "" } },
    },
  });
}

// A hard error propagates through peek without normalization.
UTEST_F(io_peek, peek_propagates_hard_error) {
  run_io_peek_test(utest_result, (io_peek_test_t){
    .results = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .buffer = 64,
    .steps = {
      { .kind = IO_PEEK_STEP_PEEK, .peek = { SP_ERR_IO_READ_FAILED, "" } },
    },
  });
}
