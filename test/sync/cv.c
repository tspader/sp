#include "harness.h"

#if !defined(SP_WASM)

typedef struct {
  const c8* name;
  u32 notifies;
  bool all;
  bool wait;
  sync_wait_t script [SYNC_MAX_WAITS];
  sync_expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "notify_one_wakes_one",
    .notifies = 1,
    .expect = { .wakes = 1 },
  },
  {
    .name = "notify_all_wakes_all",
    .notifies = 1,
    .all = true,
    .expect = { .wake_alls = 1 },
  },
  {
    .name = "wait_sleeps_until_notified",
    .wait = true,
    .script = {
      { .set = true, .to = 1 },
    },
    .expect = { .waits = 1 },
  },
  {
    .name = "wait_passes_current_sequence",
    .notifies = 1,
    .wait = true,
    .script = {
      { .expected = 1, .set = true, .to = 2 },
    },
    .expect = { .waits = 1, .wakes = 1 },
  },
};

sp_test_each(sync, cv_shell, test_t, tests, .serial = true) {

  sync_futex_begin(t, it->script, 0);

  sp_cv_t cv = sp_zero;
  sp_mutex_t mutex = sp_zero;
  sp_for(n, it->notifies) {
    if (it->all) {
      sp_cv_notify_all(&cv);
    }
    else {
      sp_cv_notify_one(&cv);
    }
  }
  if (it->wait) {
    sp_mutex_lock(&mutex);
    sp_cv_wait(&cv, &mutex);
    sp_mutex_unlock(&mutex);
  }

  sync_futex_end();

  sync_futex_verify(t, it->expect);
  return SP_OK;
}

#endif
