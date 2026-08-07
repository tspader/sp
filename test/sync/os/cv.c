#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define MAX_THREADS 8

typedef struct {
  const c8* name;
  u32 waiters;
} test_t;

static const test_t tests [] = {
  { .name = "notify_all_wakes_every_waiter", .waiters = 4 },
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

sp_test_each(sync, cv, test_t, tests) {
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

  sp_mutex_lock(&mutex);
  state.tickets = it->waiters;
  sp_mutex_unlock(&mutex);
  sp_cv_notify_all(&cv);

  sp_for(n, it->waiters) {
    sp_thread_join(&threads[n]);
  }

  sp_expect_eq(t, sp_atomic_u32_load(&state.done, SP_ATOMIC_ACQUIRE), it->waiters);
  sp_expect_eq(t, state.tickets, (u32)0);
  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
  return SP_OK;
#endif
}

#endif
