#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define DGRAM_MAX_STEPS 8
#define DGRAM_BUF_SIZE 64

typedef enum {
  STEP_NONE,
  STEP_SEND,
  STEP_RECV,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { const c8* data; } send;
    struct { const c8* content; sp_err_t err; } recv;
  };
} step_t;

typedef struct {
  const c8* name;
  step_t steps [DGRAM_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "send_recv_roundtrip",
    .steps = {
      { .kind = STEP_SEND, .send = { .data = "AB" } },
      { .kind = STEP_RECV, .recv = { .content = "AB" } },
    },
  },
  {
    .name = "preserves_datagram_boundaries",
    .steps = {
      { .kind = STEP_SEND, .send = { .data = "AB" } },
      { .kind = STEP_SEND, .send = { .data = "C" } },
      { .kind = STEP_RECV, .recv = { .content = "AB" } },
      { .kind = STEP_RECV, .recv = { .content = "C" } },
    },
  },
  {
    .name = "recv_would_block_when_idle",
    .steps = {
      { .kind = STEP_RECV, .recv = { .err = SP_ERR_SYS_WOULD_BLOCK } },
    },
  },
};

static bool dgram_pair(sp_sys_socket_t* rx, sp_sys_socket_t* tx) {
  if (sp_sys_socket_open(rx, SP_SYS_SOCKET_DGRAM, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }) != SP_OK) return false;
  if (sp_sys_socket_bind(*rx, (sp_sys_ipv4_t) { .octets = { 127, 0, 0, 1 } }) != SP_OK) return false;
  sp_sys_ipv4_t addr = sp_zero;
  if (sp_sys_socket_local_addr(*rx, &addr) != SP_OK) return false;
  if (sp_sys_socket_open(tx, SP_SYS_SOCKET_DGRAM, (sp_sys_handle_desc_t) { SP_SYS_NONBLOCKING }) != SP_OK) return false;
  return sp_sys_socket_connect(*tx, addr) == SP_OK;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_socket_t rx = SP_SYS_INVALID_SOCKET;
  sp_sys_socket_t tx = SP_SYS_INVALID_SOCKET;
  sp_must(t, dgram_pair(&rx, &tx));

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_SEND: {
        u64 len = sp_cstr_len(step->send.data);
        u64 n = 0;
        sp_must_ok(t, sp_sys_socket_send(tx, step->send.data, len, &n));
        sp_must_eq(t, n, len);
        break;
      }
      case STEP_RECV: {
        c8 buf [DGRAM_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_err_t err = SP_OK;
        while (true) {
          err = sp_sys_socket_recv(rx, buf, DGRAM_BUF_SIZE, &n);
          if (err != SP_ERR_SYS_WOULD_BLOCK) break;
          if (step->recv.err == SP_ERR_SYS_WOULD_BLOCK) break;
          sp_must_ok(t, sp_sys_socket_wait(rx, true, 1000));
        }
        sp_expect_err_eq(t, err, step->recv.err);
        if (!err && !step->recv.err) {
          sp_expect_str_eq_c(t, sp_str(buf, (u32)n), step->recv.content);
        }
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  sp_sys_socket_close(tx);
  sp_sys_socket_close(rx);
  return SP_OK;
}

sp_test_each_fn(sys, socket_dgram, test_t, tests, run);

#endif
