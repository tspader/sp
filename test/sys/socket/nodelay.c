#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

typedef enum {
  TARGET_FRESH,
  TARGET_ACCEPTED,
} target_t;

typedef struct {
  const c8* name;
  target_t  target;
} test_t;

static const test_t tests [] = {
  { .name = "fresh_socket", .target = TARGET_FRESH },
  { .name = "accepted_socket", .target = TARGET_ACCEPTED },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_socket_t listener = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t client = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t accepted = SP_SYS_INVALID_SOCKET;
  sp_must_ok(t, sp_sys_socket_open(&client, sp_zero_s(sp_sys_handle_desc_t)));

  switch (c->target) {
    case TARGET_FRESH: {
      sp_expect_ok(t, sp_sys_socket_no_delay(client));
      break;
    }
    case TARGET_ACCEPTED: {
      sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
      sp_must_ok(t, sp_sys_socket_open(&listener, sp_zero_s(sp_sys_handle_desc_t)));
      sp_must_ok(t, sp_sys_socket_bind(listener, addr));
      sp_must_ok(t, sp_sys_socket_listen(listener, 1));
      sp_must_ok(t, sp_sys_socket_local_port(listener, &addr.port));
      sp_must_ok(t, sp_sys_socket_connect(client, addr));
      sp_must_ok(t, sp_sys_socket_accept(listener, sp_zero_s(sp_sys_handle_desc_t), &accepted));
      sp_expect_ok(t, sp_sys_socket_no_delay(accepted));
      break;
    }
  }

  if (accepted != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(accepted);
  if (listener != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(listener);
  sp_sys_socket_close(client);
  return SP_OK;
}

sp_test_each_fn(sys, socket_nodelay, test_t, tests, run);

#endif
