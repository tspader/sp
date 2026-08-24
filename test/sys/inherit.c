#include "sp.h"
#include "sp/sp_test.h"
#include "socket/sock.h"
#include "test/probe.h"

#if !defined(SP_WASM) && !defined(SP_FREESTANDING)

typedef enum {
  KIND_PIPE_R,
  KIND_PIPE_W,
  KIND_SOCK_OPEN,
  KIND_SOCK_ACCEPT,
} kind_t;

typedef struct {
  const c8* name;
  kind_t kind;
  sp_sys_inherited_t inherited;
} test_t;

// The child (test/main.c probe mode) attempts a real op on the handle value,
// which only reaches its process when the desc said INHERITED. The parent
// also observes the shared object, so a wrong exit code can't pass alone:
// pipe_r checks whether the byte it staged was consumed, and pipe_w/sock
// read back the byte the child wrote. Nothing can hang: the probe ops are a
// staged 1-byte read, a 1-byte write into an empty pipe, and a 1-byte send,
// and the parent only reads after the child exited and every other handle to
// the write side is closed, so it sees data or EOF immediately.
static const test_t tests [] = {
  { .name = "pipe_r", .kind = KIND_PIPE_R },
  { .name = "pipe_w", .kind = KIND_PIPE_W },
  { .name = "sock_open", .kind = KIND_SOCK_OPEN },
  { .name = "sock_accept", .kind = KIND_SOCK_ACCEPT },
};

static const c8* inherited_name(s64 value) {
  switch ((sp_sys_inherited_t)value) {
    case SP_SYS_NOT_INHERITED: return "no";
    case SP_SYS_INHERITED:     return "yes";
  }
  return "?";
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  s32 want_exit = c->inherited == SP_SYS_INHERITED ? 0 : 1;
  u64 want_bytes = c->inherited == SP_SYS_INHERITED ? 1 : 0;

  switch (c->kind) {
    case KIND_PIPE_R: {
      sp_sys_pipe_t p = sp_zero;
      sp_must_ok(t, sp_sys_pipe(&p, (sp_sys_pipe_desc_t) { .r = c->inherited }));
      u64 n = 0;
      sp_must_ok(t, sp_sys_write(p.w, "A", 1, &n));
      sp_must_eq(t, n, (u64)1);

      sp_expect_eq(t, spawn_probe(t, "read", (s64)p.r), want_exit);

      u8 ready = 0;
      sp_expect_ok(t, sp_sys_pipe_ready(p.r, &ready));
      sp_expect_eq(t, ready, (u8)(c->inherited == SP_SYS_INHERITED ? 0 : 1));

      sp_sys_close(p.r);
      sp_sys_close(p.w);
      break;
    }
    case KIND_PIPE_W: {
      sp_sys_pipe_t p = sp_zero;
      sp_must_ok(t, sp_sys_pipe(&p, (sp_sys_pipe_desc_t) { .w = c->inherited }));

      sp_expect_eq(t, spawn_probe(t, "write", (s64)p.w), want_exit);

      sp_sys_close(p.w);
      c8 byte = 0;
      u64 n = 0;
      sp_expect_ok(t, sp_sys_read(p.r, &byte, 1, &n));
      sp_expect_eq(t, n, want_bytes);
      if (n == 1) sp_expect_eq(t, byte, 'A');
      sp_sys_close(p.r);
      break;
    }
    case KIND_SOCK_OPEN:
    case KIND_SOCK_ACCEPT: {
      sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
      sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
      sp_sys_socket_t server = SP_SYS_INVALID_SOCKET;
      u16 port = 0;
      sp_must(t, socket_open_listener(&listener, sp_zero_s(sp_sys_handle_desc_t), &port));

      sp_sys_handle_desc_t desc = { .inherited = c->inherited };
      sp_sys_handle_desc_t open_desc = sp_zero;
      sp_sys_handle_desc_t accept_desc = sp_zero;
      if (c->kind == KIND_SOCK_OPEN) open_desc = desc;
      else                           accept_desc = desc;

      sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 }, .port = port };
      sp_must_ok(t, sp_sys_socket_open(&client, open_desc));
      sp_must_ok(t, sp_sys_socket_connect(client, addr));
      sp_must_ok(t, sp_sys_socket_accept(listener, accept_desc, &server));

      sp_sys_socket_t probed = c->kind == KIND_SOCK_OPEN ? client : server;
      sp_sys_socket_t peer = c->kind == KIND_SOCK_OPEN ? server : client;

      sp_expect_eq(t, spawn_probe(t, "send", (s64)probed), want_exit);

      sp_sys_socket_close(probed);
      c8 byte = 0;
      u64 n = 0;
      sp_expect_ok(t, sp_sys_socket_recv(peer, &byte, 1, &n));
      sp_expect_eq(t, n, want_bytes);
      if (n == 1) sp_expect_eq(t, byte, 'A');

      sp_sys_socket_close(peer);
      sp_sys_socket_close(listener);
      break;
    }
  }
  return SP_OK;
}

sp_test_each_fn(sys, inherit, test_t, tests, run, .axes = {
  sp_test_axis_named(test_t, inherited, inherited_name, SP_SYS_NOT_INHERITED, SP_SYS_INHERITED)
});

#endif
