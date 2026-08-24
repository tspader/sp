#include "harness.h"

#define WAKE_MAX_STEPS 6

typedef enum {
  STEP_NONE,
  STEP_WAKE,
  STEP_SPAWN,
  STEP_SUBMIT,
  STEP_WAIT,
} step_kind_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { u32 delay_ms; } spawn;
    ops_submit_t submit;
    struct { ops_wait_t inner; bool woken; } wait;
  };
} step_t;

typedef struct {
  const c8* name;
  ops_fixture_t fixture;
  step_t steps [WAKE_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "thread_wake_breaks_unbounded_wait",
    .steps = {
      { .kind = STEP_SPAWN, .spawn = { .delay_ms = 50 } },
      { .kind = STEP_WAIT, .wait = { .inner = { .kind = WAIT_NONE }, .woken = true } },
    },
  },
  {
    .name = "wake_before_wait_returns_at_once",
    .steps = {
      { .kind = STEP_WAKE },
      { .kind = STEP_WAIT, .wait = { .inner = { .kind = WAIT_NONE } } },
    },
  },
  {
    .name = "wakes_coalesce",
    .steps = {
      { .kind = STEP_WAKE },
      { .kind = STEP_WAKE },
      { .kind = STEP_WAIT, .wait = { .inner = { .kind = WAIT_NONE } } },
      { .kind = STEP_WAIT, .wait = { .inner = { .kind = WAIT_AFTER, .ms = 50, .expect = { .min_elapsed_ms = 50 } } } },
    },
  },
  {
    .name = "wake_does_not_disturb_completions_in_same_wait",
    .fixture = FIXTURE_SOCKETS,
    .steps = {
      { .kind = STEP_SUBMIT, .submit = { .op = SP_IO_OP_SEND, .handle = H_SERVER, .data = "A" } },
      { .kind = STEP_WAKE },
      { .kind = STEP_WAIT, .wait = { .inner = { .kind = WAIT_NONE, .expect = { .count = 1, .done = { { .len = 1 } } } } } },
    },
  },
};

typedef struct {
  sp_io_t io;
  u32 delay_ms;
  sp_atomic_s32_t woken;
} waker_t;

static s32 waker(void* user_data) {
  waker_t* w = (waker_t*)user_data;
  sp_os_sleep_ms(w->delay_ms);
  sp_atomic_s32_store(&w->woken, 1, SP_ATOMIC_SEQ_CST);
  sp_io_wake(w->io);
  return 0;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  ops_harness_t h;
  sp_try(harness_open(t, &h, OPS_ENTRIES, c->fixture));

  waker_t w = { .io = h.io };
  sp_thread_t thread = sp_zero;
  bool spawned = false;

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_WAKE: {
        sp_expect_ok(t, sp_io_wake(h.io));
        break;
      }
      case STEP_SPAWN: {
#if defined(SP_FREESTANDING)
        harness_close(&h);
        return sp_test_skip(t, "threads are unsupported on freestanding");
#else
        w.delay_ms = step->spawn.delay_ms;
        sp_thread_init(&thread, waker, &w);
        spawned = true;
        break;
#endif
      }
      case STEP_SUBMIT: {
        sp_expect_ok(t, harness_submit(t, &h, &step->submit));
        break;
      }
      case STEP_WAIT: {
        harness_wait(t, &h, &step->wait.inner);
        if (step->wait.woken) {
          sp_expect_eq(t, sp_atomic_s32_load(&w.woken, SP_ATOMIC_SEQ_CST), (s32)1);
        }
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  if (spawned) sp_thread_join(&thread);
  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, wake, test_t, tests, run);
