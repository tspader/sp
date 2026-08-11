#ifndef SYS_SOCK_H
#define SYS_SOCK_H

#include "sp.h"

#if !defined(SP_WASM)

static bool socket_open_listener(sp_sys_socket_t* listener, u16* port) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  if (sp_sys_socket_open(listener, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }) != SP_OK) return false;
  if (sp_sys_socket_bind(*listener, addr) != SP_OK) return false;
  if (sp_sys_socket_listen(*listener, 1) != SP_OK) return false;
  if (sp_sys_socket_local_port(*listener, port) != SP_OK) return false;
  return true;
}

#endif

#endif
