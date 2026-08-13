#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  u64 req_ns;
  u32 count;
} test_t;

static const test_t tests [] = {
  { .name = "waits_full_duration", .req_ns = 40 * 1000 * 1000, .count = 5 },
  { .name = "waits_sub_ms_requests", .req_ns = 500 * 1000, .count = 20 },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_for(it, c->count) {
    sp_tm_point_t start = sp_tm_now_point();
    sp_sys_timespec_t req = { .tv_sec = 0, .tv_nsec = (s64)c->req_ns };
    sp_expect_ok(t, sp_sys_nanosleep(&req, SP_NULLPTR));
    u64 ns = sp_tm_point_diff(sp_tm_now_point(), start);
    if (ns < c->req_ns) {
      sp_test_fail(t, "sleep {} returned after {}ns of {}ns", sp_fmt_uint(it), sp_fmt_uint(ns), sp_fmt_uint(c->req_ns));
      return SP_OK;
    }
  }
  return SP_OK;
}

sp_test_each_fn(sys, nanosleep, test_t, tests, run);
