#include "harness.h"

#define CLOCK_REAL_MIN_NS 1000000000000000000ull

typedef struct {
  bool monotonic;
  bool counts_at_least_awake;
  u64 min_ns;
} expect_t;

typedef struct {
  const c8* name;
  sp_io_clock_t clock;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "awake_is_monotonic",
    .clock = SP_IO_CLOCK_AWAKE,
    .expect = { .monotonic = true },
  },
  {
    .name = "boot_is_monotonic_and_counts_at_least_awake",
    .clock = SP_IO_CLOCK_BOOT,
    .expect = { .monotonic = true, .counts_at_least_awake = true },
  },
  {
    .name = "real_is_wall_clock",
    .clock = SP_IO_CLOCK_REAL,
    .expect = { .min_ns = CLOCK_REAL_MIN_NS },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  ops_harness_t h;
  sp_try(harness_open(t, &h, OPS_ENTRIES, FIXTURE_NONE));

  sp_io_time_t awake = sp_io_now(h.io, SP_IO_CLOCK_AWAKE);
  sp_io_time_t first = sp_io_now(h.io, c->clock);
  sp_io_time_t second = sp_io_now(h.io, c->clock);

  sp_expect_eq(t, (s32)first.clock, (s32)c->clock);
  sp_expect_ge(t, second.ns, c->expect.min_ns);
  if (c->expect.monotonic) sp_expect_ge(t, second.ns, first.ns);
  if (c->expect.counts_at_least_awake) sp_expect_ge(t, first.ns, awake.ns);

  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, clock, test_t, tests, run);
