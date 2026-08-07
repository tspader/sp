#include "harness.h"

#if !defined(SP_WASM)

typedef struct {
  const c8* name;
  bool locked;
  sync_wait_t script [SYNC_MAX_WAITS];
  sync_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "uncontended_lock_unlock_makes_no_futex_calls",
  },
  {
    .name = "contended_lock_waits_until_released",
    .locked = true,
    .script = {
      { .expected = 2, .set = true, .to = 0 },
    },
    .expect = { .waits = 1, .wakes = 1 },
  },
  {
    .name = "spurious_wake_returns_to_waiting",
    .locked = true,
    .script = {
      { .expected = 2 },
      { .expected = 2, .set = true, .to = 0 },
    },
    .expect = { .waits = 2, .wakes = 1 },
  },
};

sp_test_each(sync, mutex_shell, test_t, tests, .serial = true) {

  sync_futex_begin(t, it->script, 0);

  sp_mutex_t mutex = sp_zero;
  if (it->locked) {
    sp_mutex_lock(&mutex);
  }
  sp_mutex_lock(&mutex);
  sp_mutex_unlock(&mutex);

  sync_futex_end();

  sync_futex_verify(t, it->expect);
  return SP_OK;
}

#endif
