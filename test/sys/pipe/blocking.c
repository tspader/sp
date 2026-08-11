#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define BLOCKING_DELAY_MS 50
#define BLOCKING_CHUNK 4096
#define BLOCKING_TOTAL (256 * 1024)

typedef enum {
  PEER_NONE,
  PEER_WRITER,
  PEER_DRAINER,
} peer_kind_t;

typedef struct {
  const c8* name;
  peer_kind_t peer;
} test_t;

// A zero desc must produce blocking ends; the delayed peer unblocking a
// parked op is the proof. Both cases fail fast instead of hanging when the
// mode is mis-plumbed: a wrongly nonblocking end surfaces WOULD_BLOCK
// immediately, and a wrongly overlapped Win32 end fails the op outright.
// BLOCKING_TOTAL exceeds the pipe buffer on every platform (64KB, including
// the quota sp_sys_pipe sets on Win32), so the writer must park.
static const test_t tests [] = {
  { .name = "read_parks_until_peer_writes", .peer = PEER_WRITER },
  { .name = "write_parks_until_peer_drains", .peer = PEER_DRAINER },
};

typedef struct {
  sp_sys_fd_t fd;
  sp_err_t err;
  u64 bytes;
} peer_t;

static s32 writer(void* userdata) {
  peer_t* peer = (peer_t*)userdata;
  sp_os_sleep_ms(BLOCKING_DELAY_MS);
  peer->err = sp_sys_write(peer->fd, "A", 1, &peer->bytes);
  return 0;
}

static s32 drainer(void* userdata) {
  peer_t* peer = (peer_t*)userdata;
  sp_os_sleep_ms(BLOCKING_DELAY_MS);
  while (true) {
    u8 buf [BLOCKING_CHUNK];
    u64 n = 0;
    peer->err = sp_sys_read(peer->fd, buf, sizeof(buf), &n);
    if (peer->err || !n) return 0;
    peer->bytes += n;
  }
}

static sp_err_t run(sp_test_t* t, test_t* c) {
#if defined(SP_FREESTANDING)
  return sp_test_skip(t, "threads are unsupported on freestanding");
#else
  sp_sys_pipe_t p = sp_zero;
  sp_must_ok(t, sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)));

  peer_t peer = sp_zero;
  sp_thread_t thread = sp_zero;

  switch (c->peer) {
    case PEER_NONE: break;

    case PEER_WRITER: {
      peer.fd = p.w;
      sp_thread_init(&thread, writer, &peer);

      c8 buf [BLOCKING_CHUNK] = sp_zero;
      u64 n = 0;
      sp_err_t err = sp_sys_read(p.r, buf, sizeof(buf), &n);
      sp_thread_join(&thread);

      sp_expect_ok(t, err);
      sp_expect_ok(t, peer.err);
      if (!err) {
        sp_expect_str_eq_c(t, sp_str(buf, (u32)n), "A");
      }
      break;
    }
    case PEER_DRAINER: {
      peer.fd = p.r;
      sp_thread_init(&thread, drainer, &peer);

      u8 chunk [BLOCKING_CHUNK] = sp_zero;
      u64 total = 0;
      sp_err_t err = SP_OK;
      while (!err && total < BLOCKING_TOTAL) {
        u64 n = 0;
        err = sp_sys_write(p.w, chunk, sp_min(sizeof(chunk), BLOCKING_TOTAL - total), &n);
        if (!err) total += n;
      }
      sp_sys_close(p.w);
      p.w = SP_SYS_INVALID_FD;
      sp_thread_join(&thread);

      sp_expect_ok(t, err);
      sp_expect_ok(t, peer.err);
      sp_expect_eq(t, total, (u64)BLOCKING_TOTAL);
      sp_expect_eq(t, peer.bytes, total);
      break;
    }
  }

  sp_sys_close(p.r);
  if (p.w != SP_SYS_INVALID_FD) sp_sys_close(p.w);
  return SP_OK;
#endif
}

sp_test_each_fn(sys, pipe_blocking, test_t, tests, run);

#endif
