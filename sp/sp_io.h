#ifndef SP_IO_H
#define SP_IO_H

#include "sp.h"

typedef struct sp_io_vtable_t sp_io_vtable_t;
typedef struct sp_io_op sp_io_op_t;

typedef struct {
  void* user_data;
  const sp_io_vtable_t* vt;
} sp_io_t;

typedef enum {
  SP_IO_CLOCK_AWAKE,
  SP_IO_CLOCK_BOOT,
  SP_IO_CLOCK_REAL,
} sp_io_clock_t;

typedef struct {
  u64 ns;
  sp_io_clock_t clock;
} sp_io_time_t;

typedef enum {
  SP_IO_TIMEOUT_NONE,
  SP_IO_TIMEOUT_DURATION,
  SP_IO_TIMEOUT_DEADLINE,
} sp_io_timeout_kind_t;

typedef struct {
  sp_io_timeout_kind_t kind;
  sp_io_time_t time;
} sp_io_timeout_t;

#define sp_io_timeout_none()     ((sp_io_timeout_t) { .kind = SP_IO_TIMEOUT_NONE })
#define sp_io_timeout_after(_ns) ((sp_io_timeout_t) { .kind = SP_IO_TIMEOUT_DURATION, .time = { .ns = (_ns), .clock = SP_IO_CLOCK_AWAKE } })
#define sp_io_timeout_at(_time)  ((sp_io_timeout_t) { .kind = SP_IO_TIMEOUT_DEADLINE, .time = (_time) })

typedef enum {
  SP_IO_OP_ACCEPT,
  SP_IO_OP_CONNECT,
  SP_IO_OP_RECV,
  SP_IO_OP_SEND,
  SP_IO_OP_READ,
  SP_IO_OP_WRITE,
  SP_IO_OP_TIMEOUT,
  SP_IO_OP_WORK,
  SP_IO_OP_IS_TTY,
  SP_IO_OP_TTY_GET,
  SP_IO_OP_TTY_SET,
} sp_io_op_kind_t;

SP_TYPEDEF_FN(void, sp_io_op_fn_t, sp_io_t io, sp_io_op_t* op);
SP_TYPEDEF_FN(void, sp_io_work_fn_t, void* context);

struct sp_io_op {
  sp_io_op_kind_t kind;
  union {
    struct {
      sp_sys_socket_t socket;
      sp_sys_handle_desc_t desc;
    } accept;
    struct {
      sp_sys_ipv4_t addr;
      sp_sys_handle_desc_t desc;
      sp_sys_socket_t socket;
      u8 sockaddr [16];
    } connect;
    struct {
      sp_sys_socket_t socket;
      sp_mem_slice_t buf;
    } recv;
    struct {
      sp_sys_socket_t socket;
      sp_mem_slice_t buf;
    } send;
    struct {
      sp_sys_fd_t fd;
      sp_mem_slice_t buf;
    } read;
    struct {
      sp_sys_fd_t fd;
      sp_mem_slice_t buf;
    } write;
    struct {
      sp_io_timeout_t timeout;
      sp_sys_timespec_t ts;
    } timeout;
    struct {
      sp_io_work_fn_t fn;
      void* context;
    } work;
    struct {
      sp_sys_fd_t fd;
    } is_tty;
    struct {
      sp_sys_fd_t fd;
      sp_sys_tty_attr_t* attr;
    } tty_get;
    struct {
      sp_sys_fd_t fd;
      const sp_sys_tty_attr_t* attr;
    } tty_set;
  };
  sp_io_op_fn_t callback;
  void* user_data;
  bool done;
  struct {
    sp_err_t err;
    u64 len;
    sp_sys_socket_t socket;
    bool flag;
  } result;
};

struct sp_io_vtable_t {
  sp_err_t     (*submit)(void* user_data, sp_io_op_t* op);
  sp_err_t     (*wait)(void* user_data, sp_io_op_t** done, u32 max, sp_io_timeout_t timeout, u32* count);
  sp_err_t     (*cancel)(void* user_data, sp_io_op_t* op);
  sp_err_t     (*close)(void* user_data, sp_sys_socket_t socket);
  sp_err_t     (*wake)(void* user_data);
  sp_io_time_t (*now)(void* user_data, sp_io_clock_t clock);
  void         (*destroy)(void* user_data);
};

SP_API sp_err_t     sp_io_new(sp_io_t* io);
SP_API void         sp_io_destroy(sp_io_t io);
SP_API sp_err_t     sp_io_submit(sp_io_t io, sp_io_op_t* op);
SP_API void         sp_io_complete(sp_io_t io, sp_io_op_t* op);
SP_API sp_err_t     sp_io_dispatch(sp_io_t io, sp_io_timeout_t timeout, u32* count);
SP_API sp_err_t     sp_io_cancel(sp_io_t io, sp_io_op_t* op);
SP_API sp_err_t     sp_io_close(sp_io_t io, sp_sys_socket_t socket);
SP_API sp_err_t     sp_io_wake(sp_io_t io);
SP_API sp_io_time_t sp_io_now(sp_io_t io, sp_io_clock_t clock);

typedef struct {
  sp_atomic_u32_t woken;
} sp_io_blocking_t;

SP_API sp_io_t sp_io_blocking_init(sp_io_blocking_t* b);

typedef struct sp_io_uring_sqe sp_io_uring_sqe_t;
typedef struct sp_io_uring_cqe sp_io_uring_cqe_t;

typedef struct {
  s32 fd;
  u32 features;
  u32 to_submit;
  struct {
    u32* head;
    u32* tail;
    u32* mask;
    u32* array;
    u32 entries;
    sp_io_uring_sqe_t* sqes;
  } sq;
  struct {
    u32* head;
    u32* tail;
    u32* mask;
    sp_io_uring_cqe_t* cqes;
  } cq;
  struct {
    u8* rings;
    u64 rings_len;
    u8* sqes;
    u64 sqes_len;
  } map;
  sp_sys_event_t wake;
  bool wake_armed;
} sp_io_uring_t;

#define SP_IO_URING_DEFAULT_ENTRIES 256

SP_API sp_err_t sp_io_uring_init(sp_io_uring_t* ring, u32 entries);
SP_API void     sp_io_uring_deinit(sp_io_uring_t* ring);
SP_API sp_io_t  sp_io_uring_as_io(sp_io_uring_t* ring);

#define SP_IO_SIM_MAX_OPS       32
#define SP_IO_SIM_MAX_CONNS     8
#define SP_IO_SIM_MAX_LISTENERS 2
#define SP_IO_SIM_MAX_BACKLOG   4
#define SP_IO_SIM_WIRE_MAX      4096
#define SP_IO_SIM_SOCKET_BASE   0x51000000

typedef struct sp_io_sim sp_io_sim_t;

typedef struct {
  sp_sys_socket_t socket;
  u16             port;
  sp_sys_socket_t backlog [SP_IO_SIM_MAX_BACKLOG];
  u32             backlog_count;
  bool            live;
} sp_io_sim_listener_t;

typedef struct {
  u8  data [SP_IO_SIM_WIRE_MAX];
  u64 len;
} sp_io_sim_wire_t;

typedef struct {
  sp_sys_socket_t  sockets [2];
  sp_io_sim_wire_t wire [2];
  bool             open [2];
  u64              chunk;
  bool             reset;
  bool             live;
} sp_io_sim_conn_t;

typedef struct {
  sp_io_op_t* op;
  u64         deadline;
} sp_io_sim_op_t;

struct sp_io_sim {
  u64                  now;
  u64                  completions;
  sp_sys_socket_t      next_socket;
  sp_io_sim_listener_t listeners [SP_IO_SIM_MAX_LISTENERS];
  sp_io_sim_conn_t     conns [SP_IO_SIM_MAX_CONNS];
  sp_io_sim_op_t       armed [SP_IO_SIM_MAX_OPS];
  u32                  armed_count;
  sp_io_sim_op_t       done [SP_IO_SIM_MAX_OPS];
  u32                  done_count;
};

SP_API void            sp_io_sim_init(sp_io_sim_t* sim);
SP_API sp_io_t         sp_io_sim_actor(sp_io_sim_t* sim);
SP_API sp_sys_socket_t sp_io_sim_listen(sp_io_sim_t* sim, u16 port);
SP_API void            sp_io_sim_advance(sp_io_sim_t* sim, u64 ns);
SP_API void            sp_io_sim_kill(sp_io_sim_t* sim, sp_sys_socket_t socket);
SP_API void            sp_io_sim_chunk(sp_io_sim_t* sim, sp_sys_socket_t socket, u64 max);

typedef struct sp_task sp_task_t;
typedef struct sp_task_sched_vtable_t sp_task_sched_vtable_t;

typedef struct {
  void* user_data;
  const sp_task_sched_vtable_t* vt;
} sp_task_sched_t;

SP_TYPEDEF_FN(void, sp_task_fn_t, void* context);

typedef enum {
  SP_TASK_WAIT_NONE,
  SP_TASK_WAIT_PARK,
  SP_TASK_WAIT_OP,
  SP_TASK_WAIT_LOCKED,
} sp_task_wait_kind_t;

typedef struct {
  sp_task_t*      task;
  sp_atomic_u32_t signal;
} sp_task_waiter_t;

struct sp_task {
  sp_task_sched_t sched;
  sp_task_fn_t    fn;
  void*           context;
  sp_atomic_u32_t done;
  sp_atomic_u32_t cancel_requested;
  sp_atomic_u32_t unparked;
  sp_atomic_u32_t wait_state;
  struct {
    sp_io_t     io;
    sp_io_op_t* op;
  } wait;
  sp_atomic_ptr_t awaiter;
  bool            parked;
  sp_task_t*      next;
  sp_task_t*      inbox_next;
  sp_atomic_u32_t inboxed;
  sp_thread_t     thread;
  void*           sp;
  sp_mem_slice_t  stack;
};

struct sp_task_sched_vtable_t {
  sp_task_t* (*spawn)(void* user_data, sp_task_fn_t fn, void* context);
  void       (*park)(void* user_data, sp_task_t* task);
  void       (*unpark)(void* user_data, sp_task_t* task);
  void       (*run)(void* user_data);
};

SP_API sp_task_t*       sp_task_current(void);
SP_API sp_task_t*       sp_task_spawn(sp_task_sched_t sched, sp_task_fn_t fn, void* context);
SP_API void             sp_task_run(sp_task_sched_t sched);
SP_API void             sp_task_await(sp_task_t* task);
SP_API void             sp_task_cancel(sp_task_t* task);
SP_API void             sp_task_enter(sp_task_t* task);
SP_API sp_task_waiter_t sp_task_waiter(void);
SP_API void             sp_task_wait(sp_task_waiter_t* w);
SP_API sp_err_t         sp_task_wait_cancelable(sp_task_waiter_t* w);
SP_API void             sp_task_notify(sp_task_waiter_t* w);

#define SP_TASK_GROUP_MAX 8

typedef struct {
  sp_task_sched_t sched;
  sp_task_t*      tasks [SP_TASK_GROUP_MAX];
  u32             count;
} sp_task_group_t;

SP_API sp_task_group_t sp_task_group(sp_task_sched_t sched);
SP_API sp_task_t*      sp_task_group_spawn(sp_task_group_t* g, sp_task_fn_t fn, void* context);
SP_API void            sp_task_group_await(sp_task_group_t* g);
SP_API void            sp_task_group_cancel(sp_task_group_t* g);

#define SP_TASK_QUEUE_WAITERS 16

typedef struct {
  sp_mem_slice_t    buffer;
  u32               elem_size;
  u64               start;
  u64               len;
  bool              closed;
  sp_spin_lock_t    lock;
  sp_task_waiter_t* getters [SP_TASK_QUEUE_WAITERS];
  u32               getter_count;
  sp_task_waiter_t* putters [SP_TASK_QUEUE_WAITERS];
  u32               putter_count;
} sp_task_queue_t;

SP_API sp_task_queue_t sp_task_queue_init(sp_mem_slice_t buffer, u32 elem_size);
SP_API void            sp_task_queue_close(sp_task_queue_t* q);
SP_API sp_err_t        sp_task_queue_put(sp_task_queue_t* q, const void* elem);
SP_API sp_err_t        sp_task_queue_get(sp_task_queue_t* q, void* elem);

#define SP_TASK_MAX              64
#define SP_TASK_FIBER_STACK_SIZE 262144

#if (defined(SP_AMD64) || defined(SP_ARM64)) && !defined(SP_WIN32) && !defined(SP_WASM)
  #define SP_TASK_FIBER_SUPPORTED
#endif

typedef struct {
  sp_io_t         io;
  sp_mem_t        mem;
  sp_task_t       tasks [SP_TASK_MAX];
  u32             count;
  sp_atomic_u32_t live;
  sp_task_t*      ready;
  sp_task_t*      ready_tail;
  sp_spin_lock_t  inbox_lock;
  sp_task_t*      inbox;
  u64             thread_id;
  sp_task_t*      running;
  void*           run_sp;
} sp_task_fiber_t;

SP_API sp_task_sched_t sp_task_fiber_init(sp_task_fiber_t* f, sp_io_t io, sp_mem_t mem);
SP_API void            sp_task_fiber_deinit(sp_task_fiber_t* f);

typedef struct {
  sp_io_t         io;
  sp_task_t       tasks [SP_TASK_MAX];
  u32             count;
  sp_atomic_u32_t live;
} sp_task_threaded_t;

SP_API sp_task_sched_t sp_task_threaded_init(sp_task_threaded_t* t, sp_io_t io);

SP_API sp_err_t sp_io_op_run(sp_io_t io, sp_io_op_t* op);

SP_API sp_err_t sp_io_connect(sp_io_t io, sp_sys_ipv4_t addr, sp_sys_socket_t* out);
SP_API sp_err_t sp_io_accept(sp_io_t io, sp_sys_socket_t listener, sp_sys_socket_t* out);
SP_API sp_err_t sp_io_recv(sp_io_t io, sp_sys_socket_t socket, sp_mem_slice_t buf, u64* out_len);
SP_API sp_err_t sp_io_send_all(sp_io_t io, sp_sys_socket_t socket, sp_mem_slice_t buf);
SP_API sp_err_t sp_io_fd_read(sp_io_t io, sp_sys_fd_t fd, sp_mem_slice_t buf, u64* out_len);
SP_API sp_err_t sp_io_fd_write_all(sp_io_t io, sp_sys_fd_t fd, sp_mem_slice_t buf);
SP_API sp_err_t sp_io_sleep(sp_io_t io, u64 ns);
SP_API sp_err_t sp_io_work(sp_io_t io, sp_io_work_fn_t fn, void* context);
SP_API sp_err_t sp_io_is_tty(sp_io_t io, sp_sys_fd_t fd, bool* out);
SP_API sp_err_t sp_io_tty_get(sp_io_t io, sp_sys_fd_t fd, sp_sys_tty_attr_t* attr);
SP_API sp_err_t sp_io_tty_set(sp_io_t io, sp_sys_fd_t fd, const sp_sys_tty_attr_t* attr);

#endif

#if defined(SP_IMPLEMENTATION) && !defined(SP_IO_IMPLEMENTATION)
  #define SP_IO_IMPLEMENTATION
#endif

#if defined(SP_IO_IMPLEMENTATION) && !defined(SP_IO_C)
#define SP_IO_C

#define SP_IO_DISPATCH_MAX 64

void sp_io_destroy(sp_io_t io) {
  io.vt->destroy(io.user_data);
}

sp_err_t sp_io_submit(sp_io_t io, sp_io_op_t* op) {
  op->done = false;
  return io.vt->submit(io.user_data, op);
}

void sp_io_complete(sp_io_t io, sp_io_op_t* op) {
  op->done = true;
  if (op->callback) op->callback(io, op);
}

sp_err_t sp_io_dispatch(sp_io_t io, sp_io_timeout_t timeout, u32* out_count) {
  sp_io_op_t* done [SP_IO_DISPATCH_MAX];
  u32 count = 0;
  sp_try(io.vt->wait(io.user_data, done, SP_IO_DISPATCH_MAX, timeout, &count));
  sp_for(it, count) {
    sp_io_complete(io, done[it]);
  }
  if (out_count) *out_count = count;
  return SP_OK;
}

sp_err_t sp_io_cancel(sp_io_t io, sp_io_op_t* op) {
  return io.vt->cancel(io.user_data, op);
}

sp_err_t sp_io_close(sp_io_t io, sp_sys_socket_t socket) {
  return io.vt->close(io.user_data, socket);
}

sp_err_t sp_io_wake(sp_io_t io) {
  return io.vt->wake(io.user_data);
}

sp_io_time_t sp_io_now(sp_io_t io, sp_io_clock_t clock) {
  return io.vt->now(io.user_data, clock);
}

SP_PRIVATE sp_err_t     sp_io_blocking_submit(void* user_data, sp_io_op_t* op);
SP_PRIVATE sp_err_t     sp_io_blocking_wait(void* user_data, sp_io_op_t** done, u32 max, sp_io_timeout_t timeout, u32* count);
SP_PRIVATE sp_err_t     sp_io_blocking_cancel(void* user_data, sp_io_op_t* op);
SP_PRIVATE sp_err_t     sp_io_blocking_close(void* user_data, sp_sys_socket_t socket);
SP_PRIVATE sp_err_t     sp_io_blocking_wake(void* user_data);
SP_PRIVATE sp_io_time_t sp_io_blocking_now(void* user_data, sp_io_clock_t clock);
SP_PRIVATE void         sp_io_blocking_destroy(void* user_data);

SP_PRIVATE const sp_io_vtable_t sp_io_blocking_vtable = {
  .submit  = sp_io_blocking_submit,
  .wait    = sp_io_blocking_wait,
  .cancel  = sp_io_blocking_cancel,
  .close   = sp_io_blocking_close,
  .wake    = sp_io_blocking_wake,
  .now     = sp_io_blocking_now,
  .destroy = sp_io_blocking_destroy,
};

SP_PRIVATE sp_io_t sp_io_blocking_as_io(sp_io_blocking_t* b) {
  return (sp_io_t) { .user_data = b, .vt = &sp_io_blocking_vtable };
}

SP_PRIVATE s32 sp_io_blocking_clockid(sp_io_clock_t clock) {
  switch (clock) {
    case SP_IO_CLOCK_AWAKE: return SP_CLOCK_MONOTONIC;
    case SP_IO_CLOCK_BOOT:  return SP_CLOCK_MONOTONIC;
    case SP_IO_CLOCK_REAL:  return SP_CLOCK_REALTIME;
  }
  return SP_CLOCK_MONOTONIC;
}

SP_PRIVATE u64 sp_io_blocking_now_ns(sp_io_clock_t clock) {
  sp_sys_timespec_t ts = sp_zero;
  sp_sys_clock_gettime(sp_io_blocking_clockid(clock), &ts);
  return (u64)ts.tv_sec * SP_TM_S_TO_NS + (u64)ts.tv_nsec;
}

SP_PRIVATE u64 sp_io_blocking_remaining(sp_io_timeout_t timeout) {
  switch (timeout.kind) {
    case SP_IO_TIMEOUT_NONE: return 0;
    case SP_IO_TIMEOUT_DURATION: return timeout.time.ns;
    case SP_IO_TIMEOUT_DEADLINE: {
      u64 now = sp_io_blocking_now_ns(timeout.time.clock);
      return timeout.time.ns > now ? timeout.time.ns - now : 0;
    }
  }
  return 0;
}

SP_PRIVATE sp_err_t sp_io_blocking_sleep(sp_io_timeout_t timeout) {
  sp_sys_timespec_t ts = sp_sys_timespec_from_ns(sp_io_blocking_remaining(timeout));
  return sp_sys_nanosleep(&ts, SP_NULLPTR);
}

SP_PRIVATE sp_err_t sp_io_blocking_submit(void* user_data, sp_io_op_t* op) {
  sp_io_blocking_t* b = (sp_io_blocking_t*)user_data;
  op->result.err = SP_OK;
  op->result.len = 0;
  op->result.socket = SP_SYS_INVALID_SOCKET;
  op->result.flag = false;

  switch (op->kind) {
    case SP_IO_OP_ACCEPT: {
      op->result.err = sp_sys_socket_accept(op->accept.socket, op->accept.desc, &op->result.socket);
      break;
    }
    case SP_IO_OP_CONNECT: {
      sp_sys_socket_t socket = SP_SYS_INVALID_SOCKET;
      op->result.err = sp_sys_socket_open(&socket, op->connect.desc);
      if (!op->result.err) {
        op->result.err = sp_sys_socket_connect(socket, op->connect.addr);
        if (op->result.err) sp_sys_socket_close(socket);
        else op->result.socket = socket;
      }
      break;
    }
    case SP_IO_OP_RECV: {
      op->result.err = sp_sys_socket_recv(op->recv.socket, op->recv.buf.data, op->recv.buf.len, &op->result.len);
      break;
    }
    case SP_IO_OP_SEND: {
      op->result.err = sp_sys_socket_send(op->send.socket, op->send.buf.data, op->send.buf.len, &op->result.len);
      break;
    }
    case SP_IO_OP_READ: {
      op->result.err = sp_sys_read(op->read.fd, op->read.buf.data, op->read.buf.len, &op->result.len);
      break;
    }
    case SP_IO_OP_WRITE: {
      op->result.err = sp_sys_write(op->write.fd, op->write.buf.data, op->write.buf.len, &op->result.len);
      break;
    }
    case SP_IO_OP_TIMEOUT: {
      op->result.err = sp_io_blocking_sleep(op->timeout.timeout);
      break;
    }
    case SP_IO_OP_WORK: {
      op->work.fn(op->work.context);
      break;
    }
    case SP_IO_OP_IS_TTY: {
      op->result.flag = sp_sys_is_tty(op->is_tty.fd);
      break;
    }
    case SP_IO_OP_TTY_GET: {
      op->result.err = sp_sys_tty_get(op->tty_get.fd, op->tty_get.attr);
      break;
    }
    case SP_IO_OP_TTY_SET: {
      op->result.err = sp_sys_tty_set(op->tty_set.fd, op->tty_set.attr);
      break;
    }
  }

  sp_io_complete(sp_io_blocking_as_io(b), op);
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_io_blocking_wait(void* user_data, sp_io_op_t** done, u32 max, sp_io_timeout_t timeout, u32* count) {
  sp_io_blocking_t* io = (sp_io_blocking_t*)user_data;
  sp_unused(done);
  sp_unused(max);
  *count = 0;

  bool bounded = timeout.kind != SP_IO_TIMEOUT_NONE;
  sp_tm_timer_t timer = sp_tm_start_timer();
  u64 budget = sp_io_blocking_remaining(timeout);

  while (!sp_atomic_u32_exchange(&io->woken, 0, SP_ATOMIC_ACQ_REL)) {
    if (!bounded) {
      sp_sys_futex_wait(&io->woken, 0, SP_NULLPTR);
      continue;
    }
    u64 elapsed = sp_tm_read_timer(&timer);
    if (elapsed >= budget) return SP_OK;
    sp_sys_timespec_t ts = sp_sys_timespec_from_ns(budget - elapsed);
    if (!sp_sys_futex_wait(&io->woken, 0, &ts)) return SP_OK;
  }
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_io_blocking_cancel(void* user_data, sp_io_op_t* op) {
  sp_unused(user_data);
  sp_unused(op);
  return SP_ERR_SYS_UNSUPPORTED;
}

SP_PRIVATE sp_err_t sp_io_blocking_close(void* user_data, sp_sys_socket_t socket) {
  sp_unused(user_data);
  return sp_sys_socket_close(socket);
}

SP_PRIVATE sp_err_t sp_io_blocking_wake(void* user_data) {
  sp_io_blocking_t* b = (sp_io_blocking_t*)user_data;
  sp_atomic_u32_store(&b->woken, 1, SP_ATOMIC_RELEASE);
  sp_sys_futex_wake(&b->woken);
  return SP_OK;
}

SP_PRIVATE sp_io_time_t sp_io_blocking_now(void* user_data, sp_io_clock_t clock) {
  sp_unused(user_data);
  return (sp_io_time_t) { .ns = sp_io_blocking_now_ns(clock), .clock = clock };
}

SP_PRIVATE void sp_io_blocking_destroy(void* user_data) {
  sp_unused(user_data);
}

sp_io_t sp_io_blocking_init(sp_io_blocking_t* b) {
  *b = sp_zero_s(sp_io_blocking_t);
  return sp_io_blocking_as_io(b);
}

#if defined(SP_LINUX)

#if !defined(SP_IMPL_H)
  #error "sp_io.h's io_uring backend is built on sp.h's syscall layer; include it from the TU that defines SP_IMPLEMENTATION"
#endif

#define SP_IO_URING_OP_POLL_ADD      6
#define SP_IO_URING_OP_TIMEOUT       11
#define SP_IO_URING_OP_ACCEPT        13
#define SP_IO_URING_OP_ASYNC_CANCEL  14
#define SP_IO_URING_OP_CONNECT       16
#define SP_IO_URING_OP_READ          22
#define SP_IO_URING_OP_WRITE         23
#define SP_IO_URING_OP_SEND          26
#define SP_IO_URING_OP_RECV          27
#define SP_IO_URING_ENTER_GETEVENTS  (1u << 0)
#define SP_IO_URING_ENTER_EXT_ARG    (1u << 3)
#define SP_IO_URING_TIMEOUT_ABS      (1u << 0)
#define SP_IO_URING_TIMEOUT_BOOTTIME (1u << 2)
#define SP_IO_URING_TIMEOUT_REALTIME (1u << 3)
#define SP_IO_URING_FEAT_SINGLE_MMAP (1u << 0)
#define SP_IO_URING_FEAT_NODROP      (1u << 1)
#define SP_IO_URING_FEAT_EXT_ARG     (1u << 8)
#define SP_IO_URING_FEATURES         (SP_IO_URING_FEAT_SINGLE_MMAP | SP_IO_URING_FEAT_NODROP | SP_IO_URING_FEAT_EXT_ARG)
#define SP_IO_URING_OFF_SQ_RING      0ull
#define SP_IO_URING_OFF_SQES         0x10000000ull
#define SP_IO_URING_MAP_POPULATE     0x8000
#define SP_IO_URING_CLOCK_BOOTTIME   7
#define SP_IO_URING_NO_OFFSET        ((u64)-1)
#define SP_IO_URING_USER_DATA_CANCEL 0
#define SP_IO_URING_USER_DATA_WAKE   1

struct sp_io_uring_sqe {
  u8  opcode;
  u8  flags;
  u16 ioprio;
  s32 fd;
  u64 off;
  u64 addr;
  u32 len;
  u32 op_flags;
  u64 user_data;
  u16 buf_index;
  u16 personality;
  s32 splice_fd_in;
  u64 addr3;
  u64 pad;
};

struct sp_io_uring_cqe {
  u64 user_data;
  s32 res;
  u32 flags;
};

typedef struct {
  u32 head;
  u32 tail;
  u32 ring_mask;
  u32 ring_entries;
  u32 flags;
  u32 dropped;
  u32 array;
  u32 resv1;
  u64 user_addr;
} sp_io_uring_sq_offsets_t;

typedef struct {
  u32 head;
  u32 tail;
  u32 ring_mask;
  u32 ring_entries;
  u32 overflow;
  u32 cqes;
  u32 flags;
  u32 resv1;
  u64 user_addr;
} sp_io_uring_cq_offsets_t;

typedef struct {
  u32 sq_entries;
  u32 cq_entries;
  u32 flags;
  u32 sq_thread_cpu;
  u32 sq_thread_idle;
  u32 features;
  u32 wq_fd;
  u32 resv [3];
  sp_io_uring_sq_offsets_t sq_off;
  sp_io_uring_cq_offsets_t cq_off;
} sp_io_uring_params_t;

typedef struct {
  u64 sigmask;
  u32 sigmask_sz;
  u32 min_wait_usec;
  u64 ts;
} sp_io_uring_getevents_arg_t;

sp_static_assert(sizeof(sp_io_uring_sqe_t) == 64, sp_io_uring_sqe_layout);
sp_static_assert(sizeof(sp_io_uring_cqe_t) == 16, sp_io_uring_cqe_layout);

SP_PRIVATE s32 sp_io_uring_clockid(sp_io_clock_t clock) {
  switch (clock) {
    case SP_IO_CLOCK_AWAKE: return SP_CLOCK_MONOTONIC;
    case SP_IO_CLOCK_BOOT:  return SP_IO_URING_CLOCK_BOOTTIME;
    case SP_IO_CLOCK_REAL:  return SP_CLOCK_REALTIME;
  }
  return SP_CLOCK_MONOTONIC;
}

SP_PRIVATE u64 sp_io_uring_now_ns(sp_io_clock_t clock) {
  sp_sys_timespec_t ts = sp_zero;
  sp_sys_clock_gettime(sp_io_uring_clockid(clock), &ts);
  return (u64)ts.tv_sec * SP_TM_S_TO_NS + (u64)ts.tv_nsec;
}

SP_PRIVATE sp_sys_timespec_t sp_io_uring_timespec(u64 ns) {
  return (sp_sys_timespec_t) {
    .tv_sec = (s64)(ns / SP_TM_S_TO_NS),
    .tv_nsec = (s64)(ns % SP_TM_S_TO_NS),
  };
}

SP_PRIVATE u64 sp_io_uring_after(u64 now, u64 ns) {
  return ns > SP_LIMIT_U64_MAX - now ? SP_LIMIT_U64_MAX : now + ns;
}

SP_PRIVATE u64 sp_io_uring_deadline(sp_io_timeout_t timeout) {
  switch (timeout.kind) {
    case SP_IO_TIMEOUT_NONE: return 0;
    case SP_IO_TIMEOUT_DURATION: return sp_io_uring_after(sp_io_uring_now_ns(SP_IO_CLOCK_AWAKE), timeout.time.ns);
    case SP_IO_TIMEOUT_DEADLINE: {
      if (timeout.time.clock == SP_IO_CLOCK_AWAKE) return timeout.time.ns;
      u64 clock_now = sp_io_uring_now_ns(timeout.time.clock);
      u64 remaining = timeout.time.ns > clock_now ? timeout.time.ns - clock_now : 0;
      return sp_io_uring_after(sp_io_uring_now_ns(SP_IO_CLOCK_AWAKE), remaining);
    }
  }
  return 0;
}

SP_PRIVATE u32 sp_io_uring_timeout_flags(sp_io_timeout_t timeout) {
  u32 flags = 0;
  switch (timeout.kind) {
    case SP_IO_TIMEOUT_NONE:     sp_unreachable_return(0);
    case SP_IO_TIMEOUT_DURATION: break;
    case SP_IO_TIMEOUT_DEADLINE: flags |= SP_IO_URING_TIMEOUT_ABS; break;
  }
  switch (timeout.time.clock) {
    case SP_IO_CLOCK_AWAKE: break;
    case SP_IO_CLOCK_BOOT:  flags |= SP_IO_URING_TIMEOUT_BOOTTIME; break;
    case SP_IO_CLOCK_REAL:  flags |= SP_IO_URING_TIMEOUT_REALTIME; break;
  }
  return flags;
}

SP_PRIVATE u32 sp_io_uring_accept_flags(sp_sys_handle_desc_t desc) {
  u32 flags = desc.inherited == SP_SYS_INHERITED ? 0 : SP_SYS_LINUX_SOCK_CLOEXEC;
  if (desc.mode == SP_SYS_NONBLOCKING) flags |= SP_SYS_LINUX_SOCK_NONBLOCK;
  return flags;
}

SP_PRIVATE u32 sp_io_uring_len(sp_mem_slice_t buf) {
  return (u32)sp_min(buf.len, (u64)SP_LIMIT_S32_MAX);
}

SP_PRIVATE s64 sp_io_uring_enter(sp_io_uring_t* ring, u32 min_complete, u32 flags, sp_io_uring_getevents_arg_t* arg) {
  u64 argsz = arg ? sizeof(*arg) : 0;
  s64 rc = sp_syscall(SP_SYSCALL_NUM_IO_URING_ENTER, ring->fd, ring->to_submit, min_complete, flags, arg, argsz);
  if (rc >= 0) ring->to_submit -= (u32)rc;
  return rc;
}

SP_PRIVATE sp_err_t sp_io_uring_flush(sp_io_uring_t* ring) {
  s64 rc = sp_io_uring_enter(ring, 0, 0, SP_NULLPTR);
  return rc < 0 ? sp_sys_err_from_errno(-rc) : SP_OK;
}

SP_PRIVATE sp_err_t sp_io_uring_sqe(sp_io_uring_t* ring, sp_io_uring_sqe_t** out) {
  u32 tail = *ring->sq.tail;
  if (tail - sp_atomic_u32_load(ring->sq.head, SP_ATOMIC_ACQUIRE) == ring->sq.entries) {
    sp_try(sp_io_uring_flush(ring));
  }
  u32 index = tail & *ring->sq.mask;
  sp_io_uring_sqe_t* sqe = &ring->sq.sqes[index];
  *sqe = sp_zero_s(sp_io_uring_sqe_t);
  ring->sq.array[index] = index;
  *out = sqe;
  return SP_OK;
}

SP_PRIVATE void sp_io_uring_push(sp_io_uring_t* ring) {
  sp_atomic_u32_store(ring->sq.tail, *ring->sq.tail + 1, SP_ATOMIC_RELEASE);
  ring->to_submit++;
}

SP_PRIVATE sp_err_t sp_io_uring_arm_wake(sp_io_uring_t* ring) {
  sp_io_uring_sqe_t* sqe = SP_NULLPTR;
  sp_try(sp_io_uring_sqe(ring, &sqe));
  sqe->opcode = SP_IO_URING_OP_POLL_ADD;
  sqe->fd = (s32)ring->wake.fd;
  sqe->op_flags = SP_SYS_LINUX_POLLIN;
  sqe->user_data = SP_IO_URING_USER_DATA_WAKE;
  sp_io_uring_push(ring);
  ring->wake_armed = true;
  return SP_OK;
}

SP_PRIVATE bool sp_io_uring_inline(sp_io_uring_t* ring, sp_io_op_t* op) {
  op->result.err = SP_OK;
  op->result.len = 0;
  op->result.socket = SP_SYS_INVALID_SOCKET;
  op->result.flag = false;

  switch (op->kind) {
    case SP_IO_OP_IS_TTY: {
      op->result.flag = sp_sys_is_tty(op->is_tty.fd);
      break;
    }
    case SP_IO_OP_TTY_GET: {
      op->result.err = sp_sys_tty_get(op->tty_get.fd, op->tty_get.attr);
      break;
    }
    case SP_IO_OP_TTY_SET: {
      op->result.err = sp_sys_tty_set(op->tty_set.fd, op->tty_set.attr);
      break;
    }
    case SP_IO_OP_ACCEPT:
    case SP_IO_OP_CONNECT:
    case SP_IO_OP_RECV:
    case SP_IO_OP_SEND:
    case SP_IO_OP_READ:
    case SP_IO_OP_WRITE:
    case SP_IO_OP_TIMEOUT:
    case SP_IO_OP_WORK: {
      return false;
    }
  }

  sp_io_complete(sp_io_uring_as_io(ring), op);
  return true;
}

SP_PRIVATE sp_err_t sp_io_uring_submit(void* user_data, sp_io_op_t* op) {
  sp_io_uring_t* ring = (sp_io_uring_t*)user_data;
  if (op->kind == SP_IO_OP_WORK) return SP_ERR_SYS_UNSUPPORTED;
  if (sp_io_uring_inline(ring, op)) return SP_OK;

  sp_io_uring_sqe_t* sqe = SP_NULLPTR;
  sp_try(sp_io_uring_sqe(ring, &sqe));
  sqe->user_data = (u64)sp_uptr(op);

  switch (op->kind) {
    case SP_IO_OP_ACCEPT: {
      sqe->opcode = SP_IO_URING_OP_ACCEPT;
      sqe->fd = (s32)op->accept.socket;
      sqe->op_flags = sp_io_uring_accept_flags(op->accept.desc);
      break;
    }
    case SP_IO_OP_CONNECT: {
      sp_try(sp_sys_socket_open(&op->connect.socket, op->connect.desc));
      sp_static_assert(sizeof(sp_sys_linux_sockaddr_in_t) <= sizeof(op->connect.sockaddr), sp_io_uring_sockaddr_fits);
      sp_sys_linux_sockaddr_in_t* sa = (sp_sys_linux_sockaddr_in_t*)op->connect.sockaddr;
      *sa = sp_zero_s(sp_sys_linux_sockaddr_in_t);
      sa->family = SP_SYS_LINUX_AF_INET;
      sa->port[0] = (u8)(op->connect.addr.port >> 8);
      sa->port[1] = (u8)(op->connect.addr.port & 0xFF);
      sp_mem_copy(sa->addr, op->connect.addr.octets, 4);
      sqe->opcode = SP_IO_URING_OP_CONNECT;
      sqe->fd = (s32)op->connect.socket;
      sqe->addr = (u64)sp_uptr(op->connect.sockaddr);
      sqe->off = sizeof(sp_sys_linux_sockaddr_in_t);
      break;
    }
    case SP_IO_OP_RECV: {
      sqe->opcode = SP_IO_URING_OP_RECV;
      sqe->fd = (s32)op->recv.socket;
      sqe->addr = (u64)sp_uptr(op->recv.buf.data);
      sqe->len = sp_io_uring_len(op->recv.buf);
      break;
    }
    case SP_IO_OP_SEND: {
      sqe->opcode = SP_IO_URING_OP_SEND;
      sqe->fd = (s32)op->send.socket;
      sqe->addr = (u64)sp_uptr(op->send.buf.data);
      sqe->len = sp_io_uring_len(op->send.buf);
      sqe->op_flags = SP_SYS_LINUX_MSG_NOSIGNAL;
      break;
    }
    case SP_IO_OP_READ: {
      sqe->opcode = SP_IO_URING_OP_READ;
      sqe->fd = (s32)op->read.fd;
      sqe->off = SP_IO_URING_NO_OFFSET;
      sqe->addr = (u64)sp_uptr(op->read.buf.data);
      sqe->len = sp_io_uring_len(op->read.buf);
      break;
    }
    case SP_IO_OP_WRITE: {
      sqe->opcode = SP_IO_URING_OP_WRITE;
      sqe->fd = (s32)op->write.fd;
      sqe->off = SP_IO_URING_NO_OFFSET;
      sqe->addr = (u64)sp_uptr(op->write.buf.data);
      sqe->len = sp_io_uring_len(op->write.buf);
      break;
    }
    case SP_IO_OP_TIMEOUT: {
      op->timeout.ts = sp_io_uring_timespec(op->timeout.timeout.time.ns);
      sqe->opcode = SP_IO_URING_OP_TIMEOUT;
      sqe->fd = -1;
      sqe->addr = (u64)sp_uptr(&op->timeout.ts);
      sqe->len = 1;
      sqe->op_flags = sp_io_uring_timeout_flags(op->timeout.timeout);
      break;
    }
    case SP_IO_OP_WORK:
    case SP_IO_OP_IS_TTY:
    case SP_IO_OP_TTY_GET:
    case SP_IO_OP_TTY_SET: {
      sp_unreachable_case();
    }
  }

  sp_io_uring_push(ring);
  return SP_OK;
}

SP_PRIVATE void sp_io_uring_complete(sp_io_op_t* op, s32 res) {
  op->result.err = SP_OK;
  op->result.len = 0;
  op->result.socket = SP_SYS_INVALID_SOCKET;
  op->result.flag = false;

  if (res >= 0) {
    switch (op->kind) {
      case SP_IO_OP_ACCEPT: {
        op->result.socket = (sp_sys_socket_t)res;
        break;
      }
      case SP_IO_OP_CONNECT: {
        op->result.socket = op->connect.socket;
        break;
      }
      case SP_IO_OP_RECV:
      case SP_IO_OP_SEND:
      case SP_IO_OP_READ:
      case SP_IO_OP_WRITE: {
        op->result.len = (u64)res;
        break;
      }
      case SP_IO_OP_TIMEOUT: break;
      case SP_IO_OP_WORK:
      case SP_IO_OP_IS_TTY:
      case SP_IO_OP_TTY_GET:
      case SP_IO_OP_TTY_SET: {
        sp_unreachable_case();
      }
    }
    return;
  }

  s64 e = -(s64)res;
  if (e == SP_ECANCELED) {
    op->result.err = SP_ERR_IO_CANCELED;
  }
  else if (e == SP_ETIME && op->kind == SP_IO_OP_TIMEOUT) {
    op->result.err = SP_OK;
  }
  else {
    op->result.err = sp_sys_err_from_errno(e);
  }

  if (op->kind == SP_IO_OP_CONNECT) {
    sp_sys_socket_close(op->connect.socket);
    op->connect.socket = SP_SYS_INVALID_SOCKET;
  }
}

SP_PRIVATE void sp_io_uring_resubmit_aborted(sp_io_uring_t* ring, sp_io_op_t** done, u32* count) {
  u32 kept = 0;
  sp_for(it, *count) {
    sp_io_op_t* op = done[it];
    if (op->kind == SP_IO_OP_ACCEPT && op->result.err == SP_ERR_SYS_CONN_RESET) {
      sp_err_t err = sp_io_uring_submit(ring, op);
      if (!err) continue;
      op->result.err = err;
    }
    done[kept++] = op;
  }
  *count = kept;
}

SP_PRIVATE sp_err_t sp_io_uring_reap(sp_io_uring_t* ring, sp_io_op_t** done, u32 max, u32* count, bool* woke) {
  u32 head = *ring->cq.head;
  u32 tail = sp_atomic_u32_load(ring->cq.tail, SP_ATOMIC_ACQUIRE);

  while (head != tail && *count < max) {
    sp_io_uring_cqe_t* cqe = &ring->cq.cqes[head & *ring->cq.mask];
    head++;
    if (cqe->user_data == SP_IO_URING_USER_DATA_CANCEL) continue;
    if (cqe->user_data == SP_IO_URING_USER_DATA_WAKE) {
      ring->wake_armed = false;
      *woke = true;
      continue;
    }
    sp_io_op_t* op = (sp_io_op_t*)sp_uptr(cqe->user_data);
    sp_io_uring_complete(op, cqe->res);
    done[(*count)++] = op;
  }
  sp_atomic_u32_store(ring->cq.head, head, SP_ATOMIC_RELEASE);

  sp_io_uring_resubmit_aborted(ring, done, count);
  if (*woke) sp_try(sp_sys_event_clear(ring->wake));
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_io_uring_wait(void* user_data, sp_io_op_t** done, u32 max, sp_io_timeout_t timeout, u32* count) {
  sp_io_uring_t* ring = (sp_io_uring_t*)user_data;
  sp_assert(max);
  *count = 0;
  bool bounded = timeout.kind != SP_IO_TIMEOUT_NONE;
  u64 deadline = sp_io_uring_deadline(timeout);

  while (true) {
    if (!ring->wake_armed) sp_try(sp_io_uring_arm_wake(ring));

    sp_sys_timespec_t ts = sp_zero;
    sp_io_uring_getevents_arg_t arg = sp_zero;
    u32 flags = SP_IO_URING_ENTER_GETEVENTS;
    if (bounded) {
      u64 now = sp_io_uring_now_ns(SP_IO_CLOCK_AWAKE);
      ts = sp_io_uring_timespec(deadline > now ? deadline - now : 0);
      arg.ts = (u64)sp_uptr(&ts);
      flags |= SP_IO_URING_ENTER_EXT_ARG;
    }
    s64 rc = sp_io_uring_enter(ring, 1, flags, bounded ? &arg : SP_NULLPTR);

    bool woke = false;
    sp_err_t err = sp_io_uring_reap(ring, done, max, count, &woke);
    if (err && !*count) return err;
    if (*count || woke) return SP_OK;
    if (rc == -SP_ETIME) return SP_OK;
    if (rc < 0 && rc != -SP_EINTR) return sp_sys_err_from_errno(-rc);
    if (bounded && sp_io_uring_now_ns(SP_IO_CLOCK_AWAKE) >= deadline) return SP_OK;
  }
}

SP_PRIVATE sp_err_t sp_io_uring_cancel(void* user_data, sp_io_op_t* op) {
  sp_io_uring_t* ring = (sp_io_uring_t*)user_data;
  sp_io_uring_sqe_t* sqe = SP_NULLPTR;
  sp_try(sp_io_uring_sqe(ring, &sqe));
  sqe->opcode = SP_IO_URING_OP_ASYNC_CANCEL;
  sqe->fd = -1;
  sqe->addr = (u64)sp_uptr(op);
  sqe->user_data = SP_IO_URING_USER_DATA_CANCEL;
  sp_io_uring_push(ring);
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_io_uring_close(void* user_data, sp_sys_socket_t socket) {
  sp_unused(user_data);
  return sp_sys_socket_close(socket);
}

SP_PRIVATE sp_err_t sp_io_uring_wake(void* user_data) {
  sp_io_uring_t* ring = (sp_io_uring_t*)user_data;
  return sp_sys_event_signal(ring->wake);
}

SP_PRIVATE sp_io_time_t sp_io_uring_now(void* user_data, sp_io_clock_t clock) {
  sp_unused(user_data);
  return (sp_io_time_t) { .ns = sp_io_uring_now_ns(clock), .clock = clock };
}

SP_PRIVATE void sp_io_uring_destroy(void* user_data) {
  sp_io_uring_t* ring = (sp_io_uring_t*)user_data;
  sp_io_uring_deinit(ring);
  sp_sys_free(ring, sizeof(*ring));
}

SP_PRIVATE sp_err_t sp_io_uring_map(sp_io_uring_t* ring, u64 len, u64 offset, u8** out) {
  s64 p = sp_syscall(SP_SYSCALL_NUM_MMAP, 0, len, SP_PROT_READ | SP_PROT_WRITE, SP_MAP_SHARED | SP_IO_URING_MAP_POPULATE, ring->fd, offset);
  if ((u64)p > -4096UL) return sp_sys_err_from_errno(-p);
  *out = (u8*)p;
  return SP_OK;
}

sp_err_t sp_io_uring_init(sp_io_uring_t* ring, u32 entries) {
  *ring = sp_zero_s(sp_io_uring_t);
  ring->fd = -1;
  ring->wake.fd = SP_SYS_INVALID_FD;

  sp_io_uring_params_t params = sp_zero;
  s64 fd = sp_syscall(SP_SYSCALL_NUM_IO_URING_SETUP, entries, &params);
  if (fd < 0) return sp_sys_err_from_errno(-fd);
  ring->fd = (s32)fd;
  ring->features = params.features;
  if ((params.features & SP_IO_URING_FEATURES) != SP_IO_URING_FEATURES) {
    sp_io_uring_deinit(ring);
    return SP_ERR_SYS_UNSUPPORTED;
  }

  u64 sq_len = params.sq_off.array + (u64)params.sq_entries * sizeof(u32);
  u64 cq_len = params.cq_off.cqes + (u64)params.cq_entries * sizeof(sp_io_uring_cqe_t);
  ring->map.rings_len = sp_max(sq_len, cq_len);
  ring->map.sqes_len = (u64)params.sq_entries * sizeof(sp_io_uring_sqe_t);

  sp_err_t err = sp_io_uring_map(ring, ring->map.rings_len, SP_IO_URING_OFF_SQ_RING, &ring->map.rings);
  if (!err) err = sp_io_uring_map(ring, ring->map.sqes_len, SP_IO_URING_OFF_SQES, &ring->map.sqes);
  if (err) {
    sp_io_uring_deinit(ring);
    return err;
  }

  ring->sq.head = (u32*)(ring->map.rings + params.sq_off.head);
  ring->sq.tail = (u32*)(ring->map.rings + params.sq_off.tail);
  ring->sq.mask = (u32*)(ring->map.rings + params.sq_off.ring_mask);
  ring->sq.array = (u32*)(ring->map.rings + params.sq_off.array);
  ring->sq.entries = params.sq_entries;
  ring->sq.sqes = (sp_io_uring_sqe_t*)ring->map.sqes;
  ring->cq.head = (u32*)(ring->map.rings + params.cq_off.head);
  ring->cq.tail = (u32*)(ring->map.rings + params.cq_off.tail);
  ring->cq.mask = (u32*)(ring->map.rings + params.cq_off.ring_mask);
  ring->cq.cqes = (sp_io_uring_cqe_t*)(ring->map.rings + params.cq_off.cqes);

  err = sp_sys_event_open(&ring->wake);
  if (!err) err = sp_io_uring_arm_wake(ring);
  if (err) {
    sp_io_uring_deinit(ring);
    return err;
  }
  return SP_OK;
}

void sp_io_uring_deinit(sp_io_uring_t* ring) {
  if (ring->fd >= 0) sp_sys_close(ring->fd);
  if (ring->wake.fd != SP_SYS_INVALID_FD) sp_sys_close(ring->wake.fd);
  if (ring->map.rings) sp_syscall(SP_SYSCALL_NUM_MUNMAP, ring->map.rings, ring->map.rings_len);
  if (ring->map.sqes) sp_syscall(SP_SYSCALL_NUM_MUNMAP, ring->map.sqes, ring->map.sqes_len);
}

sp_err_t sp_io_new(sp_io_t* io) {
  *io = sp_zero_s(sp_io_t);
  sp_io_uring_t* ring = sp_sys_alloc_type(sp_io_uring_t);
  if (!ring) return SP_ERR_SYS_NO_MEMORY;
  sp_err_t err = sp_io_uring_init(ring, SP_IO_URING_DEFAULT_ENTRIES);
  if (err) {
    sp_sys_free(ring, sizeof(*ring));
    return err;
  }
  *io = sp_io_uring_as_io(ring);
  return SP_OK;
}

#else

SP_PRIVATE sp_err_t sp_io_uring_submit(void* user_data, sp_io_op_t* op) {
  sp_unused(user_data); sp_unused(op);
  return SP_ERR_SYS_UNSUPPORTED;
}

SP_PRIVATE sp_err_t sp_io_uring_wait(void* user_data, sp_io_op_t** done, u32 max, sp_io_timeout_t timeout, u32* count) {
  sp_unused(user_data); sp_unused(done); sp_unused(max); sp_unused(timeout);
  *count = 0;
  return SP_ERR_SYS_UNSUPPORTED;
}

SP_PRIVATE sp_err_t sp_io_uring_cancel(void* user_data, sp_io_op_t* op) {
  sp_unused(user_data); sp_unused(op);
  return SP_ERR_SYS_UNSUPPORTED;
}

SP_PRIVATE sp_err_t sp_io_uring_close(void* user_data, sp_sys_socket_t socket) {
  sp_unused(user_data); sp_unused(socket);
  return SP_ERR_SYS_UNSUPPORTED;
}

SP_PRIVATE sp_err_t sp_io_uring_wake(void* user_data) {
  sp_unused(user_data);
  return SP_ERR_SYS_UNSUPPORTED;
}

SP_PRIVATE sp_io_time_t sp_io_uring_now(void* user_data, sp_io_clock_t clock) {
  sp_unused(user_data);
  return (sp_io_time_t) { .clock = clock };
}

SP_PRIVATE void sp_io_uring_destroy(void* user_data) {
  sp_unused(user_data);
}

sp_err_t sp_io_uring_init(sp_io_uring_t* ring, u32 entries) {
  *ring = sp_zero_s(sp_io_uring_t);
  sp_unused(entries);
  return SP_ERR_SYS_UNSUPPORTED;
}

void sp_io_uring_deinit(sp_io_uring_t* ring) {
  sp_unused(ring);
}

sp_err_t sp_io_new(sp_io_t* io) {
  *io = sp_zero_s(sp_io_t);
  return SP_ERR_SYS_UNSUPPORTED;
}

#endif

SP_PRIVATE const sp_io_vtable_t sp_io_uring_vtable = {
  .submit  = sp_io_uring_submit,
  .wait    = sp_io_uring_wait,
  .cancel  = sp_io_uring_cancel,
  .close   = sp_io_uring_close,
  .wake    = sp_io_uring_wake,
  .now     = sp_io_uring_now,
  .destroy = sp_io_uring_destroy,
};

sp_io_t sp_io_uring_as_io(sp_io_uring_t* ring) {
  return (sp_io_t) { .user_data = ring, .vt = &sp_io_uring_vtable };
}

SP_PRIVATE sp_io_sim_conn_t* sp_io_sim_conn(sp_io_sim_t* sim, sp_sys_socket_t socket, u32* end) {
  sp_carr_for(sim->conns, it) {
    sp_io_sim_conn_t* conn = &sim->conns[it];
    if (!conn->live) continue;
    sp_for(at, 2) {
      if (conn->sockets[at] == socket) {
        *end = at;
        return conn;
      }
    }
  }
  return SP_NULLPTR;
}

SP_PRIVATE sp_io_sim_listener_t* sp_io_sim_listener(sp_io_sim_t* sim, sp_sys_socket_t socket) {
  sp_carr_for(sim->listeners, it) {
    if (sim->listeners[it].live && sim->listeners[it].socket == socket) return &sim->listeners[it];
  }
  return SP_NULLPTR;
}

SP_PRIVATE sp_io_sim_listener_t* sp_io_sim_listener_at(sp_io_sim_t* sim, u16 port) {
  sp_carr_for(sim->listeners, it) {
    if (sim->listeners[it].live && sim->listeners[it].port == port) return &sim->listeners[it];
  }
  return SP_NULLPTR;
}

SP_PRIVATE void sp_io_sim_deliver(sp_io_sim_t* sim, u32 at) {
  sp_assert(sim->done_count < SP_IO_SIM_MAX_OPS);
  sim->done[sim->done_count++] = sim->armed[at];
  sim->armed_count--;
  sp_for(it, sim->armed_count - at) {
    sim->armed[at + it] = sim->armed[at + it + 1];
  }
  sim->completions++;
}

SP_PRIVATE bool sp_io_sim_step_op(sp_io_sim_t* sim, u32 at) {
  sp_io_op_t* op = sim->armed[at].op;

  switch (op->kind) {
    case SP_IO_OP_ACCEPT: {
      sp_io_sim_listener_t* listener = sp_io_sim_listener(sim, op->accept.socket);
      sp_assert(listener);
      if (!listener->backlog_count) return false;
      op->result.err = SP_OK;
      op->result.socket = listener->backlog[0];
      listener->backlog_count--;
      sp_for(it, listener->backlog_count) {
        listener->backlog[it] = listener->backlog[it + 1];
      }
      return true;
    }
    case SP_IO_OP_CONNECT: {
      sp_io_sim_listener_t* listener = sp_io_sim_listener_at(sim, op->connect.addr.port);
      if (!listener) {
        op->result.err = SP_ERR_SYS_CONN_REFUSED;
        return true;
      }
      if (listener->backlog_count == SP_IO_SIM_MAX_BACKLOG) return false;
      sp_io_sim_conn_t* conn = SP_NULLPTR;
      sp_carr_for(sim->conns, it) {
        if (!sim->conns[it].live) {
          conn = &sim->conns[it];
          break;
        }
      }
      sp_assert(conn);
      *conn = (sp_io_sim_conn_t) {
        .sockets = { sim->next_socket, sim->next_socket + 1 },
        .open = { true, true },
        .live = true,
      };
      sim->next_socket += 2;
      listener->backlog[listener->backlog_count++] = conn->sockets[1];
      op->result.err = SP_OK;
      op->result.socket = conn->sockets[0];
      return true;
    }
    case SP_IO_OP_RECV: {
      u32 end = 0;
      sp_io_sim_conn_t* conn = sp_io_sim_conn(sim, op->recv.socket, &end);
      sp_assert(conn);
      if (conn->reset) {
        op->result.err = SP_ERR_SYS_CONN_RESET;
        return true;
      }
      sp_io_sim_wire_t* wire = &conn->wire[end];
      if (wire->len) {
        u64 n = sp_min(op->recv.buf.len, wire->len);
        if (conn->chunk) n = sp_min(n, conn->chunk);
        sp_mem_copy(op->recv.buf.data, wire->data, n);
        wire->len -= n;
        sp_mem_move(wire->data, wire->data + n, wire->len);
        op->result.err = SP_OK;
        op->result.len = n;
        return true;
      }
      if (!conn->open[1 - end]) {
        op->result.err = SP_OK;
        op->result.len = 0;
        return true;
      }
      return false;
    }
    case SP_IO_OP_SEND: {
      u32 end = 0;
      sp_io_sim_conn_t* conn = sp_io_sim_conn(sim, op->send.socket, &end);
      sp_assert(conn);
      if (conn->reset || !conn->open[1 - end]) {
        op->result.err = SP_ERR_SYS_CONN_RESET;
        return true;
      }
      sp_io_sim_wire_t* wire = &conn->wire[1 - end];
      sp_assert(wire->len + op->send.buf.len <= SP_IO_SIM_WIRE_MAX);
      sp_mem_copy(wire->data + wire->len, op->send.buf.data, op->send.buf.len);
      wire->len += op->send.buf.len;
      op->result.err = SP_OK;
      op->result.len = op->send.buf.len;
      return true;
    }
    case SP_IO_OP_TIMEOUT: {
      if (sim->now < sim->armed[at].deadline) return false;
      op->result.err = SP_OK;
      return true;
    }
    case SP_IO_OP_WORK: {
      op->work.fn(op->work.context);
      return true;
    }
    case SP_IO_OP_IS_TTY: {
      op->result.flag = false;
      return true;
    }
    case SP_IO_OP_TTY_GET:
    case SP_IO_OP_TTY_SET: {
      op->result.err = SP_ERR_SYS_UNSUPPORTED;
      return true;
    }
    case SP_IO_OP_READ:
    case SP_IO_OP_WRITE: {
      sp_assert(false);
      return false;
    }
  }
  sp_unreachable_return(false);
}

SP_PRIVATE void sp_io_sim_step(sp_io_sim_t* sim) {
  bool progressed = true;
  while (progressed) {
    progressed = false;
    sp_for(it, sim->armed_count) {
      sp_io_op_t* op = sim->armed[it].op;
      op->result.err = SP_OK;
      op->result.len = 0;
      op->result.socket = SP_SYS_INVALID_SOCKET;
      op->result.flag = false;
      if (!sp_io_sim_step_op(sim, it)) continue;
      sp_io_sim_deliver(sim, it);
      progressed = true;
      break;
    }
  }
}

SP_PRIVATE void sp_io_sim_finish(sp_io_sim_t* sim, u32 at, sp_err_t err) {
  sim->armed[at].op->result.err = err;
  sim->armed[at].op->result.len = 0;
  sp_io_sim_deliver(sim, at);
}

SP_PRIVATE sp_err_t sp_io_sim_submit(void* user_data, sp_io_op_t* op) {
  sp_io_sim_t* sim = (sp_io_sim_t*)user_data;
  sp_assert(sim->armed_count < SP_IO_SIM_MAX_OPS);

  u64 deadline = 0;
  if (op->kind == SP_IO_OP_TIMEOUT) {
    sp_assert(op->timeout.timeout.kind != SP_IO_TIMEOUT_NONE);
    deadline = op->timeout.timeout.kind == SP_IO_TIMEOUT_DURATION
      ? sim->now + op->timeout.timeout.time.ns
      : op->timeout.timeout.time.ns;
  }

  sim->armed[sim->armed_count++] = (sp_io_sim_op_t) {
    .op = op,
    .deadline = deadline,
  };
  return SP_OK;
}

SP_PRIVATE bool sp_io_sim_next_deadline(sp_io_sim_t* sim, u64* out) {
  bool found = false;
  sp_for(it, sim->armed_count) {
    if (sim->armed[it].op->kind != SP_IO_OP_TIMEOUT) continue;
    if (!found || sim->armed[it].deadline < *out) *out = sim->armed[it].deadline;
    found = true;
  }
  return found;
}

SP_PRIVATE sp_err_t sp_io_sim_wait(void* user_data, sp_io_op_t** done, u32 max, sp_io_timeout_t timeout, u32* count) {
  sp_io_sim_t* sim = (sp_io_sim_t*)user_data;

  *count = 0;
  while (true) {
    sp_io_sim_step(sim);
    while (sim->done_count && *count < max) {
      done[(*count)++] = sim->done[0].op;
      sim->done_count--;
      sp_for(it, sim->done_count) {
        sim->done[it] = sim->done[it + 1];
      }
    }
    if (*count) return SP_OK;
    u64 deadline = 0;
    if (!sp_io_sim_next_deadline(sim, &deadline)) {
      sp_assert(timeout.kind != SP_IO_TIMEOUT_NONE);
      return SP_OK;
    }
    sim->now = sp_max(sim->now, deadline);
  }
}

SP_PRIVATE sp_err_t sp_io_sim_cancel(void* user_data, sp_io_op_t* op) {
  sp_io_sim_t* sim = (sp_io_sim_t*)user_data;
  sp_for(it, sim->armed_count) {
    if (sim->armed[it].op == op) {
      sp_io_sim_finish(sim, it, SP_ERR_IO_CANCELED);
      return SP_OK;
    }
  }
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_io_sim_close(void* user_data, sp_sys_socket_t socket) {
  sp_io_sim_t* sim = (sp_io_sim_t*)user_data;

  u32 it = 0;
  while (it < sim->armed_count) {
    sp_io_op_t* op = sim->armed[it].op;
    bool owned =
      (op->kind == SP_IO_OP_ACCEPT && op->accept.socket == socket) ||
      (op->kind == SP_IO_OP_RECV && op->recv.socket == socket) ||
      (op->kind == SP_IO_OP_SEND && op->send.socket == socket);
    if (owned) {
      sp_io_sim_finish(sim, it, SP_ERR_IO_CANCELED);
      continue;
    }
    it++;
  }

  sp_io_sim_listener_t* listener = sp_io_sim_listener(sim, socket);
  if (listener) {
    sp_for(at, listener->backlog_count) {
      u32 end = 0;
      sp_io_sim_conn_t* conn = sp_io_sim_conn(sim, listener->backlog[at], &end);
      conn->reset = true;
    }
    listener->live = false;
    return SP_OK;
  }

  u32 end = 0;
  sp_io_sim_conn_t* conn = sp_io_sim_conn(sim, socket, &end);
  sp_assert(conn);
  conn->open[end] = false;
  if (!conn->open[0] && !conn->open[1]) {
    conn->live = false;
  }
  return SP_OK;
}

SP_PRIVATE sp_err_t sp_io_sim_wake(void* user_data) {
  sp_unused(user_data);
  return SP_OK;
}

SP_PRIVATE sp_io_time_t sp_io_sim_now(void* user_data, sp_io_clock_t clock) {
  sp_io_sim_t* sim = (sp_io_sim_t*)user_data;
  return (sp_io_time_t) { .ns = sim->now, .clock = clock };
}

SP_PRIVATE void sp_io_sim_destroy(void* user_data) {
  sp_unused(user_data);
}

SP_PRIVATE const sp_io_vtable_t sp_io_sim_vtable = {
  .submit  = sp_io_sim_submit,
  .wait    = sp_io_sim_wait,
  .cancel  = sp_io_sim_cancel,
  .close   = sp_io_sim_close,
  .wake    = sp_io_sim_wake,
  .now     = sp_io_sim_now,
  .destroy = sp_io_sim_destroy,
};

void sp_io_sim_init(sp_io_sim_t* sim) {
  *sim = sp_zero_s(sp_io_sim_t);
  sim->next_socket = SP_IO_SIM_SOCKET_BASE;
}

sp_io_t sp_io_sim_actor(sp_io_sim_t* sim) {
  return (sp_io_t) { .user_data = sim, .vt = &sp_io_sim_vtable };
}

sp_sys_socket_t sp_io_sim_listen(sp_io_sim_t* sim, u16 port) {
  sp_assert(!sp_io_sim_listener_at(sim, port));
  sp_carr_for(sim->listeners, it) {
    if (sim->listeners[it].live) continue;
    sim->listeners[it] = (sp_io_sim_listener_t) {
      .socket = sim->next_socket++,
      .port = port,
      .live = true,
    };
    return sim->listeners[it].socket;
  }
  sp_unreachable_return(SP_SYS_INVALID_SOCKET);
}

void sp_io_sim_advance(sp_io_sim_t* sim, u64 ns) {
  sim->now += ns;
}

void sp_io_sim_kill(sp_io_sim_t* sim, sp_sys_socket_t socket) {
  u32 end = 0;
  sp_io_sim_conn_t* conn = sp_io_sim_conn(sim, socket, &end);
  sp_assert(conn);
  conn->reset = true;
  conn->wire[0].len = 0;
  conn->wire[1].len = 0;
}

void sp_io_sim_chunk(sp_io_sim_t* sim, sp_sys_socket_t socket, u64 max) {
  u32 end = 0;
  sp_io_sim_conn_t* conn = sp_io_sim_conn(sim, socket, &end);
  sp_assert(conn);
  conn->chunk = max;
}

SP_PRIVATE SP_THREAD_LOCAL sp_task_t* sp_task_tls_current = SP_NULLPTR;

sp_task_t* sp_task_current(void) {
  return sp_task_tls_current;
}

sp_task_t* sp_task_spawn(sp_task_sched_t sched, sp_task_fn_t fn, void* context) {
  return sched.vt->spawn(sched.user_data, fn, context);
}

void sp_task_run(sp_task_sched_t sched) {
  sched.vt->run(sched.user_data);
}

void sp_task_enter(sp_task_t* task) {
  sp_task_tls_current = task;
  task->fn(task->context);
  sp_task_tls_current = SP_NULLPTR;
  sp_atomic_u32_store(&task->done, 1, SP_ATOMIC_SEQ_CST);
  sp_task_waiter_t* awaiter = (sp_task_waiter_t*)sp_atomic_ptr_exchange(&task->awaiter, SP_NULLPTR, SP_ATOMIC_SEQ_CST);
  if (awaiter) sp_task_notify(awaiter);
}

sp_task_waiter_t sp_task_waiter(void) {
  return (sp_task_waiter_t) { .task = sp_task_current() };
}

SP_PRIVATE void sp_task_waiter_park(sp_task_waiter_t* w) {
  if (w->task) {
    w->task->sched.vt->park(w->task->sched.user_data, w->task);
  }
  else {
    sp_sys_futex_wait(&w->signal, 0, SP_NULLPTR);
  }
}

void sp_task_wait(sp_task_waiter_t* w) {
  while (!sp_atomic_u32_load(&w->signal, SP_ATOMIC_ACQUIRE)) {
    sp_task_waiter_park(w);
  }
}

sp_err_t sp_task_wait_cancelable(sp_task_waiter_t* w) {
  while (!sp_atomic_u32_load(&w->signal, SP_ATOMIC_ACQUIRE)) {
    if (!w->task) {
      sp_sys_futex_wait(&w->signal, 0, SP_NULLPTR);
      continue;
    }
    if (sp_atomic_u32_load(&w->task->cancel_requested, SP_ATOMIC_SEQ_CST)) return SP_ERR_IO_CANCELED;
    sp_atomic_u32_store(&w->task->wait_state, SP_TASK_WAIT_PARK, SP_ATOMIC_SEQ_CST);
    if (sp_atomic_u32_load(&w->task->cancel_requested, SP_ATOMIC_SEQ_CST)) {
      sp_atomic_u32_store(&w->task->wait_state, SP_TASK_WAIT_NONE, SP_ATOMIC_SEQ_CST);
      return SP_ERR_IO_CANCELED;
    }
    sp_task_waiter_park(w);
    sp_atomic_u32_store(&w->task->wait_state, SP_TASK_WAIT_NONE, SP_ATOMIC_SEQ_CST);
  }
  return SP_OK;
}

void sp_task_notify(sp_task_waiter_t* w) {
  sp_task_t* task = w->task;
  sp_atomic_u32_store(&w->signal, 1, SP_ATOMIC_RELEASE);
  if (task) {
    task->sched.vt->unpark(task->sched.user_data, task);
  }
  else {
    sp_sys_futex_wake(&w->signal);
  }
}

void sp_task_await(sp_task_t* task) {
  sp_task_waiter_t w = sp_task_waiter();
  sp_atomic_ptr_store(&task->awaiter, &w, SP_ATOMIC_SEQ_CST);
  if (sp_atomic_u32_load(&task->done, SP_ATOMIC_SEQ_CST)) {
    if (sp_atomic_ptr_exchange(&task->awaiter, SP_NULLPTR, SP_ATOMIC_SEQ_CST) == &w) return;
  }
  sp_task_wait(&w);
}

void sp_task_cancel(sp_task_t* task) {
  sp_atomic_u32_store(&task->cancel_requested, 1, SP_ATOMIC_SEQ_CST);
  u32 state = sp_atomic_u32_load(&task->wait_state, SP_ATOMIC_SEQ_CST);
  if (state == SP_TASK_WAIT_OP) {
    if (sp_atomic_u32_cas(&task->wait_state, SP_TASK_WAIT_OP, SP_TASK_WAIT_LOCKED, SP_ATOMIC_ACQ_REL)) {
      sp_io_cancel(task->wait.io, task->wait.op);
      sp_atomic_u32_store(&task->wait_state, SP_TASK_WAIT_OP, SP_ATOMIC_RELEASE);
    }
  }
  else if (state == SP_TASK_WAIT_PARK) {
    task->sched.vt->unpark(task->sched.user_data, task);
  }
  sp_task_await(task);
}

sp_task_group_t sp_task_group(sp_task_sched_t sched) {
  return (sp_task_group_t) { .sched = sched };
}

sp_task_t* sp_task_group_spawn(sp_task_group_t* g, sp_task_fn_t fn, void* context) {
  sp_assert(g->count < SP_TASK_GROUP_MAX);
  sp_task_t* task = sp_task_spawn(g->sched, fn, context);
  g->tasks[g->count++] = task;
  return task;
}

void sp_task_group_await(sp_task_group_t* g) {
  sp_for(it, g->count) {
    sp_task_await(g->tasks[it]);
  }
  g->count = 0;
}

void sp_task_group_cancel(sp_task_group_t* g) {
  sp_for(it, g->count) {
    sp_atomic_u32_store(&g->tasks[it]->cancel_requested, 1, SP_ATOMIC_SEQ_CST);
  }
  sp_for(it, g->count) {
    sp_task_cancel(g->tasks[it]);
  }
  g->count = 0;
}

SP_PRIVATE void sp_task_waiters_remove(sp_task_waiter_t** list, u32* count, sp_task_waiter_t* w) {
  sp_for(it, *count) {
    if (list[it] != w) continue;
    (*count)--;
    sp_for(at, *count - it) {
      list[it + at] = list[it + at + 1];
    }
    return;
  }
}

SP_PRIVATE void sp_task_waiters_notify_one(sp_task_waiter_t** list, u32* count) {
  if (!*count) return;
  sp_task_waiter_t* w = list[0];
  sp_task_waiters_remove(list, count, w);
  sp_task_notify(w);
}

SP_PRIVATE void sp_task_waiters_notify_all(sp_task_waiter_t** list, u32* count) {
  while (*count) sp_task_waiters_notify_one(list, count);
}

sp_task_queue_t sp_task_queue_init(sp_mem_slice_t buffer, u32 elem_size) {
  return (sp_task_queue_t) { .buffer = buffer, .elem_size = elem_size };
}

void sp_task_queue_close(sp_task_queue_t* q) {
  sp_spin_lock(&q->lock);
  q->closed = true;
  sp_task_waiters_notify_all(q->getters, &q->getter_count);
  sp_task_waiters_notify_all(q->putters, &q->putter_count);
  sp_spin_unlock(&q->lock);
}

SP_PRIVATE sp_err_t sp_task_queue_block(sp_task_queue_t* q, sp_task_waiter_t** list, u32* count) {
  sp_assert(*count < SP_TASK_QUEUE_WAITERS);
  sp_task_waiter_t w = sp_task_waiter();
  list[(*count)++] = &w;
  sp_spin_unlock(&q->lock);
  sp_err_t err = sp_task_wait_cancelable(&w);
  sp_spin_lock(&q->lock);
  sp_task_waiters_remove(list, count, &w);
  if (err && sp_atomic_u32_load(&w.signal, SP_ATOMIC_ACQUIRE)) {
    sp_task_waiters_notify_one(list, count);
  }
  return err;
}

sp_err_t sp_task_queue_put(sp_task_queue_t* q, const void* elem) {
  u64 capacity = q->buffer.len / q->elem_size;
  sp_spin_lock(&q->lock);
  while (q->len == capacity) {
    if (q->closed) {
      sp_spin_unlock(&q->lock);
      return SP_ERR_IO_CLOSED;
    }
    sp_err_t err = sp_task_queue_block(q, q->putters, &q->putter_count);
    if (err) {
      sp_spin_unlock(&q->lock);
      return err;
    }
  }
  if (q->closed) {
    sp_spin_unlock(&q->lock);
    return SP_ERR_IO_CLOSED;
  }
  u64 at = ((q->start + q->len) % capacity) * q->elem_size;
  sp_mem_copy(q->buffer.data + at, elem, q->elem_size);
  q->len++;
  sp_task_waiters_notify_one(q->getters, &q->getter_count);
  sp_spin_unlock(&q->lock);
  return SP_OK;
}

sp_err_t sp_task_queue_get(sp_task_queue_t* q, void* elem) {
  u64 capacity = q->buffer.len / q->elem_size;
  sp_spin_lock(&q->lock);
  while (!q->len) {
    if (q->closed) {
      sp_spin_unlock(&q->lock);
      return SP_ERR_IO_CLOSED;
    }
    sp_err_t err = sp_task_queue_block(q, q->getters, &q->getter_count);
    if (err) {
      sp_spin_unlock(&q->lock);
      return err;
    }
  }
  sp_mem_copy(elem, q->buffer.data + q->start * q->elem_size, q->elem_size);
  q->start = (q->start + 1) % capacity;
  q->len--;
  sp_task_waiters_notify_one(q->putters, &q->putter_count);
  sp_spin_unlock(&q->lock);
  return SP_OK;
}

#if defined(SP_TASK_FIBER_SUPPORTED)

#if defined(SP_MACOS)
  #define SP_TASK_FIBER_SYM "_sp_task_fiber_swap"
  #define SP_TASK_FIBER_TYPE ""
#else
  #define SP_TASK_FIBER_SYM "sp_task_fiber_swap"
  #define SP_TASK_FIBER_TYPE ".type sp_task_fiber_swap, @function\n"
#endif

SP_EXTERN_C void sp_task_fiber_swap(void** save, void* load);

#if defined(SP_AMD64)
__asm__(
  ".text\n"
  ".globl " SP_TASK_FIBER_SYM "\n"
  SP_TASK_FIBER_TYPE
  SP_TASK_FIBER_SYM ":\n"
  "pushq %rbp\n"
  "pushq %rbx\n"
  "pushq %r12\n"
  "pushq %r13\n"
  "pushq %r14\n"
  "pushq %r15\n"
  "movq %rsp, (%rdi)\n"
  "movq %rsi, %rsp\n"
  "popq %r15\n"
  "popq %r14\n"
  "popq %r13\n"
  "popq %r12\n"
  "popq %rbx\n"
  "popq %rbp\n"
  "retq\n"
);
#elif defined(SP_ARM64)
__asm__(
  ".text\n"
  ".globl " SP_TASK_FIBER_SYM "\n"
  SP_TASK_FIBER_TYPE
  SP_TASK_FIBER_SYM ":\n"
  "sub sp, sp, #160\n"
  "stp x19, x20, [sp, #0]\n"
  "stp x21, x22, [sp, #16]\n"
  "stp x23, x24, [sp, #32]\n"
  "stp x25, x26, [sp, #48]\n"
  "stp x27, x28, [sp, #64]\n"
  "stp x29, x30, [sp, #80]\n"
  "stp d8, d9, [sp, #96]\n"
  "stp d10, d11, [sp, #112]\n"
  "stp d12, d13, [sp, #128]\n"
  "stp d14, d15, [sp, #144]\n"
  "mov x9, sp\n"
  "str x9, [x0]\n"
  "mov sp, x1\n"
  "ldp x19, x20, [sp, #0]\n"
  "ldp x21, x22, [sp, #16]\n"
  "ldp x23, x24, [sp, #32]\n"
  "ldp x25, x26, [sp, #48]\n"
  "ldp x27, x28, [sp, #64]\n"
  "ldp x29, x30, [sp, #80]\n"
  "ldp d8, d9, [sp, #96]\n"
  "ldp d10, d11, [sp, #112]\n"
  "ldp d12, d13, [sp, #128]\n"
  "ldp d14, d15, [sp, #144]\n"
  "add sp, sp, #160\n"
  "ret\n"
);
#endif

#else

SP_PRIVATE void sp_task_fiber_swap(void** save, void* load) {
  sp_unused(save);
  sp_unused(load);
  sp_assert(false);
}

#endif

SP_PRIVATE sp_task_t* sp_task_fiber_spawn(void* user_data, sp_task_fn_t fn, void* context);
SP_PRIVATE void       sp_task_fiber_park(void* user_data, sp_task_t* task);
SP_PRIVATE void       sp_task_fiber_unpark(void* user_data, sp_task_t* task);
SP_PRIVATE void       sp_task_fiber_run(void* user_data);

SP_PRIVATE const sp_task_sched_vtable_t sp_task_fiber_vtable = {
  .spawn  = sp_task_fiber_spawn,
  .park   = sp_task_fiber_park,
  .unpark = sp_task_fiber_unpark,
  .run    = sp_task_fiber_run,
};

SP_PRIVATE SP_THREAD_LOCAL sp_task_fiber_t* sp_task_fiber_tls = SP_NULLPTR;

SP_PRIVATE void sp_task_fiber_entry(void) {
  sp_task_fiber_t* f = sp_task_fiber_tls;
  sp_task_t* task = f->running;
  sp_task_enter(task);
  sp_atomic_u32_add(&f->live, (u32)-1, SP_ATOMIC_ACQ_REL);
  sp_task_fiber_swap(&task->sp, f->run_sp);
}

SP_PRIVATE void sp_task_fiber_seed(sp_task_t* task) {
  u8* top = (u8*)(sp_uptr(task->stack.data + task->stack.len) & ~(uintptr_t)15);
#if defined(SP_AMD64)
  void** frame = (void**)(top - 16);
  frame[0] = (void*)sp_uptr(sp_task_fiber_entry);
  void** saved = frame - 6;
  sp_for(it, 6) {
    saved[it] = SP_NULLPTR;
  }
  task->sp = saved;
#elif defined(SP_ARM64)
  u8* frame = top - 160;
  sp_mem_zero(frame, 160);
  *(void**)(frame + 88) = (void*)sp_uptr(sp_task_fiber_entry);
  task->sp = frame;
#else
  sp_unused(top);
  task->sp = SP_NULLPTR;
#endif
}

SP_PRIVATE void sp_task_fiber_ready(sp_task_fiber_t* f, sp_task_t* task) {
  task->next = SP_NULLPTR;
  if (f->ready_tail) f->ready_tail->next = task;
  else f->ready = task;
  f->ready_tail = task;
}

SP_PRIVATE sp_task_t* sp_task_fiber_pop(sp_task_fiber_t* f) {
  sp_task_t* task = f->ready;
  if (!task) return SP_NULLPTR;
  f->ready = task->next;
  if (!f->ready) f->ready_tail = SP_NULLPTR;
  return task;
}

SP_PRIVATE void sp_task_fiber_resume(sp_task_fiber_t* f, sp_task_t* task) {
  f->running = task;
  sp_task_tls_current = task;
  sp_task_fiber_swap(&f->run_sp, task->sp);
  sp_task_tls_current = SP_NULLPTR;
  f->running = SP_NULLPTR;
}

SP_PRIVATE void sp_task_fiber_yield(sp_task_fiber_t* f, sp_task_t* task) {
  sp_task_fiber_swap(&task->sp, f->run_sp);
}

SP_PRIVATE sp_task_t* sp_task_fiber_spawn(void* user_data, sp_task_fn_t fn, void* context) {
  sp_task_fiber_t* f = (sp_task_fiber_t*)user_data;
  sp_assert(f->count < SP_TASK_MAX);
  sp_task_t* task = &f->tasks[f->count++];
  *task = (sp_task_t) {
    .sched = { .user_data = f, .vt = &sp_task_fiber_vtable },
    .fn = fn,
    .context = context,
    .stack = sp_mem_slice(sp_alloc_n(f->mem, u8, SP_TASK_FIBER_STACK_SIZE), SP_TASK_FIBER_STACK_SIZE),
  };
  sp_task_fiber_seed(task);
  sp_atomic_u32_add(&f->live, 1, SP_ATOMIC_RELAXED);
  sp_task_fiber_ready(f, task);
  return task;
}

SP_PRIVATE void sp_task_fiber_park(void* user_data, sp_task_t* task) {
  sp_task_fiber_t* f = (sp_task_fiber_t*)user_data;
  if (sp_atomic_u32_exchange(&task->unparked, 0, SP_ATOMIC_ACQ_REL)) return;
  task->parked = true;
  sp_task_fiber_yield(f, task);
  sp_atomic_u32_exchange(&task->unparked, 0, SP_ATOMIC_ACQ_REL);
}

SP_PRIVATE void sp_task_fiber_unpark(void* user_data, sp_task_t* task) {
  sp_task_fiber_t* f = (sp_task_fiber_t*)user_data;
  sp_atomic_u32_store(&task->unparked, 1, SP_ATOMIC_RELEASE);
  if (sp_thread_get_id() == f->thread_id) {
    if (task->parked) {
      task->parked = false;
      sp_task_fiber_ready(f, task);
    }
    return;
  }
  if (sp_atomic_u32_exchange(&task->inboxed, 1, SP_ATOMIC_ACQ_REL)) return;
  sp_spin_lock(&f->inbox_lock);
  task->inbox_next = f->inbox;
  f->inbox = task;
  sp_spin_unlock(&f->inbox_lock);
  sp_io_wake(f->io);
}

SP_PRIVATE void sp_task_fiber_drain(sp_task_fiber_t* f) {
  sp_spin_lock(&f->inbox_lock);
  sp_task_t* task = f->inbox;
  f->inbox = SP_NULLPTR;
  sp_spin_unlock(&f->inbox_lock);
  while (task) {
    sp_task_t* next = task->inbox_next;
    sp_atomic_u32_store(&task->inboxed, 0, SP_ATOMIC_RELEASE);
    if (task->parked) {
      task->parked = false;
      sp_task_fiber_ready(f, task);
    }
    task = next;
  }
}

SP_PRIVATE void sp_task_fiber_run(void* user_data) {
  sp_task_fiber_t* f = (sp_task_fiber_t*)user_data;
  f->thread_id = sp_thread_get_id();
  sp_task_fiber_tls = f;
  while (true) {
    sp_task_fiber_drain(f);
    sp_task_t* task = SP_NULLPTR;
    while ((task = sp_task_fiber_pop(f))) {
      sp_task_fiber_resume(f, task);
    }
    if (!sp_atomic_u32_load(&f->live, SP_ATOMIC_ACQUIRE)) break;
    sp_assert(sp_io_dispatch(f->io, sp_io_timeout_none(), SP_NULLPTR) == SP_OK);
  }
  sp_task_fiber_tls = SP_NULLPTR;
}

sp_task_sched_t sp_task_fiber_init(sp_task_fiber_t* f, sp_io_t io, sp_mem_t mem) {
  *f = sp_zero_s(sp_task_fiber_t);
  f->io = io;
  f->mem = mem;
  return (sp_task_sched_t) { .user_data = f, .vt = &sp_task_fiber_vtable };
}

void sp_task_fiber_deinit(sp_task_fiber_t* f) {
  sp_for(it, f->count) {
    sp_free(f->mem, f->tasks[it].stack.data, f->tasks[it].stack.len);
  }
  f->count = 0;
}

SP_PRIVATE sp_task_t* sp_task_threaded_spawn(void* user_data, sp_task_fn_t fn, void* context);
SP_PRIVATE void       sp_task_threaded_park(void* user_data, sp_task_t* task);
SP_PRIVATE void       sp_task_threaded_unpark(void* user_data, sp_task_t* task);
SP_PRIVATE void       sp_task_threaded_run(void* user_data);

SP_PRIVATE const sp_task_sched_vtable_t sp_task_threaded_vtable = {
  .spawn  = sp_task_threaded_spawn,
  .park   = sp_task_threaded_park,
  .unpark = sp_task_threaded_unpark,
  .run    = sp_task_threaded_run,
};

SP_PRIVATE s32 sp_task_threaded_main(void* user_data) {
  sp_task_t* task = (sp_task_t*)user_data;
  sp_task_threaded_t* t = (sp_task_threaded_t*)task->sched.user_data;
  sp_task_enter(task);
  sp_atomic_u32_add(&t->live, (u32)-1, SP_ATOMIC_ACQ_REL);
  sp_io_wake(t->io);
  return 0;
}

SP_PRIVATE sp_task_t* sp_task_threaded_spawn(void* user_data, sp_task_fn_t fn, void* context) {
  sp_task_threaded_t* t = (sp_task_threaded_t*)user_data;
  sp_assert(t->count < SP_TASK_MAX);
  sp_task_t* task = &t->tasks[t->count++];
  *task = (sp_task_t) {
    .sched = { .user_data = t, .vt = &sp_task_threaded_vtable },
    .fn = fn,
    .context = context,
  };
  sp_atomic_u32_add(&t->live, 1, SP_ATOMIC_ACQ_REL);
  sp_thread_init(&task->thread, sp_task_threaded_main, task);
  return task;
}

SP_PRIVATE void sp_task_threaded_park(void* user_data, sp_task_t* task) {
  sp_unused(user_data);
  while (!sp_atomic_u32_exchange(&task->unparked, 0, SP_ATOMIC_ACQ_REL)) {
    sp_sys_futex_wait(&task->unparked, 0, SP_NULLPTR);
  }
}

SP_PRIVATE void sp_task_threaded_unpark(void* user_data, sp_task_t* task) {
  sp_unused(user_data);
  sp_atomic_u32_store(&task->unparked, 1, SP_ATOMIC_RELEASE);
  sp_sys_futex_wake(&task->unparked);
}

SP_PRIVATE void sp_task_threaded_run(void* user_data) {
  sp_task_threaded_t* t = (sp_task_threaded_t*)user_data;
  while (sp_atomic_u32_load(&t->live, SP_ATOMIC_ACQUIRE)) {
    sp_assert(sp_io_dispatch(t->io, sp_io_timeout_none(), SP_NULLPTR) == SP_OK);
  }
  sp_for(it, t->count) {
    sp_thread_join(&t->tasks[it].thread);
  }
}

sp_task_sched_t sp_task_threaded_init(sp_task_threaded_t* t, sp_io_t io) {
  *t = sp_zero_s(sp_task_threaded_t);
  t->io = io;
  return (sp_task_sched_t) { .user_data = t, .vt = &sp_task_threaded_vtable };
}

SP_PRIVATE void sp_io_op_notify(sp_io_t io, sp_io_op_t* op) {
  sp_unused(io);
  sp_task_notify((sp_task_waiter_t*)op->user_data);
}

sp_err_t sp_io_op_run(sp_io_t io, sp_io_op_t* op) {
  sp_task_waiter_t w = sp_task_waiter();
  op->callback = sp_io_op_notify;
  op->user_data = &w;
  if (w.task) {
    if (sp_atomic_u32_load(&w.task->cancel_requested, SP_ATOMIC_SEQ_CST)) return SP_ERR_IO_CANCELED;
    w.task->wait.io = io;
    w.task->wait.op = op;
    sp_atomic_u32_store(&w.task->wait_state, SP_TASK_WAIT_OP, SP_ATOMIC_SEQ_CST);
  }
  sp_err_t err = sp_io_submit(io, op);
  if (err) {
    if (w.task) {
      while (!sp_atomic_u32_cas(&w.task->wait_state, SP_TASK_WAIT_OP, SP_TASK_WAIT_NONE, SP_ATOMIC_ACQ_REL)) {}
    }
    return err;
  }
  if (w.task && sp_atomic_u32_load(&w.task->cancel_requested, SP_ATOMIC_SEQ_CST)) {
    sp_io_cancel(io, op);
  }
  sp_task_wait(&w);
  if (w.task) {
    while (!sp_atomic_u32_cas(&w.task->wait_state, SP_TASK_WAIT_OP, SP_TASK_WAIT_NONE, SP_ATOMIC_ACQ_REL)) {}
  }
  return SP_OK;
}

sp_err_t sp_io_connect(sp_io_t io, sp_sys_ipv4_t addr, sp_sys_socket_t* out) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_CONNECT,
    .connect = { .addr = addr },
  };
  sp_try(sp_io_op_run(io, &op));
  sp_try(op.result.err);
  *out = op.result.socket;
  return SP_OK;
}

sp_err_t sp_io_accept(sp_io_t io, sp_sys_socket_t listener, sp_sys_socket_t* out) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_ACCEPT,
    .accept = { .socket = listener },
  };
  sp_try(sp_io_op_run(io, &op));
  sp_try(op.result.err);
  *out = op.result.socket;
  return SP_OK;
}

sp_err_t sp_io_recv(sp_io_t io, sp_sys_socket_t socket, sp_mem_slice_t buf, u64* out_len) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_RECV,
    .recv = { .socket = socket, .buf = buf },
  };
  sp_try(sp_io_op_run(io, &op));
  sp_try(op.result.err);
  *out_len = op.result.len;
  return SP_OK;
}

sp_err_t sp_io_send_all(sp_io_t io, sp_sys_socket_t socket, sp_mem_slice_t buf) {
  u64 sent = 0;
  while (sent < buf.len) {
    sp_io_op_t op = {
      .kind = SP_IO_OP_SEND,
      .send = { .socket = socket, .buf = sp_mem_slice(buf.data + sent, buf.len - sent) },
    };
    sp_try(sp_io_op_run(io, &op));
    sp_try(op.result.err);
    sent += op.result.len;
  }
  return SP_OK;
}

sp_err_t sp_io_fd_read(sp_io_t io, sp_sys_fd_t fd, sp_mem_slice_t buf, u64* out_len) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_READ,
    .read = { .fd = fd, .buf = buf },
  };
  sp_try(sp_io_op_run(io, &op));
  sp_try(op.result.err);
  *out_len = op.result.len;
  return SP_OK;
}

sp_err_t sp_io_fd_write_all(sp_io_t io, sp_sys_fd_t fd, sp_mem_slice_t buf) {
  u64 written = 0;
  while (written < buf.len) {
    sp_io_op_t op = {
      .kind = SP_IO_OP_WRITE,
      .write = { .fd = fd, .buf = sp_mem_slice(buf.data + written, buf.len - written) },
    };
    sp_try(sp_io_op_run(io, &op));
    sp_try(op.result.err);
    written += op.result.len;
  }
  return SP_OK;
}

sp_err_t sp_io_sleep(sp_io_t io, u64 ns) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_TIMEOUT,
    .timeout = { .timeout = sp_io_timeout_after(ns) },
  };
  sp_try(sp_io_op_run(io, &op));
  return op.result.err;
}

sp_err_t sp_io_work(sp_io_t io, sp_io_work_fn_t fn, void* context) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_WORK,
    .work = { .fn = fn, .context = context },
  };
  sp_try(sp_io_op_run(io, &op));
  return op.result.err;
}

sp_err_t sp_io_is_tty(sp_io_t io, sp_sys_fd_t fd, bool* out) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_IS_TTY,
    .is_tty = { .fd = fd },
  };
  sp_try(sp_io_op_run(io, &op));
  sp_try(op.result.err);
  *out = op.result.flag;
  return SP_OK;
}

sp_err_t sp_io_tty_get(sp_io_t io, sp_sys_fd_t fd, sp_sys_tty_attr_t* attr) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_TTY_GET,
    .tty_get = { .fd = fd, .attr = attr },
  };
  sp_try(sp_io_op_run(io, &op));
  return op.result.err;
}

sp_err_t sp_io_tty_set(sp_io_t io, sp_sys_fd_t fd, const sp_sys_tty_attr_t* attr) {
  sp_io_op_t op = {
    .kind = SP_IO_OP_TTY_SET,
    .tty_set = { .fd = fd, .attr = attr },
  };
  sp_try(sp_io_op_run(io, &op));
  return op.result.err;
}

#endif
