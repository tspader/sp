#include "harness.h"

#if !defined(SP_WASM)

typedef struct {
  const c8* name;
  u32 signals;
  bool wait;
  sync_wait_t script [SYNC_MAX_WAITS];
  sync_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "signal_wakes",
    .signals = 1,
    .expect = { .wakes = 1 },
  },
  {
    .name = "wait_consumes_signal_without_sleeping",
    .signals = 1,
    .wait = true,
    .expect = { .wakes = 1 },
  },
  {
    .name = "wait_sleeps_until_signaled",
    .wait = true,
    .script = {
      { .set = true, .to = 1 },
    },
    .expect = { .waits = 1 },
  },
  {
    .name = "spurious_wake_returns_to_waiting",
    .wait = true,
    .script = {
      [1] = { .set = true, .to = 1 },
    },
    .expect = { .waits = 2 },
  },
};

sp_test_each(sync, semaphore_shell, test_t, tests, .serial = true) {

  sync_futex_begin(t, it->script, 1);

  sp_semaphore_t semaphore = sp_zero;
  sp_for(n, it->signals) {
    sp_semaphore_signal(&semaphore);
  }
  if (it->wait) {
    sp_semaphore_wait(&semaphore);
  }

  sync_futex_end();

  sync_futex_verify(t, it->expect);
  return SP_OK;
}

#endif
