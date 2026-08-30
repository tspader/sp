#include "sp.h"
#include "sp/sp_test.h"
#include "sp/sp_io.h"

#define TASK_CANCEL_MAX_TASKS 4

typedef struct {
  const c8* name;
  u32 tasks;
  bool group;
} cancel_test_t;

static const cancel_test_t cancel_tests [] = {
  { .name = "parked_getter_returns_canceled", .tasks = 1 },
  { .name = "group_cancels_every_task", .tasks = 3, .group = true },
};

typedef struct {
  sp_task_queue_t queue;
  sp_err_t errs [TASK_CANCEL_MAX_TASKS];
  sp_atomic_u32_t at;
  u32 storage [TASK_CANCEL_MAX_TASKS];
} cancel_state_t;

static void cancel_getter(void* context) {
  cancel_state_t* s = sp_cast(cancel_state_t*, context);
  u32 value = 0;
  sp_err_t err = sp_task_queue_get(&s->queue, &value);
  s->errs[sp_atomic_u32_add(&s->at, 1, SP_ATOMIC_ACQ_REL)] = err;
}

static sp_err_t run_cancel_test(sp_test_t* t, cancel_test_t* c) {
  sp_test_skip_on_freestanding();

  sp_io_blocking_t io_mem;
  sp_io_t io = sp_io_blocking_init(&io_mem);
  sp_task_threaded_t* sched_mem = sp_alloc_type(sp_test_arena(t), sp_task_threaded_t);
  sp_task_sched_t sched = sp_task_threaded_init(sched_mem, io);

  cancel_state_t s = sp_zero;
  s.queue = sp_task_queue_init(sp_mem_slice((u8*)s.storage, sizeof(s.storage)), sizeof(u32));

  if (c->group) {
    sp_task_group_t group = sp_task_group(sched);
    sp_for(it, c->tasks) {
      sp_task_group_spawn(&group, cancel_getter, &s);
    }
    sp_task_group_cancel(&group);
  }
  else {
    sp_for(it, c->tasks) {
      sp_task_cancel(sp_task_spawn(sched, cancel_getter, &s));
    }
  }
  sp_task_run(sched);

  sp_must_eq(t, sp_atomic_u32_load(&s.at, SP_ATOMIC_ACQUIRE), c->tasks);
  sp_for(it, c->tasks) {
    sp_expect_err_eq(t, s.errs[it], SP_ERR_IO_CANCELED);
  }
  return SP_OK;
}

sp_test_each_fn(task, cancel, cancel_test_t, cancel_tests, run_cancel_test);

#if defined(SP_TASK_FIBER_SUPPORTED)

typedef struct {
  sp_io_t io;
  sp_err_t err;
  sp_task_t* sleeper;
} fiber_cancel_t;

static void fiber_cancel_sleeper(void* context) {
  fiber_cancel_t* fc = sp_cast(fiber_cancel_t*, context);
  fc->err = sp_io_sleep(fc->io, sp_tm_ms_to_ns(2000));
}

static void fiber_cancel_canceller(void* context) {
  fiber_cancel_t* fc = sp_cast(fiber_cancel_t*, context);
  sp_task_cancel(fc->sleeper);
}

sp_test(task, cancel_kicks_inflight_op) {
  sp_io_uring_t ring;
  sp_err_t err = sp_io_uring_init(&ring, 8);
  if (err == SP_ERR_SYS_UNSUPPORTED || err == SP_ERR_SYS_ACCESS_DENIED) {
    return sp_test_skip(t, "io_uring unavailable");
  }
  sp_must_ok(t, err);
  sp_io_t io = sp_io_uring_as_io(&ring);

  sp_task_fiber_t* f = sp_alloc_type(sp_test_arena(t), sp_task_fiber_t);
  sp_task_sched_t sched = sp_task_fiber_init(f, io, sp_test_arena(t));

  fiber_cancel_t fc = { .io = io };
  fc.sleeper = sp_task_spawn(sched, fiber_cancel_sleeper, &fc);
  sp_task_spawn(sched, fiber_cancel_canceller, &fc);
  sp_task_run(sched);

  sp_task_fiber_deinit(f);
  sp_io_uring_deinit(&ring);
  sp_expect_err_eq(t, fc.err, SP_ERR_IO_CANCELED);
  return SP_OK;
}

#endif
