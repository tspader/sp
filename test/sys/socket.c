#include "test.h"
#include "utest.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(sys_socket)

static bool sys_socket_pair(sp_sys_socket_t* listener, sp_sys_socket_t* client, sp_sys_socket_t* server) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  if (sp_sys_socket_listen(addr, 1, listener) != 0) return false;

  u16 port = 0;
  if (sp_sys_socket_local_port(*listener, &port) != 0) return false;

  sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
  if (sp_sys_socket_connect(dial, 1000, client) != 0) return false;
  if (sp_sys_socket_accept(*listener, 1000, server) != 0) return false;
  return true;
}

typedef enum {
  SYS_SOCKET_STEP_NONE,
  SYS_SOCKET_STEP_SEND,
  SYS_SOCKET_STEP_RECV,
  SYS_SOCKET_STEP_WAIT_READABLE,
  SYS_SOCKET_STEP_CLOSE_CLIENT,
} sys_socket_step_kind_t;

typedef struct {
  sys_socket_step_kind_t kind;
  union {
    struct { const c8* data; } send;
    struct { u64 request; s64 expect; const c8* content; u32 timeout_ms; } recv;
    struct { u32 timeout_ms; s32 expect; } wait;
  };
} sys_socket_step_t;

typedef struct {
  sys_socket_step_t steps [8];
} sys_socket_test_t;

void run_sys_socket_test(int* utest_result, sys_socket_test_t t) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t server = SP_SYS_INVALID_SOCKET;
  ASSERT_TRUE(sys_socket_pair(&listener, &client, &server));

  sp_carr_for(t.steps, it) {
    const sys_socket_step_t* step = &t.steps[it];
    if (step->kind == SYS_SOCKET_STEP_NONE) break;

    switch (step->kind) {
      case SYS_SOCKET_STEP_NONE: break;

      case SYS_SOCKET_STEP_SEND: {
        u64 len = sp_cstr_len(step->send.data);
        EXPECT_EQ(sp_sys_socket_send(client, step->send.data, len, 1000), (s64)len);
        break;
      }

      case SYS_SOCKET_STEP_RECV: {
        u8 buf[64] = sp_zero;
        s64 n = sp_sys_socket_recv(server, buf, step->recv.request, step->recv.timeout_ms);
        EXPECT_EQ(n, step->recv.expect);
        u64 expect_bytes = sp_cstr_len(step->recv.content);
        sp_for(jt, expect_bytes) EXPECT_EQ((c8)buf[jt], step->recv.content[jt]);
        break;
      }

      case SYS_SOCKET_STEP_WAIT_READABLE: {
        EXPECT_EQ(sp_sys_socket_wait(server, true, step->wait.timeout_ms), step->wait.expect);
        break;
      }

      case SYS_SOCKET_STEP_CLOSE_CLIENT: {
        sp_sys_socket_close(client);
        client = SP_SYS_INVALID_SOCKET;
        break;
      }
    }
  }

  if (client != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(client);
  sp_sys_socket_close(server);
  sp_sys_socket_close(listener);
}

UTEST_F(sys_socket, send_recv_roundtrip) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_SEND, .send = { "hello world" } },
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { 32, 11, "hello world" } },
    },
  });
}

UTEST_F(sys_socket, recv_returns_zero_after_peer_close) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_SEND, .send = { "x" } },
      { .kind = SYS_SOCKET_STEP_CLOSE_CLIENT },
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { 8, 1, "x" } },
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { 8, 0 } },
    },
  });
}

UTEST_F(sys_socket, wait_readable_ready_after_send) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_SEND, .send = { "x" } },
      { .kind = SYS_SOCKET_STEP_WAIT_READABLE, .wait = { 1000, 0 } },
    },
  });
}

UTEST_F(sys_socket, wait_readable_times_out_when_idle) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_WAIT_READABLE, .wait = { 50, 1 } },
    },
  });
}

UTEST_F(sys_socket, recv_times_out_when_idle) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { 8, SP_SYS_SOCKET_TIMEOUT, SP_NULLPTR, 50 } },
    },
  });
}

UTEST_F(sys_socket, listen_assigns_and_reports_local_port) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  ASSERT_EQ(sp_sys_socket_listen(addr, 1, &listener), 0);

  u16 port = 0;
  EXPECT_EQ(sp_sys_socket_local_port(listener, &port), 0);
  EXPECT_NE(port, (u16)0);

  sp_sys_socket_close(listener);
}

UTEST_F(sys_socket, accept_times_out_when_nobody_connects) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  ASSERT_EQ(sp_sys_socket_listen(addr, 1, &listener), 0);

  sp_sys_socket_t accepted = SP_SYS_INVALID_SOCKET;
  EXPECT_EQ(sp_sys_socket_accept(listener, 50, &accepted), 1);
  EXPECT_EQ(accepted, SP_SYS_INVALID_SOCKET);

  sp_sys_socket_close(listener);
}

UTEST_F(sys_socket, connect_refused_when_nothing_listens) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  ASSERT_EQ(sp_sys_socket_listen(addr, 1, &listener), 0);

  u16 port = 0;
  ASSERT_EQ(sp_sys_socket_local_port(listener, &port), 0);
  sp_sys_socket_close(listener);

  sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  EXPECT_EQ(sp_sys_socket_connect(dial, 2000, &client), -1);
  EXPECT_EQ(client, SP_SYS_INVALID_SOCKET);
}

#endif
