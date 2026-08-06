#include "harness.h"

#if !defined(SP_WASM)

#define SYS_FUTEX_MAX_STEPS 4
#define SYS_FUTEX_MAX_WAITERS 8
#define SYS_FUTEX_WAKE_ATTEMPTS 5000

typedef enum {
  SYS_FUTEX_STEP_NONE,
  SYS_FUTEX_STEP_WAIT,
  SYS_FUTEX_STEP_WAKE,
  SYS_FUTEX_STEP_SPAWN,
  SYS_FUTEX_STEP_SET,
  SYS_FUTEX_STEP_AWAIT,
} sys_futex_step_kind_t;

typedef struct {
  sys_futex_step_kind_t kind;
  union {
    struct { u32 expected; u64 timeout_ns; bool timed_out; u64 min_elapsed_ns; } wait;
    struct { bool all; } wake;
    struct { u32 count; } spawn;
    struct { u32 value; } set;
    struct { u32 count; bool all; } await;
  };
} sys_futex_step_t;

typedef struct {
  const c8* name;
  u32 initial;
  sys_futex_step_t steps [SYS_FUTEX_MAX_STEPS];
} sys_futex_case_t;

static const sys_futex_case_t sys_futex_cases [] = {
  {
    .name = "mismatched_value_returns_immediately",
    .initial = 1,
    .steps = {
      {
        .kind = SYS_FUTEX_STEP_WAIT,
        .wait = { .timeout_ns = 200 * SP_TM_MS_TO_NS }
      },
    },
  },
  {
    .name = "timeout_expires_after_requested_duration",
    .steps = {
      {
        .kind = SYS_FUTEX_STEP_WAIT,
        .wait = {
          .timeout_ns = 20 * SP_TM_MS_TO_NS,
          .timed_out = true,
          .min_elapsed_ns = 20 * SP_TM_MS_TO_NS
        }
      },
    },
  },
  {
    .name = "wake_with_no_waiters",
    .steps = {
      { .kind = SYS_FUTEX_STEP_WAKE, .wake = { .all = false } },
    },
  },
  {
    .name = "wake_all_with_no_waiters",
    .steps = {
      { .kind = SYS_FUTEX_STEP_WAKE, .wake = { .all = true } },
    },
  },
  {
    .name = "wake_wakes_waiter",
    .steps = {
      { .kind = SYS_FUTEX_STEP_SPAWN, .spawn = { .count = 1 } },
      { .kind = SYS_FUTEX_STEP_SET, .set = { .value = 1 } },
      { .kind = SYS_FUTEX_STEP_AWAIT, .await = { .count = 1, .all = false } },
    },
  },
  {
    .name = "wake_all_wakes_every_waiter",
    .steps = {
      { .kind = SYS_FUTEX_STEP_SPAWN, .spawn = { .count = 4 } },
      { .kind = SYS_FUTEX_STEP_SET, .set = { .value = 1 } },
      { .kind = SYS_FUTEX_STEP_AWAIT, .await = { .count = 4, .all = true } },
    },
  },
};

typedef struct {
  sp_atomic_u32_t* futex;
  sp_atomic_u32_t* woken;
} sys_futex_waiter_t;

static s32 worker(void* userdata) {
  sys_futex_waiter_t* waiter = (sys_futex_waiter_t*)userdata;
  while (sp_atomic_u32_load(waiter->futex, SP_ATOMIC_ACQUIRE) == 0) {
    sp_sys_futex_wait(waiter->futex, 0, SP_NULLPTR);
  }
  sp_atomic_u32_add(waiter->woken, 1, SP_ATOMIC_ACQ_REL);
  return 0;
}

sp_test_each(sys, futex, sys_futex_case_t, sys_futex_cases) {
  sp_atomic_u32_t futex = it->initial;
  sp_atomic_u32_t woken = 0;
  sys_futex_waiter_t waiter = { .futex = &futex, .woken = &woken };
  sp_thread_t threads [SYS_FUTEX_MAX_WAITERS];
  u32 spawned = 0;

  sp_carr_for(it->steps, s) {
    const sys_futex_step_t* step = &it->steps[s];
    if (step->kind == SYS_FUTEX_STEP_NONE) break;

    switch (step->kind) {
      case SYS_FUTEX_STEP_NONE: {
        break;
      }
      case SYS_FUTEX_STEP_WAIT: {
        sp_sys_timespec_t timeout = {
          .tv_sec = (s64)(step->wait.timeout_ns / SP_TM_S_TO_NS),
          .tv_nsec = (s64)(step->wait.timeout_ns % SP_TM_S_TO_NS),
        };
        sp_sys_timespec_t begin = sp_zero;
        sp_sys_timespec_t end = sp_zero;
        sp_try(sp_sys_clock_gettime(SP_CLOCK_MONOTONIC, &begin));
        bool woken = sp_sys_futex_wait(&futex, step->wait.expected, &timeout);
        sp_try(sp_sys_clock_gettime(SP_CLOCK_MONOTONIC, &end));

        sp_expect_eq(t, woken, !step->wait.timed_out);
        s64 elapsed = ((end.tv_sec - begin.tv_sec) * (s64)SP_TM_S_TO_NS) + (end.tv_nsec - begin.tv_nsec);
        sp_expect_ge(t, elapsed, (s64)step->wait.min_elapsed_ns);
        break;
      }
      case SYS_FUTEX_STEP_WAKE: {
        if (step->wake.all) {
          sp_sys_futex_wake_all(&futex);
        }
        else {
          sp_sys_futex_wake(&futex);
        }
        break;
      }
      case SYS_FUTEX_STEP_SPAWN: {
#if defined(SP_FREESTANDING)
        return sp_test_skip(t, "threads are unsupported on freestanding");
#else
        sp_for(w, step->spawn.count) {
          sp_thread_init(&threads[spawned++], worker, &waiter);
        }
        break;
#endif
      }
      case SYS_FUTEX_STEP_SET: {
        sp_atomic_u32_store(&futex, step->set.value, SP_ATOMIC_RELEASE);
        break;
      }
      case SYS_FUTEX_STEP_AWAIT: {
        sp_sys_timespec_t poll = { .tv_nsec = (s64)SP_TM_MS_TO_NS };
        sp_for(attempt, SYS_FUTEX_WAKE_ATTEMPTS) {
          if (sp_atomic_u32_load(&woken, SP_ATOMIC_ACQUIRE) == step->await.count) break;
          if (step->await.all) {
            sp_sys_futex_wake_all(&futex);
          }
          else {
            sp_sys_futex_wake(&futex);
          }
          sp_sys_nanosleep(&poll, SP_NULLPTR);
        }
        if (sp_atomic_u32_load(&woken, SP_ATOMIC_ACQUIRE) != step->await.count) {
          sp_test_fail(t, "woke {} of {} waiters", sp_fmt_uint(sp_atomic_u32_load(&woken, SP_ATOMIC_ACQUIRE)), sp_fmt_uint(step->await.count));
          return SP_OK;
        }
        break;
      }
    }
  }

  sp_for(w, spawned) {
    sp_thread_join(&threads[w]);
  }
  return SP_OK;
}

#endif
