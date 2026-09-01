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
  sp_sys_ipv4_t addr = sp_zero;
  sp_must(t, socket_open_listener(&listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }, &addr));
  sp_expect_ne(t, addr.port, (u16)0);

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
        sp_must_ok(t, sp_sys_socket_open(&other, SP_SYS_SOCKET_STREAM, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }));
        sp_expect_err_eq(t, sp_sys_socket_bind(other, addr), step->err);
        sp_sys_socket_close(other);
        break;
      }
      case STEP_CONNECT: {
        sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
        sp_must_ok(t, sp_sys_socket_open(&client, SP_SYS_SOCKET_STREAM, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }));
        sp_err_t err = sp_sys_socket_connect(client, addr);
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

#endif
