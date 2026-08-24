#include "io.h"

UTEST_EMPTY_FIXTURE(io_copy)

typedef struct {
  io_result_t results [IO_MAX_RESPONSES];
  struct {
    u64 copy;
    u64 r;
  } buffer;
  struct {
    u64 writer;
  } capacity;
  struct {
    sp_err_t err;
    u64 copied;
    const c8* final;
  } expect;
} io_mock_copy_reader_test_t;

////////////
// RUNNER //
////////////
void run_io_mock_copy_reader_test(int* utest_result, io_mock_copy_reader_test_t t) {
  u64 num_responses = io_get_num_results(t.results, IO_MAX_RESPONSES);
  io_mock_reader_t r = sp_zero;
  io_mock_reader_init(&r, t.results, num_responses);

  u8 reader_buf[64] = sp_zero;
  if (t.buffer.r) {
    sp_io_reader_set_buffer(&r.base, reader_buf, t.buffer.r);
  }

  u8 backing[64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, backing, t.capacity.writer);

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r.base, copy_buf, t.buffer.copy, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.final);
  sp_for(it, n) {
    EXPECT_EQ((c8)backing[it], t.expect.final[it]);
  }
}

typedef struct {
  const c8* source;
  io_result_t responses[IO_MAX_RESPONSES];
  struct {
    u64 copy;
    u64 write;
  } buffer;

  struct {
    sp_err_t err;
    u64 copied;
    const c8* received;
  } expect;
} io_mock_copy_writer_test_t;

void run_io_mock_copy_writer_test(int* utest_result, io_mock_copy_writer_test_t t) {
  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, t.source, sp_cstr_len(t.source));

  u64 num_responses = io_get_num_results(t.responses, IO_MAX_RESPONSES);
  io_mock_writer_t w = sp_zero;
  io_mock_writer_init(&w, t.responses, num_responses);

  u8 writer_buf[64] = sp_zero;
  if (t.buffer.write) {
    sp_io_writer_set_buffer(&w.base, writer_buf, t.buffer.write);
  }

  u8 copy_buf[64] = sp_zero;
  u64 copied = 0;
  sp_err_t err = sp_io_copy_b(&w.base, &r, copy_buf, t.buffer.copy, &copied);

  EXPECT_EQ(err, t.expect.err);
  EXPECT_EQ(copied, t.expect.copied);
  u64 n = sp_cstr_len(t.expect.received);
  EXPECT_EQ(w.received_len, n);
  sp_for(it, n) {
    EXPECT_EQ((c8)w.received[it], t.expect.received[it]);
  }
}


// Source fails AFTER returning some bytes in a prior call. Copy should
// preserve the bytes that did transfer.
UTEST_F(io_copy, reader_fails_after_success) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 3, .err = SP_OK, .data = "abc" },
      { .bytes = 0, .err = SP_ERR_SYS_ACCESS_DENIED },
    },
    .buffer = { .copy = 8 }, .capacity = { .writer = 32 },
    .expect = { .err = SP_ERR_SYS_ACCESS_DENIED, .copied = 3, .final = "abc" },
  });
}

// Source returns bytes AND error in the same call. Per the orthogonal
// contract, copy should still commit those bytes to the destination.
UTEST_F(io_copy, reader_bytes_and_error) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 3, .err = SP_ERR_SYS_ACCESS_DENIED, .data = "abc" },
    },
    .buffer = { .copy = 8 }, .capacity = { .writer = 32 },
    .expect = { .err = SP_ERR_SYS_ACCESS_DENIED, .copied = 3, .final = "abc" },
  });
}

// Writer accepts a partial prefix then signals NO_SPACE on a single call.
// Copy should report the prefix as copied and surface the error.
UTEST_F(io_copy, writer_partial_in_call) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 4, .err = SP_ERR_IO_NO_SPACE },
    },
    .buffer = { .copy = 8 },
    .expect = { .err = SP_ERR_IO_NO_SPACE, .copied = 4, .received = "0123" },
  });
}

// Writer accepts a partial prefix with no error, then takes the rest on a
// later call. Copy must not drop the unaccepted suffix: all source bytes
// reach the destination across the short writes.
UTEST_F(io_copy, writer_short_write_completes) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 4, .err = SP_OK },
      { .bytes = 4, .err = SP_OK },
      { .bytes = 2, .err = SP_OK },
    },
    .buffer = { .copy = 8 },
    .expect = { .err = SP_OK, .copied = 10, .received = "0123456789" },
  });
}

// Writer rejects everything on the first call with zero bytes accepted.
// Copy should report copied=0 and the destination should be untouched.
UTEST_F(io_copy, writer_fails_immediately) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "0123456789",
    .responses = {
      { .bytes = 0, .err = SP_ERR_SYS_ACCESS_DENIED },
    },
    .buffer = { .copy = 8 },
    .expect = { .err = SP_ERR_SYS_ACCESS_DENIED, .copied = 0, .received = "" },
  });
}


//////////////
// BUFFERED //
//////////////

// Reader has a wrapper buffer smaller than the copy buffer. Each backend
// read fills the wrapper buffer; user-facing reads come from there until
// the wrapper buffer drains and the backend is called again. All source
// bytes still reach the destination.
UTEST_F(io_copy, buffered_reader_drains_through_buffer) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 4, .err = SP_OK, .data = "0123" },
      { .bytes = 4, .err = SP_OK, .data = "4567" },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = { .copy = 2, .r = 4 },
    .capacity = { .writer = 32 },
    .expect = { .err = SP_OK, .copied = 8, .final = "01234567" },
  });
}

// Reader wrapper buffer is larger than the copy buffer; one backend read
// fills it and the copy drains it across multiple small reads.
UTEST_F(io_copy, buffered_reader_one_fill_many_drains) {
  run_io_mock_copy_reader_test(utest_result, (io_mock_copy_reader_test_t){
    .results = {
      { .bytes = 8, .err = SP_OK, .data = "ABCDEFGH" },
      { .bytes = 0, .err = SP_ERR_IO_EOF },
    },
    .buffer = { .copy = 2, .r = 8 },
    .capacity = { .writer = 32 },
    .expect = { .err = SP_OK, .copied = 8, .final = "ABCDEFGH" },
  });
}

// Writer wrapper buffer smaller than copy size. As the buffer fills, the
// wrapper flushes to the backend. The tail of the data is left in the
// buffer when copy returns because sp_io_copy_b does not flush at end.
// Pins this behavior explicitly.
UTEST_F(io_copy, buffered_writer_overflow_flushes_partial) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "01234567",
    .responses = {
      { .bytes = 4, .err = SP_OK },
    },
    .buffer = { .copy = 2, .write = 4 },
    .expect = { .err = SP_OK, .copied = 8, .received = "0123" },
  });
}

///////////////////
// FAST PATH     //
///////////////////
// Pin the transfer-on-writer negotiation contract. A tracking writer records
// whether the transfer hook was driven and how many bytes flowed through each
// path. We pair it with a reader that claims as_file (a file reader) or one
// that does not (a mem reader) to exercise the dispatch.

typedef struct {
  sp_io_writer_t base;
  sp_err_t      transfer_then;      // returned once transfer_max_calls succeeded; 0 = never
  u64           transfer_max_calls;
  u64           transfer_calls;
  u64           bytes_via_write;
  u64           bytes_via_transfer;
} io_tracking_writer_t;

static sp_err_t io_tracking_writer_write(sp_io_writer_t* w, const void* ptr, u64 size, u64* bytes_written) {
  io_tracking_writer_t* t = (io_tracking_writer_t*)w;
  (void)ptr;
  t->bytes_via_write += size;
  if (bytes_written) *bytes_written = size;
  return SP_OK;
}

// Single-shot per the transfer contract: one bounded move per call, honoring
// the source cursor; sp_io_copy owns the loop.
static sp_err_t io_tracking_writer_transfer(sp_io_writer_t* w, sp_sys_fd_t fd, u64* pos, u64 count, u64* moved) {
  io_tracking_writer_t* t = (io_tracking_writer_t*)w;
  if (t->transfer_then && t->transfer_calls >= t->transfer_max_calls) {
    if (moved) *moved = 0;
    return t->transfer_then;
  }
  t->transfer_calls++;
  u8 buf [4096];
  u64 n = 0;
  u64 want = sp_min(count, (u64)sizeof(buf));
  sp_err_t err = pos
    ? sp_sys_pread(fd, buf, want, *pos, &n)
    : sp_sys_read(fd, buf, want, &n);
  if (err) {
    if (moved) *moved = 0;
    return err;
  }
  if (pos) *pos += n;
  t->bytes_via_transfer += n;
  if (moved) *moved = n;
  return SP_OK;
}

// Source claims as_file (file reader) and writer advertises transfer. The
// fast path is taken; the byte-loop path is not touched.
UTEST_F(io_copy, fast_path_taken_when_both_sides_support) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_create_empty(&fm, sp_str_lit("fastpath_src.bin"));

  const c8* content = "0123456789abcdef";
  u64 n = sp_cstr_len(content);
  {
    sp_io_file_writer_t fw = sp_zero;
    sp_io_file_writer_from_path(&fw, path);
    sp_io_write(&fw.base, content, n, SP_NULLPTR);
    sp_io_file_writer_close(&fw);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, path);

  io_tracking_writer_t w = sp_zero;
  w.base.write     = io_tracking_writer_write;
  w.base.transfer = io_tracking_writer_transfer;

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &r.base, &copied), SP_OK);
  EXPECT_EQ(copied, n);
  EXPECT_EQ(w.bytes_via_transfer, n);
  EXPECT_EQ(w.bytes_via_write, 0);

  sp_io_file_reader_close(&r);
  sp_test_file_manager_cleanup(&fm);
}

// Source does not claim as_file (mem reader). Fast path declines; we fall
// through to the byte-loop path. The tracking writer's transfer hook is
// never driven.
UTEST_F(io_copy, fast_path_skipped_when_source_is_not_a_file) {
  const c8* content = "the-quick-brown-fox-jumps-over";
  u64 n = sp_cstr_len(content);

  sp_io_reader_t r = sp_zero;
  sp_io_reader_from_mem(&r, content, n);

  io_tracking_writer_t w = sp_zero;
  w.base.write     = io_tracking_writer_write;
  w.base.transfer = io_tracking_writer_transfer;

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &r, &copied), SP_OK);
  EXPECT_EQ(copied, n);
  EXPECT_EQ(w.bytes_via_transfer, 0);
  EXPECT_EQ(w.bytes_via_write, n);
}

// Writer declines the fast path with SP_ERR_SYS_UNSUPPORTED. Copy falls
// through to the byte loop without surfacing the decline to the
// caller. The end-to-end byte count is intact.
UTEST_F(io_copy, fast_path_declined_falls_through) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_create_empty(&fm, sp_str_lit("fastpath_declined.bin"));

  const c8* content = "fallback-content-xyz";
  u64 n = sp_cstr_len(content);
  {
    sp_io_file_writer_t fw = sp_zero;
    sp_io_file_writer_from_path(&fw, path);
    sp_io_write(&fw.base, content, n, SP_NULLPTR);
    sp_io_file_writer_close(&fw);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, path);

  io_tracking_writer_t w = sp_zero;
  w.base.write     = io_tracking_writer_write;
  w.base.transfer = io_tracking_writer_transfer;
  w.transfer_then = SP_ERR_SYS_UNSUPPORTED;

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &r.base, &copied), SP_OK);
  EXPECT_EQ(copied, n);
  EXPECT_EQ(w.bytes_via_transfer, 0);
  EXPECT_EQ(w.bytes_via_write, n);

  sp_io_file_reader_close(&r);
  sp_test_file_manager_cleanup(&fm);
}

// The transfer hook declines mid-stream, after real progress. The claimed
// cursor has already advanced past the transferred prefix, so the generic
// loop must resume exactly there: every byte lands once and only once.
UTEST_F(io_copy, fast_path_mid_stream_decline_resumes_in_fallback) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_create_empty(&fm, sp_str_lit("fastpath_midstream.bin"));

  u8 source [6000];
  sp_for(i, sizeof(source)) source[i] = (u8)((i * 1103515245u + 12345u) >> 8);
  {
    sp_io_file_writer_t fw = sp_zero;
    sp_io_file_writer_from_path(&fw, path);
    EXPECT_EQ(sp_io_write(&fw.base, source, sizeof(source), SP_NULLPTR), SP_OK);
    sp_io_file_writer_close(&fw);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, path);

  io_tracking_writer_t w = sp_zero;
  w.base.write    = io_tracking_writer_write;
  w.base.transfer = io_tracking_writer_transfer;
  w.transfer_max_calls = 1;
  w.transfer_then = SP_ERR_SYS_UNSUPPORTED;

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &r.base, &copied), SP_OK);
  EXPECT_EQ(copied, sizeof(source));
  EXPECT_EQ(w.bytes_via_transfer, 4096);
  EXPECT_EQ(w.bytes_via_write, sizeof(source) - 4096);

  sp_io_file_reader_close(&r);
  sp_test_file_manager_cleanup(&fm);
}

// A real transfer error surfaces to the caller with the fast-path progress
// accounted in bytes_copied; the fallback does not run.
UTEST_F(io_copy, transfer_error_surfaces_with_partial_progress) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_create_empty(&fm, sp_str_lit("fastpath_error.bin"));

  u8 source [6000];
  sp_for(i, sizeof(source)) source[i] = (u8)((i * 2654435761u + 7) >> 8);
  {
    sp_io_file_writer_t fw = sp_zero;
    sp_io_file_writer_from_path(&fw, path);
    EXPECT_EQ(sp_io_write(&fw.base, source, sizeof(source), SP_NULLPTR), SP_OK);
    sp_io_file_writer_close(&fw);
  }

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, path);

  io_tracking_writer_t w = sp_zero;
  w.base.write    = io_tracking_writer_write;
  w.base.transfer = io_tracking_writer_transfer;
  w.transfer_max_calls = 1;
  w.transfer_then = SP_ERR_SYS_ACCESS_DENIED;

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &r.base, &copied), SP_ERR_SYS_ACCESS_DENIED);
  EXPECT_EQ(copied, 4096);
  EXPECT_EQ(w.bytes_via_write, 0);

  sp_io_file_reader_close(&r);
  sp_test_file_manager_cleanup(&fm);
}

// A streaming claim hands the transfer hook a NULL cursor: the kernel's fd
// cursor drives the source and the whole copy rides the fast path.
UTEST_F(io_copy, fast_path_stream_claim_uses_fd_cursor) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_create_empty(&fm, sp_str_lit("fastpath_stream.bin"));

  const c8* content = "stream-claim-content";
  u64 n = sp_cstr_len(content);
  {
    sp_io_file_writer_t fw = sp_zero;
    sp_io_file_writer_from_path(&fw, path);
    sp_io_write(&fw.base, content, n, SP_NULLPTR);
    sp_io_file_writer_close(&fw);
  }

  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  ASSERT_EQ(sp_sys_open_s(sp_sys_get_root(0), path, SP_SYS_OPEN_MODE_RO, 0, &fd), SP_OK);
  sp_io_stream_reader_t sr = sp_zero;
  sp_io_stream_reader_from_file(&sr, fd, SP_IO_CLOSE_MODE_AUTO);

  io_tracking_writer_t w = sp_zero;
  w.base.write    = io_tracking_writer_write;
  w.base.transfer = io_tracking_writer_transfer;

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &sr.base, &copied), SP_OK);
  EXPECT_EQ(copied, n);
  EXPECT_EQ(w.bytes_via_transfer, n);
  EXPECT_EQ(w.bytes_via_write, 0);

  sp_io_stream_reader_close(&sr);
  sp_test_file_manager_cleanup(&fm);
}

// End-to-end: regular file -> pipe via stream_writer. On Linux this exercises
// the sendfile fast path; elsewhere it falls back to the generic loop. The
// bytes that emerge on the pipe's read end must match the source content.
UTEST_F(io_copy, file_to_pipe_sendfile) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_create_empty(&fm, sp_str_lit("sendfile_src.bin"));

  u8 source [4096];
  sp_for(i, sizeof(source)) source[i] = (u8)((i * 2654435761u + 7) >> 8);
  {
    sp_io_file_writer_t fw = sp_zero;
    sp_io_file_writer_from_path(&fw, path);
    EXPECT_EQ(sp_io_write(&fw.base, source, sizeof(source), SP_NULLPTR), SP_OK);
    sp_io_file_writer_close(&fw);
  }

  sp_sys_pipe_t p = sp_zero;
  EXPECT_EQ(sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)), SP_OK);
  sp_sys_fd_t pipe_r = p.r;
  sp_sys_fd_t pipe_w = p.w;

  sp_io_file_reader_t r = sp_zero;
  sp_io_file_reader_from_path(&r, path);

  sp_io_stream_writer_t w = sp_zero;
  sp_io_stream_writer_from_fd(&w, pipe_w, SP_IO_CLOSE_MODE_AUTO);

  u64 copied = 0;
  EXPECT_EQ(sp_io_copy(&w.base, &r.base, &copied), SP_OK);
  EXPECT_EQ(copied, sizeof(source));

  sp_io_file_reader_close(&r);
  sp_io_stream_writer_close(&w);

  sp_io_stream_reader_t sr = sp_zero;
  sp_io_stream_reader_from_fd(&sr, pipe_r, SP_IO_CLOSE_MODE_AUTO);

  u8 received [sizeof(source)] = sp_zero;
  u64 got = 0;
  while (got < sizeof(received)) {
    u64 chunk = 0;
    sp_err_t err = sp_io_read(&sr.base, received + got, sizeof(received) - got, &chunk);
    got += chunk;
    if (err == SP_ERR_IO_EOF) break;
    EXPECT_EQ(err, SP_OK);
  }
  EXPECT_EQ(got, sizeof(source));
  sp_for(i, sizeof(source)) EXPECT_EQ(received[i], source[i]);

  sp_io_stream_reader_close(&sr);
  sp_test_file_manager_cleanup(&fm);
}

// Writer buffer attached; backend errors on the flush that the wrapper
// triggers when the buffer overflows. Copy surfaces the error.
UTEST_F(io_copy, buffered_writer_flush_error) {
  run_io_mock_copy_writer_test(utest_result, (io_mock_copy_writer_test_t){
    .source = "01234567",
    .responses = {
      { .bytes = 0, .err = SP_ERR_SYS_ACCESS_DENIED },
    },
    .buffer = { .copy = 2, .write = 4 },
    .expect = { .err = SP_ERR_SYS_ACCESS_DENIED, .copied = 4, .received = "" },
  });
}
