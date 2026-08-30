#include "harness.h"

typedef struct {
  u32 waits [OPS_MAX_SLOTS];
} expect_t;

typedef struct {
  const c8* name;
  ops_fixture_t fixture;
  u32 entries;
  ops_submit_t op;
  u32 ops;
  u32 max;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "more_in_flight_than_entries",
    .entries = 4,
    .op = { .op = SP_IO_OP_TIMEOUT },
    .ops = 16,
    .max = 16,
  },
  {
    .name = "max_smaller_than_available",
    .fixture = FIXTURE_SOCKETS,
    .entries = 8,
    .op = { .op = SP_IO_OP_SEND, .handle = H_SERVER, .data = "A" },
    .ops = 4,
    .max = 2,
    .expect = { .waits = { 2, 2 } },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  ops_harness_t h;
  sp_try(harness_open(t, &h, c->entries, c->fixture));

  sp_for(it, c->ops) {
    ops_submit_t submit = c->op;
    submit.slot = it;
    sp_must_ok(t, harness_submit(t, &h, &submit));
  }

  bool seen [OPS_MAX_SLOTS] = sp_zero;
  u32 total = 0;
  sp_for(call, c->ops) {
    if (total == c->ops) break;
    sp_test_kv(t, "wait", sp_test_format(t, "{}", sp_fmt_uint(call)));

    sp_io_op_t* done [OPS_MAX_SLOTS] = sp_zero;
    u32 count = 0;
    sp_must_ok(t, h.io.vt->wait(h.io.user_data, done, c->max, sp_io_timeout_after(sp_tm_ms_to_ns(OPS_WAIT_MS)), &count));
    sp_expect_le(t, count, c->max);
    sp_must_gt(t, count, (u32)0);
    if (c->expect.waits[call]) sp_expect_eq(t, count, c->expect.waits[call]);

    sp_for(it, count) {
      u64 slot = (u64)(done[it] - h.ops);
      sp_expect_lt(t, slot, (u64)c->ops);
      sp_expect(t, !seen[slot]);
      sp_expect_ok(t, done[it]->result.err);
      seen[slot] = true;
    }
    total += count;
  }

  sp_test_kv_clear(t, "wait");
  sp_expect_eq(t, total, c->ops);
  harness_close(&h);
  return SP_OK;
}

sp_test_each_fn(io, ring, test_t, tests, run);
