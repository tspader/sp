#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

typedef enum {
  CV,
  SEM,
} kind_t;

typedef struct {
  bool signaled;
  u64 min_elapsed_ns;
} expect_t;

typedef struct {
  const c8* name;
  kind_t kind;
  u32 ms;
  bool signal;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "cv_wait_for_times_out",
    .kind = CV,
    .ms = 100,
    .expect = { .min_elapsed_ns = 75 * SP_TM_MS_TO_NS },
  },
  {
    .name = "cv_wait_for_wakes_on_notify",
    .kind = CV,
    .ms = 5000,
    .signal = true,
    .expect = { .signaled = true },
  },
  {
    .name = "sem_wait_for_times_out",
    .kind = SEM,
    .ms = 100,
    .expect = { .min_elapsed_ns = 75 * SP_TM_MS_TO_NS },
  },
  {
    .name = "sem_wait_for_wakes_on_signal",
    .kind = SEM,
    .ms = 5000,
    .signal = true,
    .expect = { .signaled = true },
  },
};

typedef struct {
  sp_cv_t* cv;
  sp_mutex_t* mutex;
  sp_semaphore_t* semaphore;
  kind_t kind;
  bool flag;
  sp_atomic_u32_t waiting;
} state_t;

static s32 signaler(void* userdata) {
  state_t* state = (state_t*)userdata;
  switch (state->kind) {
    case CV: {
      while (sp_atomic_u32_load(&state->waiting, SP_ATOMIC_ACQUIRE) == 0) {
        sp_os_sleep_ms(1);
      }
      sp_mutex_lock(state->mutex);
      state->flag = true;
      sp_mutex_unlock(state->mutex);
      sp_cv_notify_one(state->cv);
      break;
    }
    case SEM: {
      sp_semaphore_signal(state->semaphore);
      break;
    }
  }
  return 0;
}

sp_test_each(sync, timed, test_t, tests) {
#if defined(SP_FREESTANDING)
  return sp_test_skip(t, "threads are unsupported on freestanding");
#else
  sp_cv_t cv;
  sp_mutex_t mutex;
  sp_semaphore_t semaphore;
  sp_cv_init(&cv);
  sp_mutex_init(&mutex);
  sp_semaphore_init(&semaphore);

  state_t state = {
    .cv = &cv,
    .mutex = &mutex,
    .semaphore = &semaphore,
    .kind = it->kind,
  };
  sp_thread_t thread = sp_zero;
  if (it->signal) {
    sp_thread_init(&thread, signaler, &state);
  }

  bool signaled = false;
  sp_tm_timer_t timer = sp_tm_start_timer();
  switch (it->kind) {
    case CV: {
      sp_mutex_lock(&mutex);
      sp_atomic_u32_store(&state.waiting, 1, SP_ATOMIC_RELEASE);
      bool ok = true;
      while (!state.flag && ok) {
        ok = sp_cv_wait_for(&cv, &mutex, it->ms);
      }
      signaled = state.flag;
      sp_mutex_unlock(&mutex);
      break;
    }
    case SEM: {
      signaled = sp_semaphore_wait_for(&semaphore, it->ms);
      break;
    }
  }
  u64 elapsed = sp_tm_read_timer(&timer);

  if (it->signal) {
    sp_thread_join(&thread);
  }

  sp_expect_eq(t, signaled, it->expect.signaled);
  sp_expect_ge(t, elapsed, it->expect.min_elapsed_ns);
  sp_cv_destroy(&cv);
  sp_mutex_destroy(&mutex);
  sp_semaphore_destroy(&semaphore);
  return SP_OK;
#endif
}

#endif
