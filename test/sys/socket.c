#include "test.h"
#include "utest.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(sys_socket)

static bool sys_socket_open_listener(sp_sys_socket_t* listener, u16* port) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  if (sp_sys_socket_open(listener) != 0) return false;
  if (sp_sys_socket_bind(*listener, addr) != 0) return false;
  if (sp_sys_socket_listen(*listener, 1) != 0) return false;
  if (sp_sys_socket_local_port(*listener, port) != 0) return false;
  return true;
}

static bool sys_socket_dial(sp_sys_socket_t socket, u16 port) {
  sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
  s32 rc = sp_sys_socket_connect(socket, dial);
  if (rc == 0) return true;
  if (rc != 1) return false;
  if (sp_sys_socket_wait(socket, false, 1000) != 0) return false;
  return sp_sys_socket_error(socket) == 0;
}

static bool sys_socket_pair(sp_sys_socket_t* listener, sp_sys_socket_t* client, sp_sys_socket_t* server) {
  u16 port = 0;
  if (!sys_socket_open_listener(listener, &port)) return false;
  if (sp_sys_socket_open(client) != 0) return false;
  if (!sys_socket_dial(*client, port)) return false;

  while (true) {
    s32 rc = sp_sys_socket_accept(*listener, server);
    if (rc == 0) return true;
    if (rc != 1) return false;
    if (sp_sys_socket_wait(*listener, true, 1000) != 0) return false;
  }
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
    struct { u64 request; s64 expect; const c8* content; } recv;
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
        u64 sent = 0;
        while (sent < len) {
          s64 n = sp_sys_socket_send(client, step->send.data + sent, len - sent);
          if (n == SP_SYS_SOCKET_WOULD_BLOCK) {
            ASSERT_EQ(sp_sys_socket_wait(client, false, 1000), 0);
            continue;
          }
          ASSERT_TRUE(n > 0);
          sent += (u64)n;
        }
        break;
      }

      case SYS_SOCKET_STEP_RECV: {
        u8 buf[64] = sp_zero;
        s64 n = 0;
        while (true) {
          n = sp_sys_socket_recv(server, buf, step->recv.request);
          if (n != SP_SYS_SOCKET_WOULD_BLOCK) break;
          if (step->recv.expect == SP_SYS_SOCKET_WOULD_BLOCK) break;
          ASSERT_EQ(sp_sys_socket_wait(server, true, 1000), 0);
        }
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

UTEST_F(sys_socket, recv_would_block_when_idle) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { 8, SP_SYS_SOCKET_WOULD_BLOCK } },
    },
  });
}

UTEST_F(sys_socket, listen_assigns_and_reports_local_port) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  u16 port = 0;
  ASSERT_TRUE(sys_socket_open_listener(&listener, &port));
  EXPECT_NE(port, (u16)0);
  sp_sys_socket_close(listener);
}

UTEST_F(sys_socket, accept_would_block_when_nobody_connects) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  u16 port = 0;
  ASSERT_TRUE(sys_socket_open_listener(&listener, &port));

  sp_sys_socket_t accepted = SP_SYS_INVALID_SOCKET;
  EXPECT_EQ(sp_sys_socket_accept(listener, &accepted), 1);
  EXPECT_EQ(accepted, SP_SYS_INVALID_SOCKET);

  sp_sys_socket_close(listener);
}

UTEST_F(sys_socket, connect_refused_when_nothing_listens) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  u16 port = 0;
  ASSERT_TRUE(sys_socket_open_listener(&listener, &port));
  sp_sys_socket_close(listener);

  sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  ASSERT_EQ(sp_sys_socket_open(&client), 0);

  s32 rc = sp_sys_socket_connect(client, dial);
  if (rc == 1) {
    EXPECT_EQ(sp_sys_socket_wait(client, false, 2000), 0);
    EXPECT_EQ(sp_sys_socket_error(client), -1);
  }
  else {
    EXPECT_EQ(rc, -1);
  }

  sp_sys_socket_close(client);
}

#endif
