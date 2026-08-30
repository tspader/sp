#include "sp.h"
#include "sp/sp_test.h"
#include "sp/sp_io.h"

#if defined(SP_TASK_FIBER_SUPPORTED)

#define FIBER_TEST_PORT 4321
#define FIBER_TEST_BUF  64

typedef struct {
  sp_io_sim_t sim;
  sp_io_t io_sched;
  sp_io_t io_server;
  sp_io_t io_client;
  sp_sys_socket_t listener;
  sp_err_t server_err;
  sp_err_t client_err;
  u64 slept;
  c8 echoed [FIBER_TEST_BUF];
  u32 echoed_len;
} fiber_state_t;

static void fiber_echo_server(void* context) {
  fiber_state_t* s = sp_cast(fiber_state_t*, context);
  sp_sys_socket_t conn = SP_SYS_INVALID_SOCKET;
  s->server_err = sp_io_accept(s->io_server, s->listener, &conn);
  if (s->server_err) return;
  while (true) {
    u8 buf [FIBER_TEST_BUF];
    u64 n = 0;
    s->server_err = sp_io_recv(s->io_server, conn, sp_mem_slice(buf, sizeof(buf)), &n);
    if (s->server_err || !n) break;
    s->server_err = sp_io_send_all(s->io_server, conn, sp_mem_slice(buf, n));
    if (s->server_err) break;
  }
  sp_io_close(s->io_server, conn);
}

static void fiber_echo_client(void* context) {
  fiber_state_t* s = sp_cast(fiber_state_t*, context);
  sp_sys_socket_t socket = SP_SYS_INVALID_SOCKET;
  s->client_err = sp_io_connect(s->io_client, (sp_sys_ipv4_t) { .port = FIBER_TEST_PORT }, &socket);
  if (s->client_err) return;

  sp_str_t ping = sp_str_lit("ping");
  s->client_err = sp_io_send_all(s->io_client, socket, sp_mem_slice((u8*)ping.data, ping.len));
  if (!s->client_err) {
    u8 buf [FIBER_TEST_BUF];
    u64 n = 0;
    s->client_err = sp_io_recv(s->io_client, socket, sp_mem_slice(buf, sizeof(buf)), &n);
    if (!s->client_err) {
      sp_mem_copy(s->echoed, buf, n);
      s->echoed_len = (u32)n;
    }
  }
  sp_io_close(s->io_client, socket);
}

static void fiber_sleeper(void* context) {
  fiber_state_t* s = sp_cast(fiber_state_t*, context);
  u64 before = sp_io_now(s->io_sched, SP_IO_CLOCK_AWAKE).ns;
  sp_io_sleep(s->io_sched, sp_tm_ms_to_ns(50));
  s->slept = sp_io_now(s->io_sched, SP_IO_CLOCK_AWAKE).ns - before;
}

sp_test(task, fiber_sim_blocking_style) {
  fiber_state_t* s = sp_alloc_type(sp_test_arena(t), fiber_state_t);
  *s = sp_zero_s(fiber_state_t);
  sp_io_sim_init(&s->sim);
  s->io_sched = sp_io_sim_actor(&s->sim);
  s->io_server = sp_io_sim_actor(&s->sim);
  s->io_client = sp_io_sim_actor(&s->sim);
  s->listener = sp_io_sim_listen(&s->sim, FIBER_TEST_PORT);

  sp_task_fiber_t* f = sp_alloc_type(sp_test_arena(t), sp_task_fiber_t);
  sp_task_sched_t sched = sp_task_fiber_init(f, s->io_sched, sp_test_arena(t));
  sp_task_spawn(sched, fiber_echo_server, s);
  sp_task_spawn(sched, fiber_echo_client, s);
  sp_task_spawn(sched, fiber_sleeper, s);
  sp_task_run(sched);
  sp_task_fiber_deinit(f);

  sp_expect_err_eq(t, s->server_err, SP_OK);
  sp_expect_err_eq(t, s->client_err, SP_OK);
  sp_expect_str_eq_c(t, sp_str(s->echoed, s->echoed_len), "ping");
  sp_expect_ge(t, s->slept, sp_tm_ms_to_ns(50));
  return SP_OK;
}

#endif
