#include "test.h"
#include "utest.h"

#if !defined(SP_WASM)

UTEST_EMPTY_FIXTURE(sys_socket)

static bool sys_socket_open_listener(sp_sys_socket_t* listener, u16* port) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  if (sp_sys_socket_open(listener) != SP_OK) return false;
  if (sp_sys_socket_bind(*listener, addr) != SP_OK) return false;
  if (sp_sys_socket_listen(*listener, 1) != SP_OK) return false;
  if (sp_sys_socket_local_port(*listener, port) != SP_OK) return false;
  return true;
}

static bool sys_socket_dial(sp_sys_socket_t socket, u16 port) {
  sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
  sp_err_t err = sp_sys_socket_connect(socket, dial);
  if (err == SP_OK) return true;
  if (err != SP_ERR_SYS_WOULD_BLOCK) return false;
  if (sp_sys_socket_wait(socket, false, 1000) != SP_OK) return false;
  return sp_sys_socket_error(socket) == SP_OK;
}

static bool sys_socket_pair(sp_sys_socket_t* listener, sp_sys_socket_t* client, sp_sys_socket_t* server) {
  u16 port = 0;
  if (!sys_socket_open_listener(listener, &port)) return false;
  if (sp_sys_socket_open(client) != SP_OK) return false;
  if (!sys_socket_dial(*client, port)) return false;

  while (true) {
    sp_err_t err = sp_sys_socket_accept(*listener, server);
    if (err == SP_OK) return true;
    if (err != SP_ERR_SYS_WOULD_BLOCK) return false;
    if (sp_sys_socket_wait(*listener, true, 1000) != SP_OK) return false;
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
    struct { u64 request; u64 expect; const c8* content; sp_err_t err; } recv;
    struct { u32 timeout_ms; sp_err_t expect; } wait;
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
          u64 n = 0;
          sp_err_t err = sp_sys_socket_send(client, step->send.data + sent, len - sent, &n);
          if (err == SP_ERR_SYS_WOULD_BLOCK) {
            ASSERT_EQ(sp_sys_socket_wait(client, false, 1000), SP_OK);
            continue;
          }
          ASSERT_EQ(err, SP_OK);
          ASSERT_TRUE(n > 0);
          sent += n;
        }
        break;
      }

      case SYS_SOCKET_STEP_RECV: {
        u8 buf[64] = sp_zero;
        u64 n = 0;
        sp_err_t err = SP_OK;
        while (true) {
          err = sp_sys_socket_recv(server, buf, step->recv.request, &n);
          if (err != SP_ERR_SYS_WOULD_BLOCK) break;
          if (step->recv.err == SP_ERR_SYS_WOULD_BLOCK) break;
          ASSERT_EQ(sp_sys_socket_wait(server, true, 1000), SP_OK);
        }
        EXPECT_EQ(err, step->recv.err);
        if (err == SP_OK) {
          EXPECT_EQ(n, step->recv.expect);
          if (step->recv.content) {
            u64 expect_bytes = sp_cstr_len(step->recv.content);
            sp_for(jt, expect_bytes) EXPECT_EQ((c8)buf[jt], step->recv.content[jt]);
          }
        }
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
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { .request = 32, .expect = 11, .content = "hello world" } },
    },
  });
}

UTEST_F(sys_socket, recv_returns_zero_after_peer_close) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_SEND, .send = { "x" } },
      { .kind = SYS_SOCKET_STEP_CLOSE_CLIENT },
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { .request = 8, .expect = 1, .content = "x" } },
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { .request = 8 } },
    },
  });
}

UTEST_F(sys_socket, wait_readable_ready_after_send) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_SEND, .send = { "x" } },
      { .kind = SYS_SOCKET_STEP_WAIT_READABLE, .wait = { .timeout_ms = 1000 } },
    },
  });
}

UTEST_F(sys_socket, wait_readable_times_out_when_idle) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_WAIT_READABLE, .wait = { .timeout_ms = 50, .expect = SP_ERR_SYS_TIMED_OUT } },
    },
  });
}

UTEST_F(sys_socket, recv_would_block_when_idle) {
  run_sys_socket_test(utest_result, (sys_socket_test_t){
    .steps = {
      { .kind = SYS_SOCKET_STEP_RECV, .recv = { .request = 8, .err = SP_ERR_SYS_WOULD_BLOCK } },
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
  EXPECT_EQ(sp_sys_socket_accept(listener, &accepted), SP_ERR_SYS_WOULD_BLOCK);
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
  ASSERT_EQ(sp_sys_socket_open(&client), SP_OK);

  sp_err_t err = sp_sys_socket_connect(client, dial);
  if (err == SP_ERR_SYS_WOULD_BLOCK) {
    EXPECT_EQ(sp_sys_socket_wait(client, false, 2000), SP_OK);
    EXPECT_EQ(sp_sys_socket_error(client), SP_ERR_SYS_CONN_REFUSED);
  }
  else {
    EXPECT_EQ(err, SP_ERR_SYS_CONN_REFUSED);
  }

  sp_sys_socket_close(client);
}

#endif
