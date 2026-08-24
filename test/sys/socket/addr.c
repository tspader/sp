#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

typedef enum {
  SETUP_BIND,
  SETUP_CONNECT,
} setup_t;

typedef struct {
  const c8* name;
  sp_sys_socket_type_t type;
  setup_t setup;
} test_t;

static const test_t tests [] = {
  { .name = "stream_bind_reports_bound_addr" },
  { .name = "dgram_bind_reports_bound_addr", .type = SP_SYS_SOCKET_DGRAM },
  { .name = "dgram_connect_reports_route_source", .type = SP_SYS_SOCKET_DGRAM, .setup = SETUP_CONNECT },
};

static const u8 loopback [4] = { 127, 0, 0, 1 };

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_socket_t socket = SP_SYS_INVALID_SOCKET;
  sp_must_ok(t, sp_sys_socket_open(&socket, c->type, sp_zero_s(sp_sys_handle_desc_t)));

  switch (c->setup) {
    case SETUP_BIND: {
      sp_must_ok(t, sp_sys_socket_bind(socket, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 } }));
      break;
    }
    case SETUP_CONNECT: {
      sp_must_ok(t, sp_sys_socket_connect(socket, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 }, .port = 9 }));
      break;
    }
  }

  sp_sys_ipv4_t addr = sp_zero;
  sp_must_ok(t, sp_sys_socket_local_addr(socket, &addr));
  sp_expect_mem_eq(t, addr.octets, loopback, 4);
  sp_expect_ne(t, addr.port, (u16)0);

  sp_sys_socket_close(socket);
  return SP_OK;
}

sp_test_each_fn(sys, socket_addr, test_t, tests, run);

#endif
