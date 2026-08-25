#include "harness.h"

#if !defined(SP_WASM)

typedef enum {
  CV,
  SEM,
} kind_t;

typedef struct {
  bool signaled;
  u32 waits;
} expect_t;

typedef struct {
  const c8* name;
  kind_t kind;
  u32 ms;
  sync_wait_t script [SYNC_MAX_WAITS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "cv_wait_for_times_out",
    .kind = CV,
    .ms = 100,
    .script = {
      { .timed_out = true },
    },
    .expect = { .waits = 1 },
  },
  {
    .name = "cv_wait_for_returns_on_notify",
    .kind = CV,
    .ms = 100,
    .script = {
      { .set = true, .to = 1 },
    },
    .expect = { .signaled = true, .waits = 1 },
  },
  {
    .name = "cv_wait_for_surfaces_spurious_wake",
    .kind = CV,
    .ms = 100,
    .expect = { .signaled = true, .waits = 1 },
  },
  {
    .name = "sem_wait_for_times_out",
    .kind = SEM,
    .ms = 100,
    .script = {
      { .timed_out = true },
    },
    .expect = { .waits = 1 },
  },
  {
    .name = "sem_wait_for_returns_on_signal",
    .kind = SEM,
    .ms = 100,
    .script = {
      { .set = true, .to = 1 },
    },
    .expect = { .signaled = true, .waits = 1 },
  },
  {
    .name = "sem_wait_for_rearms_with_remaining_time",
    .kind = SEM,
    .ms = 100,
    .script = {
      { .sleep_ms = 10 },
      { .timed_out = true },
    },
    .expect = { .waits = 2 },
  },
};

sp_test_each(sync, timed_shell, test_t, tests, .serial = true) {

  sync_futex_begin(t, it->script, it->kind == SEM ? 1 : 0);

  bool signaled = false;
  switch (it->kind) {
    case CV: {
      sp_cv_t cv = sp_zero;
      sp_mutex_t mutex = sp_zero;
      sp_mutex_lock(&mutex);
      signaled = sp_cv_wait_for(&cv, &mutex, it->ms);
      sp_mutex_unlock(&mutex);
      break;
    }
    case SEM: {
      sp_semaphore_t semaphore = sp_zero;
      signaled = sp_semaphore_wait_for(&semaphore, it->ms);
      break;
    }
  }

  sync_futex_end();

  sp_expect_eq(t, signaled, it->expect.signaled);
  sync_futex_verify(t, (sync_expect_t) { .waits = it->expect.waits, .timed = true });
  sp_for(w, sync_futex.waits) {
    sp_expect_le(t, sync_futex.timeouts_ns[w], (u64)it->ms * SP_TM_MS_TO_NS);
  }
  if (sync_futex.waits) {
    sp_expect_ge(t, sync_futex.timeouts_ns[0], it->ms ? (u64)(it->ms - 1) * SP_TM_MS_TO_NS : 0);
  }
  for (u32 w = 1; w < sync_futex.waits; w++) {
    sp_expect_le(t, sync_futex.timeouts_ns[w], sync_futex.timeouts_ns[w - 1] - 5 * SP_TM_MS_TO_NS);
    sp_expect_ge(t, sync_futex.timeouts_ns[w], sync_futex.timeouts_ns[w - 1] - 60 * SP_TM_MS_TO_NS);
  }
  return SP_OK;
}

#endif
