#define SP_IMPLEMENTATION
#include "sp.h"
#include <stdio.h>

typedef enum {
  SLEEP_FN_OS_SLEEP_NS,
  SLEEP_FN_OS_SLEEP_MS,
  SLEEP_FN_SLEEP_NS,
  SLEEP_FN_SLEEP_MS,
} sleep_fn_t;

typedef struct {
  sleep_fn_t fn;
  u64 target_ns;
  u64 actual_ns;
} sleep_test_t;

static void run_test(sleep_test_t* test) {
  sp_tm_timer_t timer = sp_tm_start_timer();

  switch (test->fn) {
    case SLEEP_FN_OS_SLEEP_NS: {
      sp_os_sleep_ns(test->target_ns);
      break;
    }
    case SLEEP_FN_OS_SLEEP_MS: {
      sp_os_sleep_ms((f64)test->target_ns / 1000000.0);
      break;
    }
    case SLEEP_FN_SLEEP_NS: {
      sp_sleep_ns(test->target_ns);
      break;
    }
    case SLEEP_FN_SLEEP_MS: {
      sp_sleep_ms((f64)test->target_ns / 1000000.0);
      break;
    }
  }

  test->actual_ns = sp_tm_read_timer(&timer);
}

static const c8* fn_name(sleep_fn_t fn) {
  switch (fn) {
    case SLEEP_FN_OS_SLEEP_NS: { return "sp_os_sleep_ns"; }
    case SLEEP_FN_OS_SLEEP_MS: { return "sp_os_sleep_ms"; }
    case SLEEP_FN_SLEEP_NS:    { return "sp_sleep_ns"; }
    case SLEEP_FN_SLEEP_MS:    { return "sp_sleep_ms"; }
  }
  return "unknown";
}

int main(void) {
  sleep_test_t tests[] = {
    { .fn = SLEEP_FN_OS_SLEEP_NS, .target_ns =   1000000 },
    { .fn = SLEEP_FN_OS_SLEEP_NS, .target_ns =   5000000 },
    { .fn = SLEEP_FN_OS_SLEEP_NS, .target_ns =  10000000 },
    { .fn = SLEEP_FN_OS_SLEEP_NS, .target_ns =  16666667 },
    { .fn = SLEEP_FN_OS_SLEEP_NS, .target_ns = 100000000 },

    { .fn = SLEEP_FN_OS_SLEEP_MS, .target_ns =   1000000 },
    { .fn = SLEEP_FN_OS_SLEEP_MS, .target_ns =   5000000 },
    { .fn = SLEEP_FN_OS_SLEEP_MS, .target_ns =  10000000 },
    { .fn = SLEEP_FN_OS_SLEEP_MS, .target_ns =  16666667 },
    { .fn = SLEEP_FN_OS_SLEEP_MS, .target_ns = 100000000 },

    { .fn = SLEEP_FN_SLEEP_NS,    .target_ns =   1000000 },
    { .fn = SLEEP_FN_SLEEP_NS,    .target_ns =   5000000 },
    { .fn = SLEEP_FN_SLEEP_NS,    .target_ns =  10000000 },
    { .fn = SLEEP_FN_SLEEP_NS,    .target_ns =  16666667 },
    { .fn = SLEEP_FN_SLEEP_NS,    .target_ns = 100000000 },

    { .fn = SLEEP_FN_SLEEP_MS,    .target_ns =   1000000 },
    { .fn = SLEEP_FN_SLEEP_MS,    .target_ns =   5000000 },
    { .fn = SLEEP_FN_SLEEP_MS,    .target_ns =  10000000 },
    { .fn = SLEEP_FN_SLEEP_MS,    .target_ns =  16666667 },
    { .fn = SLEEP_FN_SLEEP_MS,    .target_ns = 100000000 },
  };

  sp_carr_for(tests, i) {
    run_test(&tests[i]);
  }

  printf("%-16s %12s %12s %12s %10s\n",
    "function", "target(us)", "actual(us)", "delta(us)", "delta(%)");
  printf("%-16s %12s %12s %12s %10s\n",
    "----------------", "------------", "------------", "------------", "----------");

  sp_carr_for(tests, i) {
    sleep_test_t* t = &tests[i];
    f64 target_us = (f64)t->target_ns / 1000.0;
    f64 actual_us = (f64)t->actual_ns / 1000.0;
    f64 delta_us = actual_us - target_us;
    f64 delta_pct = (delta_us / target_us) * 100.0;

    printf("%-16s %12.1f %12.1f %12.1f %10.3f\n",
      fn_name(t->fn), target_us, actual_us, delta_us, delta_pct);
  }

  return 0;
}
