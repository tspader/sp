#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define MAX_THREADS 8

typedef struct {
  const c8* name;
  u32 pre;
  u32 waiters;
  u32 post;
} test_t;

static const test_t tests [] = {
  { .name = "signals_before_waiters", .pre = 4, .waiters = 4 },
  { .name = "signals_after_waiters", .waiters = 4, .post = 4 },
  { .name = "signals_straddle_waiters", .pre = 2, .waiters = 4, .post = 2 },
};

typedef struct {
  sp_semaphore_t* semaphore;
  sp_atomic_u32_t done;
} state_t;

static s32 worker(void* userdata) {
  state_t* state = (state_t*)userdata;
  sp_semaphore_wait(state->semaphore);
  sp_atomic_u32_add(&state->done, 1, SP_ATOMIC_RELEASE);
  return 0;
}

sp_test_each(sync, semaphore, test_t, tests) {
#if defined(SP_FREESTANDING)
  return sp_test_skip(t, "threads are unsupported on freestanding");
#else
  sp_semaphore_t semaphore;
  sp_semaphore_init(&semaphore);

  state_t state = {
    .semaphore = &semaphore,
  };
  sp_for(n, it->pre) {
    sp_semaphore_signal(&semaphore);
  }
  sp_thread_t threads [MAX_THREADS];
  sp_for(n, it->waiters) {
    sp_thread_init(&threads[n], worker, &state);
  }
  sp_for(n, it->post) {
    sp_semaphore_signal(&semaphore);
  }
  sp_for(n, it->waiters) {
    sp_thread_join(&threads[n]);
  }

  sp_expect_eq(t, sp_atomic_u32_load(&state.done, SP_ATOMIC_ACQUIRE), it->waiters);
  sp_semaphore_destroy(&semaphore);
  return SP_OK;
#endif
}

#endif
