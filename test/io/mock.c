#include "io.h"

#define IO_MAX_RESPONSES 8
#define IO_MOCK_BUFFER_MAX 256

typedef struct {
  u64 bytes;
  sp_err_t err;
  const c8* data;
} io_mock_response_t;

typedef struct {
  sp_io_reader_t base;
  io_mock_response_t responses[IO_MAX_RESPONSES];
  u64 num_responses;
  u64 cursor;
} io_mock_reader_t;

typedef struct {
  sp_io_writer_t base;
  io_mock_response_t responses[IO_MAX_RESPONSES];
  u64 num_responses;
  u64 cursor;
  u8 received[IO_MOCK_BUFFER_MAX];
  u64 received_len;
} io_mock_writer_t;

static sp_err_t io_mock_reader_read(sp_io_reader_t* r, void* ptr, u64 size, u64* bytes_read) {
  io_mock_reader_t* m = (io_mock_reader_t*)r;
  sp_assert(m->cursor < m->num_responses);
  io_mock_response_t* resp = &m->responses[m->cursor++];
  u64 n = sp_min(size, resp->bytes);
  if (n && resp->data) {
    sp_mem_copy(resp->data, ptr, n);
  }
  if (bytes_read) *bytes_read = n;
  return resp->err;
}

static sp_err_t io_mock_writer_write(sp_io_writer_t* w, const void* ptr, u64 size, u64* bytes_written) {
  io_mock_writer_t* m = (io_mock_writer_t*)w;
  sp_assert(m->cursor < m->num_responses);
  io_mock_response_t* resp = &m->responses[m->cursor++];
  u64 n = sp_min(size, resp->bytes);
  if (n) {
    sp_assert(m->received_len + n <= IO_MOCK_BUFFER_MAX);
    sp_mem_copy(ptr, m->received + m->received_len, n);
    m->received_len += n;
  }
  if (bytes_written) *bytes_written = n;
  return resp->err;
}

static u64 io_mock_response_count(const io_mock_response_t* responses, u64 max) {
  u64 n = 0;
  sp_for(it, max) {
    if (!responses[it].bytes && !responses[it].err && !responses[it].data) break;
    n = it + 1;
  }
  return n;
}

static void io_mock_reader_init(io_mock_reader_t* m, const io_mock_response_t* responses, u64 n) {
  *m = sp_zero_s(io_mock_reader_t);
  m->base.read = io_mock_reader_read;
  m->num_responses = n;
  sp_for(it, n) m->responses[it] = responses[it];
}

static void io_mock_writer_init(io_mock_writer_t* m, const io_mock_response_t* responses, u64 n) {
  *m = sp_zero_s(io_mock_writer_t);
  m->base.write = io_mock_writer_write;
  m->num_responses = n;
  sp_for(it, n) m->responses[it] = responses[it];
}


//////////////////////////
// READ MOCK TEST RUNNER
//////////////////////////
typedef struct {
  io_mock_response_t responses[IO_MAX_RESPONSES];
  io_step_t steps[IO_MAX_STEPS];
} io_mock_read_test_t;

void run_io_mock_read_test(int* utest_result, io_mock_read_test_t t) {
  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.responses, num_responses);

  sp_carr_for(t.steps, j) {
    const io_step_t* step = &t.steps[j];
    if (step->kind == IO_STEP_NONE) break;

    u8 dest[64] = sp_zero;
    u64 bytes = 0;
    sp_err_t err = sp_io_read(&r.base, dest, step->read.request, &bytes);
    EXPECT_EQ(err, step->read.err);

    u64 expect_bytes = sp_cstr_len(step->read.content);
    EXPECT_EQ(bytes, expect_bytes);
    sp_for(it, expect_bytes) {
      EXPECT_EQ((c8)dest[it], step->read.content[it]);
    }
  }

  EXPECT_EQ(r.cursor, r.num_responses);
}


//////////////////////////
// WRITE MOCK TEST RUNNER
//////////////////////////
typedef struct {
  io_mock_response_t responses[IO_MAX_RESPONSES];
  io_step_t steps[IO_MAX_STEPS];
  struct {
    const c8* received;
  } expect;
} io_mock_write_test_t;

void run_io_mock_write_test(int* utest_result, io_mock_write_test_t t) {
  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.responses, num_responses);

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
      case IO_STEP_NONE:
      case IO_STEP_READ: {
        sp_unreachable_case();
      }
    }
  }

  EXPECT_EQ(w.cursor, w.num_responses);

  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}


//////////////////////////
// COPY MOCK TEST RUNNERS
//////////////////////////
typedef struct {
  io_mock_response_t responses[IO_MAX_RESPONSES];
  u64 writer_capacity;
  u64 buffer;
  struct {
    sp_err_t err;
    u64 copied;
    const c8* final;
  } expect;
} io_mock_copy_reader_test_t;

void run_io_mock_copy_reader_test(int* utest_result, io_mock_copy_reader_test_t t) {
  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.responses, num_responses);

  u8 backing[64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, backing, t.writer_capacity);

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r.base, copy_buf, t.buffer, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.final);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.final[it]);
  }
}

typedef struct {
  const c8* source;
  io_mock_response_t responses[IO_MAX_RESPONSES];
  u64 buffer;
  struct {
    sp_err_t err;
    u64 copied;
    const c8* received;
  } expect;
} io_mock_copy_writer_test_t;

void run_io_mock_copy_writer_test(int* utest_result, io_mock_copy_writer_test_t t) {
  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));

  u64 num_responses = io_mock_response_count(t.responses, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.responses, num_responses);

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r, copy_buf, t.buffer, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}


//////////////////////////
// SMOKE TESTS
//////////////////////////
UTEST_F(io, mock_read_smoke) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "abc" } },
    },
  });
}

UTEST_F(io, mock_write_smoke) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 3, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
    },
    .expect.received = "abc",
  });
}


//////////////////////////
// READ FAILURE MODES
//////////////////////////

// Short read with OK, then EOF. Pins that short reads do NOT signal EOF.
UTEST_F(io, mock_read_short_then_eof) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "abc" } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

// EOF returned at offset 0 (empty stream).
UTEST_F(io, mock_read_eof_empty) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_EOF } },
    },
  });
}

// EOF is idempotent: each repeated read returns the same terminal state.
UTEST_F(io, mock_read_eof_idempotent) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_EOF },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
      { .kind = IO_STEP_READ, .read = { 4, SP_ERR_IO_EOF } },
    },
  });
}

// Hard error with zero bytes: not-done -> broken.
UTEST_F(io, mock_read_error_zero_bytes) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}

// Successful read, then a hard error on the next call.
UTEST_F(io, mock_read_error_after_success) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 4, .data = "abcd", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_OK, "abcd" } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}

// Orthogonal contract: backend returns bytes AND error in the same call.
// Caller must see the bytes AND the error.
UTEST_F(io, mock_read_bytes_and_error) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED, "abc" } },
    },
  });
}

// After a hard error, subsequent reads continue to report the error if the
// backend continues to surface it. Pins that the wrapper does NOT latch
// errors itself: it is the backend's job to decide whether to retry.
UTEST_F(io, mock_read_error_then_error) {
  run_io_mock_read_test(utest_result, (io_mock_read_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
      { .kind = IO_STEP_READ, .read = { 8, SP_ERR_IO_READ_FAILED } },
    },
  });
}


//////////////////////////
// WRITE FAILURE MODES
//////////////////////////

// Partial write paired with NO_SPACE. Caller sees both bytes_written>0 and
// the error.
UTEST_F(io, mock_write_partial_no_space) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 4, .err = SP_ERR_IO_NO_SPACE },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcdefgh", SP_ERR_IO_NO_SPACE, 4 } },
    },
    .expect.received = "abcd",
  });
}

// Hard error with zero bytes accepted.
UTEST_F(io, mock_write_error_zero_bytes) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_ERR_IO_WRITE_FAILED, 0 } },
    },
  });
}

// Successful write, then a hard error on the next call.
UTEST_F(io, mock_write_error_after_success) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 3, .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_OK, 3 } },
      { .kind = IO_STEP_WRITE, .write = { "xyz", SP_ERR_IO_WRITE_FAILED, 0 } },
    },
    .expect.received = "abc",
  });
}

// Orthogonal contract for writes: partial bytes AND error in one call.
UTEST_F(io, mock_write_bytes_and_error) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 2, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abc", SP_ERR_IO_WRITE_FAILED, 2 } },
    },
    .expect.received = "ab",
  });
}

// After a NO_SPACE, a smaller write that fits should still succeed: the
// wrapper does not latch the error.
UTEST_F(io, mock_write_smaller_after_overflow) {
  run_io_mock_write_test(utest_result, (io_mock_write_test_t){
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_NO_SPACE },
      { .bytes = 2, .err = SP_OK },
    },
    .steps = {
      { .kind = IO_STEP_WRITE, .write = { "abcd", SP_ERR_IO_NO_SPACE, 0 } },
      { .kind = IO_STEP_WRITE, .write = { "xy", SP_OK, 2 } },
    },
    .expect.received = "xy",
  });
}


//////////////////////////
// COPY FAILURE MODES
//////////////////////////

// Source fails AFTER returning some bytes in a prior call. Copy should
// preserve the bytes that did transfer.
UTEST_F(io, mock_copy_reader_fails_after_success) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_OK },
      { .bytes = 0, .err = SP_ERR_IO_READ_FAILED },
    },
    .writer_capacity = 32, .buffer = 8,
    .expect = { .err = SP_ERR_IO_READ_FAILED, .copied = 3, .final = "abc" },
  });
}

// Source returns bytes AND error in the same call. Per the orthogonal
// contract, copy should still commit those bytes to the destination.
UTEST_F(io, mock_copy_reader_bytes_and_error) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .responses = {
      { .bytes = 3, .data = "abc", .err = SP_ERR_IO_READ_FAILED },
    },
    .writer_capacity = 32, .buffer = 8,
    .expect = { .err = SP_ERR_IO_READ_FAILED, .copied = 3, .final = "abc" },
  });
}

// Writer accepts a partial prefix then signals NO_SPACE on a single call.
// Copy should report the prefix as copied and surface the error.
UTEST_F(io, mock_copy_writer_partial_in_call) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 4, .err = SP_ERR_IO_NO_SPACE },
    },
    .buffer = 8,
    .expect = { .err = SP_ERR_IO_NO_SPACE, .copied = 4, .received = "0123" },
  });
}

// Writer rejects everything on the first call with zero bytes accepted.
// Copy should report copied=0 and the destination should be untouched.
UTEST_F(io, mock_copy_writer_fails_immediately) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 0, .err = SP_ERR_IO_WRITE_FAILED },
    },
    .buffer = 8,
    .expect = { .err = SP_ERR_IO_WRITE_FAILED, .copied = 0, .received = "" },
  });
}
