#ifndef IO_OPS_HARNESS_H
#define IO_OPS_HARNESS_H

#include "sp.h"
#include "sp/sp_test.h"
#include "sp/sp_io.h"

#define OPS_MAX_SLOTS 16
#define OPS_MAX_DONE  4
#define OPS_BUF_SIZE  64
#define OPS_ENTRIES   8
#define OPS_WAIT_MS   2000

typedef enum {
  H_NONE,
  H_LISTENER,
  H_SERVER,
  H_CLIENT,
  H_ACCEPTED,
  H_PIPE_R,
  H_PIPE_W,
} ops_handle_t;

typedef enum {
  FIXTURE_NONE,
  FIXTURE_LISTENER,
  FIXTURE_SOCKETS,
  FIXTURE_PIPE,
} ops_fixture_t;

typedef struct {
  u32 slot;
  sp_io_op_kind_t op;
  ops_handle_t handle;
  u64 size;
  const c8* data;
  u32 ms;
} ops_submit_t;

typedef struct {
  u32 slot;
  sp_err_t err;
  u64 len;
  const c8* content;
  bool partial;
} ops_done_t;

typedef struct {
  u32 count;
  ops_done_t done [OPS_MAX_DONE];
  u32 min_elapsed_ms;
  bool after_deadline;
} ops_wait_expect_t;

typedef enum {
  WAIT_BOUNDED,
  WAIT_NONE,
  WAIT_AFTER,
  WAIT_AT,
} ops_wait_kind_t;

typedef struct {
  ops_wait_kind_t kind;
  s64 ms;
  sp_io_clock_t clock;
  ops_wait_expect_t expect;
} ops_wait_t;

typedef struct {
  sp_io_uring_t ring;
  sp_io_t io;
  sp_sys_socket_t listener;
  sp_sys_socket_t server;
  sp_sys_socket_t client;
  sp_sys_socket_t accepted;
  sp_sys_pipe_t pipe;
  sp_io_time_t deadline;
  sp_io_op_t ops [OPS_MAX_SLOTS];
  sp_mem_slice_t bufs [OPS_MAX_SLOTS];
  u8 storage [OPS_MAX_SLOTS][OPS_BUF_SIZE];
  sp_io_op_t* done [OPS_MAX_SLOTS];
  u32 done_count;
} ops_harness_t;

static void harness_on_done(sp_io_t io, sp_io_op_t* op) {
  sp_unused(io);
  ops_harness_t* h = (ops_harness_t*)op->user_data;
  sp_assert(h->done_count < OPS_MAX_SLOTS);
  h->done[h->done_count++] = op;
}

static void harness_arm(ops_harness_t* h, sp_io_op_t* op) {
  op->callback = harness_on_done;
  op->user_data = h;
}

static sp_err_t harness_listen(sp_test_t* t, ops_harness_t* h) {
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 } };
  sp_must_ok(t, sp_sys_socket_open(&h->listener, sp_zero_s(sp_sys_handle_desc_t)));
  sp_must_ok(t, sp_sys_socket_bind(h->listener, addr));
  sp_must_ok(t, sp_sys_socket_listen(h->listener, 2));
  return SP_OK;
}

static sp_err_t harness_connect(sp_test_t* t, ops_harness_t* h) {
  u16 port = 0;
  sp_must_ok(t, sp_sys_socket_local_port(h->listener, &port));
  sp_sys_ipv4_t addr = { .octets = { 127, 0, 0, 1 }, .port = port };
  sp_must_ok(t, sp_sys_socket_open(&h->client, sp_zero_s(sp_sys_handle_desc_t)));
  sp_must_ok(t, sp_sys_socket_connect(h->client, addr));
  return SP_OK;
}

static sp_err_t harness_open(sp_test_t* t, ops_harness_t* h, u32 entries, ops_fixture_t fixture) {
  *h = sp_zero_s(ops_harness_t);
  h->listener = SP_SYS_INVALID_SOCKET;
  h->server = SP_SYS_INVALID_SOCKET;
  h->client = SP_SYS_INVALID_SOCKET;
  h->accepted = SP_SYS_INVALID_SOCKET;
  h->pipe.r = SP_SYS_INVALID_FD;
  h->pipe.w = SP_SYS_INVALID_FD;
  sp_for(it, OPS_MAX_SLOTS) h->bufs[it] = sp_mem_slice(h->storage[it], OPS_BUF_SIZE);

  sp_err_t err = sp_io_uring_init(&h->ring, entries);
  if (err == SP_ERR_SYS_UNSUPPORTED || err == SP_ERR_SYS_ACCESS_DENIED) {
    return sp_test_skip(t, "io_uring unavailable");
  }
  sp_must_ok(t, err);
  h->io = sp_io_uring_as_io(&h->ring);

  switch (fixture) {
    case FIXTURE_NONE: break;
    case FIXTURE_LISTENER: {
      sp_try(harness_listen(t, h));
      break;
    }
    case FIXTURE_SOCKETS: {
      sp_try(harness_listen(t, h));
      sp_try(harness_connect(t, h));
      sp_must_ok(t, sp_sys_socket_accept(h->listener, sp_zero_s(sp_sys_handle_desc_t), &h->server));
      break;
    }
    case FIXTURE_PIPE: {
      sp_must_ok(t, sp_sys_pipe(&h->pipe, sp_zero_s(sp_sys_pipe_desc_t)));
      break;
    }
  }
  return SP_OK;
}

static void harness_close(ops_harness_t* h) {
  sp_io_uring_deinit(&h->ring);
  if (h->accepted != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(h->accepted);
  if (h->client != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(h->client);
  if (h->server != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(h->server);
  if (h->listener != SP_SYS_INVALID_SOCKET) sp_sys_socket_close(h->listener);
  if (h->pipe.r != SP_SYS_INVALID_FD) sp_sys_close(h->pipe.r);
  if (h->pipe.w != SP_SYS_INVALID_FD) sp_sys_close(h->pipe.w);
}

static s64 harness_handle(ops_harness_t* h, ops_handle_t which) {
  switch (which) {
    case H_NONE:     return -1;
    case H_LISTENER: return h->listener;
    case H_SERVER:   return h->server;
    case H_CLIENT:   return h->client;
    case H_ACCEPTED: return h->accepted;
    case H_PIPE_R:   return h->pipe.r;
    case H_PIPE_W:   return h->pipe.w;
  }
  return -1;
}

static sp_mem_slice_t harness_op_buf(const sp_io_op_t* op) {
  switch (op->kind) {
    case SP_IO_OP_ACCEPT:  return sp_zero_s(sp_mem_slice_t);
    case SP_IO_OP_RECV:    return op->recv.buf;
    case SP_IO_OP_SEND:    return op->send.buf;
    case SP_IO_OP_READ:    return op->read.buf;
    case SP_IO_OP_WRITE:   return op->write.buf;
    case SP_IO_OP_TIMEOUT: return sp_zero_s(sp_mem_slice_t);
    case SP_IO_OP_CONNECT: return sp_zero_s(sp_mem_slice_t);
    case SP_IO_OP_WORK:    return sp_zero_s(sp_mem_slice_t);
    case SP_IO_OP_IS_TTY:  return sp_zero_s(sp_mem_slice_t);
    case SP_IO_OP_TTY_GET: return sp_zero_s(sp_mem_slice_t);
    case SP_IO_OP_TTY_SET: return sp_zero_s(sp_mem_slice_t);
  }
  return sp_zero_s(sp_mem_slice_t);
}

static sp_io_timeout_t harness_make_timeout(ops_harness_t* h, sp_io_timeout_kind_t kind, s64 ms, sp_io_clock_t clock) {
  switch (kind) {
    case SP_IO_TIMEOUT_NONE: return sp_io_timeout_none();
    case SP_IO_TIMEOUT_DURATION: {
      return (sp_io_timeout_t) {
        .kind = SP_IO_TIMEOUT_DURATION,
        .time = { .ns = sp_tm_ms_to_ns((u64)ms), .clock = clock },
      };
    }
    case SP_IO_TIMEOUT_DEADLINE: {
      sp_io_time_t at = sp_io_now(h->io, clock);
      at.ns = (u64)((s64)at.ns + ms * (s64)SP_TM_MS_TO_NS);
      h->deadline = at;
      return sp_io_timeout_at(at);
    }
  }
  return sp_io_timeout_none();
}

static sp_err_t harness_submit(sp_test_t* t, ops_harness_t* h, const ops_submit_t* s) {
  sp_io_op_t* op = &h->ops[s->slot];
  u64 size = s->data ? sp_cstr_len(s->data) : s->size;
  if (size > h->bufs[s->slot].len) {
    h->bufs[s->slot] = sp_mem_slice(sp_alloc_n(sp_test_arena(t), u8, size), size);
  }
  sp_mem_slice_t buf = sp_mem_slice_prefix(h->bufs[s->slot], size);
  if (s->data) sp_mem_copy(buf.data, s->data, size);

  op->kind = s->op;
  switch (s->op) {
    case SP_IO_OP_ACCEPT: {
      op->accept.socket = (sp_sys_socket_t)harness_handle(h, s->handle);
      op->accept.desc = sp_zero_s(sp_sys_handle_desc_t);
      break;
    }
    case SP_IO_OP_RECV: {
      op->recv.socket = (sp_sys_socket_t)harness_handle(h, s->handle);
      op->recv.buf = buf;
      break;
    }
    case SP_IO_OP_SEND: {
      op->send.socket = (sp_sys_socket_t)harness_handle(h, s->handle);
      op->send.buf = buf;
      break;
    }
    case SP_IO_OP_READ: {
      op->read.fd = (sp_sys_fd_t)harness_handle(h, s->handle);
      op->read.buf = buf;
      break;
    }
    case SP_IO_OP_WRITE: {
      op->write.fd = (sp_sys_fd_t)harness_handle(h, s->handle);
      op->write.buf = buf;
      break;
    }
    case SP_IO_OP_TIMEOUT: {
      op->timeout.timeout = harness_make_timeout(h, SP_IO_TIMEOUT_DURATION, s->ms, SP_IO_CLOCK_AWAKE);
      break;
    }
    case SP_IO_OP_CONNECT:
    case SP_IO_OP_WORK:
    case SP_IO_OP_IS_TTY:
    case SP_IO_OP_TTY_GET:
    case SP_IO_OP_TTY_SET: {
      sp_unreachable_case();
    }
  }
  harness_arm(h, op);
  return sp_io_submit(h->io, op);
}

static sp_io_timeout_t harness_timeout(ops_harness_t* h, const ops_wait_t* w) {
  switch (w->kind) {
    case WAIT_BOUNDED: return harness_make_timeout(h, SP_IO_TIMEOUT_DURATION, OPS_WAIT_MS, SP_IO_CLOCK_AWAKE);
    case WAIT_NONE:    return sp_io_timeout_none();
    case WAIT_AFTER:   return harness_make_timeout(h, SP_IO_TIMEOUT_DURATION, w->ms, w->clock);
    case WAIT_AT:      return harness_make_timeout(h, SP_IO_TIMEOUT_DEADLINE, w->ms, w->clock);
  }
  return sp_io_timeout_none();
}

static void harness_wait(sp_test_t* t, ops_harness_t* h, const ops_wait_t* w) {
  h->done_count = 0;
  u32 count = 0;
  sp_tm_timer_t timer = sp_tm_start_timer();
  sp_err_t err = sp_io_dispatch(h->io, harness_timeout(h, w), &count);
  u64 elapsed = sp_tm_read_timer(&timer);

  sp_expect_ok(t, err);
  sp_expect_eq(t, count, w->expect.count);
  sp_expect_ge(t, elapsed, (u64)w->expect.min_elapsed_ms * SP_TM_MS_TO_NS);
  if (w->expect.after_deadline) {
    sp_expect_ge(t, sp_io_now(h->io, h->deadline.clock).ns, h->deadline.ns);
  }

  sp_for(it, w->expect.count) {
    const ops_done_t* d = &w->expect.done[it];
    sp_test_kv(t, "slot", sp_test_format(t, "{}", sp_fmt_uint(d->slot)));
    sp_io_op_t* op = &h->ops[d->slot];
    bool found = false;
    sp_for(jt, h->done_count) found = found || h->done[jt] == op;
    sp_expect(t, found);
    if (!found) continue;

    sp_mem_slice_t buf = harness_op_buf(op);
    sp_expect_err_eq(t, op->result.err, d->err);
    if (d->partial) {
      sp_expect_gt(t, op->result.len, (u64)0);
      sp_expect_lt(t, op->result.len, buf.len);
    }
    else {
      sp_expect_eq(t, op->result.len, d->len);
    }
    if (d->content) {
      sp_expect_str_eq_c(t, sp_str((const c8*)buf.data, (u32)op->result.len), d->content);
    }
    if (op->result.err == SP_ERR_IO_CANCELED) {
      bool untouched = true;
      sp_for(kt, buf.len) untouched = untouched && buf.data[kt] == 0;
      sp_expect(t, untouched);
    }
    if (op->kind == SP_IO_OP_ACCEPT && !op->result.err) {
      h->accepted = op->result.socket;
      sp_expect_ne(t, h->accepted, SP_SYS_INVALID_SOCKET);
    }
  }
  sp_test_kv_clear(t, "slot");
}

#endif
