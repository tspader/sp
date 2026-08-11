#include "sp/sp_test.h"
#include "sock.h"

#if !defined(SP_WASM)

#define DESC_BUF_SIZE 8
#define DESC_DELAY_MS 50

typedef enum {
  THREAD_NONE,
  THREAD_DIAL,
  THREAD_SEND,
} thread_kind_t;

typedef struct {
  sp_err_t err;
  const c8* content;
} expect_t;

typedef struct {
  const c8* name;
  sp_sys_handle_desc_t listener;
  sp_sys_handle_desc_t accept;
  thread_kind_t thread;
  bool set_nonblocking;
  const c8* send;
  u64 recv;
  expect_t expect;
} test_t;

// Every case is arranged so a mis-applied mode fails fast instead of hanging:
// a wrongly nonblocking handle surfaces WOULD_BLOCK immediately, and a
// wrongly blocking recv is unblocked by the delayed sender.
static const test_t tests [] = {
  {
    .name = "zero_desc_roundtrip",
    .send = "A",
    .recv = DESC_BUF_SIZE,
    .expect = { .content = "A" },
  },
  {
    .name = "blocking_accept_parks_until_connect",
    .thread = THREAD_DIAL,
  },
  {
    .name = "accept_desc_overrides_nonblocking_listener",
    .listener = { SP_SYS_NONBLOCKING },
    .thread = THREAD_SEND,
    .recv = DESC_BUF_SIZE,
    .expect = { .content = "A" },
  },
  {
    .name = "accept_desc_overrides_blocking_listener",
    .accept = { SP_SYS_NONBLOCKING },
    .thread = THREAD_SEND,
    .recv = DESC_BUF_SIZE,
    .expect = { .err = SP_ERR_SYS_WOULD_BLOCK },
  },
  {
    .name = "set_nonblocking_adopts_blocking_socket",
    .thread = THREAD_SEND,
    .set_nonblocking = true,
    .recv = DESC_BUF_SIZE,
    .expect = { .err = SP_ERR_SYS_WOULD_BLOCK },
  },
};

typedef struct {
  u16 port;
  sp_sys_socket_t socket;
  sp_err_t err;
} peer_t;

static s32 dialer(void* userdata) {
  peer_t* peer = (peer_t*)userdata;
  sp_os_sleep_ms(DESC_DELAY_MS);
  peer->err = sp_sys_socket_open(&peer->socket, sp_zero_s(sp_sys_handle_desc_t));
  if (peer->err) return 0;
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 }, .port = peer->port };
  peer->err = sp_sys_socket_connect(peer->socket, addr);
  return 0;
}

static s32 sender(void* userdata) {
  peer_t* peer = (peer_t*)userdata;
  sp_os_sleep_ms(DESC_DELAY_MS);
  u64 n = 0;
  peer->err = sp_sys_socket_send(peer->socket, "A", 1, &n);
  return 0;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
#if defined(SP_FREESTANDING)
  if (c->thread != THREAD_NONE) {
    return sp_test_skip(t, "threads are unsupported on freestanding");
  }
#endif

  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t server = SP_SYS_INVALID_SOCKET;
  u16 port = 0;
  sp_must(t, socket_open_listener(&listener, c->listener, &port));

  peer_t peer = { .port = port };
  sp_thread_t thread = sp_zero;
  bool spawned = false;

  if (c->thread == THREAD_DIAL) {
    sp_thread_init(&thread, dialer, &peer);
    sp_err_t err = sp_sys_socket_accept(listener, c->accept, &server);
    sp_thread_join(&thread);
    client = peer.socket;
    sp_must_ok(t, err);
    sp_must_ok(t, peer.err);
  }
  else {
    sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 }, .port = port };
    sp_must_ok(t, sp_sys_socket_open(&client, sp_zero_s(sp_sys_handle_desc_t)));
    sp_must_ok(t, sp_sys_socket_connect(client, addr));

    if (c->listener.mode == SP_SYS_NONBLOCKING) {
      while (true) {
        sp_err_t err = sp_sys_socket_accept(listener, c->accept, &server);
        if (err != SP_ERR_SYS_WOULD_BLOCK) {
          sp_must_ok(t, err);
          break;
        }
        sp_must_ok(t, sp_sys_socket_wait(listener, true, 1000));
      }
    }
    else {
      sp_must_ok(t, sp_sys_socket_accept(listener, c->accept, &server));
    }
  }

  if (c->send) {
    u64 len = sp_cstr_len(c->send);
    u64 n = 0;
    sp_must_ok(t, sp_sys_socket_send(client, c->send, len, &n));
    sp_expect_eq(t, n, len);
  }

  if (c->set_nonblocking) {
    sp_must_ok(t, sp_sys_socket_set_nonblocking(server));
  }

  if (c->thread == THREAD_SEND) {
    peer.socket = client;
    sp_thread_init(&thread, sender, &peer);
    spawned = true;
  }

  if (c->recv) {
    c8 buf [DESC_BUF_SIZE] = sp_zero;
    u64 n = 0;
    sp_err_t err = sp_sys_socket_recv(server, buf, c->recv, &n);
    sp_expect_err_eq(t, err, c->expect.err);
    if (!err && !c->expect.err) {
      sp_expect_str_eq_c(t, sp_str(buf, (u32)n), c->expect.content);
    }
  }

  if (spawned) {
    sp_thread_join(&thread);
    sp_expect_ok(t, peer.err);
  }
  sp_sys_socket_close(client);
  sp_sys_socket_close(server);
  sp_sys_socket_close(listener);
  return SP_OK;
}

sp_test_each_fn(sys, socket_desc, test_t, tests, run);

#endif
