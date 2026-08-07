#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define MAX_THREADS 8
#define AWAIT_ATTEMPTS 5000

typedef struct {
  const c8* name;
  u32 waiters;
} test_t;

static const test_t tests [] = {
  { .name = "notify_one_makes_progress", .waiters = 4 },
};

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  u32 tickets;
  sp_atomic_u32_t done;
} state_t;

static s32 worker(void* userdata) {
  state_t* state = (state_t*)userdata;
  sp_mutex_lock(state->mutex);
  while (state->tickets == 0) {
    sp_cv_wait(state->cv, state->mutex);
  }
  state->tickets--;
  sp_mutex_unlock(state->mutex);
  sp_atomic_u32_add(&state->done, 1, SP_ATOMIC_RELEASE);
  return 0;
}

sp_test_each(sync, cv_one, test_t, tests) {
#if defined(SP_FREESTANDING)
  return sp_test_skip(t, "threads are unsupported on freestanding");
#else
  sp_cv_t cv;
  sp_mutex_t mutex;
  sp_cv_init(&cv);
  sp_mutex_init(&mutex);

  state_t state = {
    .cv = &cv,
    .mutex = &mutex,
  };
  sp_thread_t threads [MAX_THREADS];
  sp_for(n, it->waiters) {
    sp_thread_init(&threads[n], worker, &state);
  }

  bool released_early = false;
  sp_for(i, it->waiters) {
    sp_mutex_lock(&mutex);
    state.tickets = 1;
    sp_mutex_unlock(&mutex);
    sp_cv_notify_one(&cv);

    u32 done = 0;
    sp_for(attempt, AWAIT_ATTEMPTS) {
      done = sp_atomic_u32_load(&state.done, SP_ATOMIC_ACQUIRE);
      if (done == i + 1) break;
      sp_os_sleep_ms(1);
    }
    sp_expect_eq(t, done, i + 1);
    if (done != i + 1) {
      sp_mutex_lock(&mutex);
      state.tickets = it->waiters;
      sp_mutex_unlock(&mutex);
      sp_cv_notify_all(&cv);
      released_early = true;
      break;
    }
  }

  sp_for(n, it->waiters) {
    sp_thread_join(&threads[n]);
  }

  if (!released_early) {
    sp_expect_eq(t, sp_atomic_u32_load(&state.done, SP_ATOMIC_ACQUIRE), it->waiters);
    sp_expect_eq(t, state.tickets, (u32)0);
  }
  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
  return SP_OK;
#endif
}

#endif
