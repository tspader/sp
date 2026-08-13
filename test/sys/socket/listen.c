#include "sp/sp_test.h"
#include "sock.h"

#if !defined(SP_WASM)

#if defined(SP_POSIX)
  #include <signal.h>
  #include <sys/socket.h>
  #include <sys/wait.h>
  #include <unistd.h>
#endif

#define LISTEN_MAX_STEPS 4

typedef enum {
  STEP_NONE,
  STEP_CLOSE,
  STEP_ACCEPT,
  STEP_BIND,
  STEP_CONNECT,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  sp_err_t err;
} step_t;

typedef struct {
  const c8* name;
  step_t steps [LISTEN_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "assigns_local_port",
  },
  {
    .name = "accept_would_block_when_nobody_connects",
    .steps = {
      { .kind = STEP_ACCEPT, .err = SP_ERR_SYS_WOULD_BLOCK },
    },
  },
  {
    .name = "bind_reports_addr_in_use",
    .steps = {
      { .kind = STEP_BIND, .err = SP_ERR_SYS_ADDR_IN_USE },
    },
  },
  {
    .name = "connect_refused_when_nothing_listens",
    .steps = {
      { .kind = STEP_CLOSE },
      { .kind = STEP_CONNECT, .err = SP_ERR_SYS_CONN_REFUSED },
    },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  u16 port = 0;
  sp_must(t, socket_open_listener(&listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }, &port));
  sp_expect_ne(t, port, (u16)0);

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_CLOSE: {
        sp_sys_socket_close(listener);
        listener = SP_SYS_INVALID_SOCKET;
        break;
      }
      case STEP_ACCEPT: {
        sp_sys_socket_t accepted = SP_SYS_INVALID_SOCKET;
        sp_expect_err_eq(t, sp_sys_socket_accept(listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }, &accepted), step->err);
        sp_expect_eq(t, accepted, SP_SYS_INVALID_SOCKET);
        break;
      }
      case STEP_BIND: {
        sp_sys_socket_t other = SP_SYS_INVALID_SOCKET;
        sp_must_ok(t, sp_sys_socket_open(&other, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }));
        sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 }, .port = port };
        sp_expect_err_eq(t, sp_sys_socket_bind(other, addr), step->err);
        sp_sys_socket_close(other);
        break;
      }
      case STEP_CONNECT: {
        sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
        sp_must_ok(t, sp_sys_socket_open(&client, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }));
        sp_sys_ipv4_t dial = { .octets = { 127, 0, 0, 1 }, .port = port };
        sp_err_t err = sp_sys_socket_connect(client, dial);
        if (err == SP_ERR_SYS_WOULD_BLOCK) {
          sp_expect_ok(t, sp_sys_socket_wait(client, false, 2000));
          err = sp_sys_socket_error(client);
        }
        sp_expect_err_eq(t, err, step->err);
        sp_sys_socket_close(client);
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  if (listener != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(listener);
  return SP_OK;
}

sp_test_each_fn(sys, socket_listen, test_t, tests, run);

// Imperative: the expected failure mode is an infinite retry loop, so the
// accept needs a sacrificial process and a deadline. The datagram socket is
// made with raw socket() because sp_sys only opens stream sockets.
sp_test(sys, socket_accept_refuses_non_stream) {
#if defined(SP_POSIX)
  s32 raw = socket(AF_INET, SOCK_DGRAM, 0);
  if (raw < 0) return sp_test_skip(t, "no datagram sockets");

  pid_t pid = fork();
  if (pid < 0) {
    close(raw);
    return sp_test_skip(t, "fork not available");
  }
  if (pid == 0) {
    sp_sys_socket_t out = SP_SYS_INVALID_SOCKET;
    sp_sys_handle_desc_t desc = sp_zero;
    _exit(sp_sys_socket_accept((sp_sys_socket_t)raw, desc, &out) == SP_ERR_SYS_UNSUPPORTED ? 0 : 1);
  }

  s32 status = 0;
  sp_err_t result = SP_OK;
  sp_for(it, 100) {
    if (waitpid(pid, &status, WNOHANG) == pid) {
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        sp_test_fail(t, "accept did not return UNSUPPORTED");
      }
      goto done;
    }
    sp_os_sleep_ms(5);
  }

  sp_test_fail(t, "accept spun on a non-stream socket");
  kill(pid, SIGKILL);
  waitpid(pid, &status, 0);

done:
  close(raw);
  return result;
#else
  return sp_test_skip(t, "no raw sockets");
#endif
}

#endif
