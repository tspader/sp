#ifndef SYNC_HARNESS_H
#define SYNC_HARNESS_H

#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

#define SYNC_MAX_WAITS 4

typedef struct {
  u32 expected;
  bool set;
  u32 to;
  bool timed_out;
  u32 sleep_ms;
} sync_wait_t;

typedef struct {
  u32 waits;
  u32 wakes;
  u32 wake_alls;
  bool timed;
} sync_expect_t;

typedef struct {
  sp_test_t* t;
  const sync_wait_t* script;
  u32 escape;
  u32 waits;
  u32 wakes;
  u32 wake_alls;
  u32 timed_waits;
  u64 timeouts_ns [SYNC_MAX_WAITS];
  const sp_sys_vtable_t* saved;
  sp_sys_vtable_t vt;
} sync_futex_t;

static sync_futex_t sync_futex;

static bool sync_futex_wait(u32* addr, u32 expected, const sp_sys_timespec_t* timeout) {
  sync_futex_t* f = &sync_futex;
  if (f->waits >= SYNC_MAX_WAITS) {
    sp_test_fail(f->t, "futex_wait called more than {} times", sp_fmt_uint(SYNC_MAX_WAITS));
    *addr = f->escape;
    return false;
  }

  const sync_wait_t* step = &f->script[f->waits];
  if (timeout) {
    f->timeouts_ns[f->waits] = (u64)timeout->tv_sec * SP_TM_S_TO_NS + (u64)timeout->tv_nsec;
    f->timed_waits++;
  }
  f->waits++;

  sp_expect_eq(f->t, expected, step->expected);
  sp_expect_eq(f->t, *addr, step->expected);
  if (step->set) *addr = step->to;
  if (step->sleep_ms) sp_os_sleep_ms(step->sleep_ms);
  return !step->timed_out;
}

static void sync_futex_wake(u32* addr) {
  sync_futex.wakes++;
}

static void sync_futex_wake_all(u32* addr) {
  sync_futex.wake_alls++;
}

static void sync_futex_begin(sp_test_t* t, const sync_wait_t* script, u32 escape) {
  sync_futex = sp_zero_s(sync_futex_t);
  sync_futex.t = t;
  sync_futex.script = script;
  sync_futex.escape = escape;
  sync_futex.vt = sp_sys_vtable_platform;
  sync_futex.vt.futex_wait = sync_futex_wait;
  sync_futex.vt.futex_wake = sync_futex_wake;
  sync_futex.vt.futex_wake_all = sync_futex_wake_all;
  sync_futex.saved = sp_sys_set_vtable(&sync_futex.vt);
}

static void sync_futex_end() {
  sp_sys_set_vtable(sync_futex.saved);
}

static void sync_futex_verify(sp_test_t* t, sync_expect_t expect) {
  sp_expect_eq(t, sync_futex.waits, expect.waits);
  sp_expect_eq(t, sync_futex.wakes, expect.wakes);
  sp_expect_eq(t, sync_futex.wake_alls, expect.wake_alls);
  sp_expect_eq(t, sync_futex.timed_waits, expect.timed ? expect.waits : (u32)0);
}

#endif

#endif
