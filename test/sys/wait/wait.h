#ifndef SYS_WAIT_H
#define SYS_WAIT_H

#include "sp.h"
#include "sp/sp_test.h"

#define EVENT_SLOTS 2
#define EVENT_MAX_STEPS 8

typedef enum {
  STEP_NONE,
  STEP_SIGNAL,
  STEP_CLEAR,
  STEP_SPAWN,
  STEP_WAIT,
} step_kind_t;

typedef struct {
  u64 index;
  sp_err_t err;
  u32 min_elapsed_ms;
} wait_expect_t;

typedef struct {
  step_kind_t kind;
  union {
    struct { u32 slot; } signal;
    struct { u32 slot; } clear;
    struct { u32 slot; } spawn;
    struct { u32 timeout_ms; wait_expect_t expect; } wait;
  };
} step_t;

typedef struct {
  const c8* name;
  u32 events;
  step_t steps [EVENT_MAX_STEPS];
} test_t;

static s32 signaler(void* userdata) {
  sp_sys_event_signal(*(sp_sys_event_t*)userdata);
  return 0;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_event_t events [EVENT_SLOTS];
  sp_sys_fd_t fds [EVENT_SLOTS];
  sp_thread_t thread = sp_zero;
  bool spawned = false;
  u64 n = 0;

  sp_for(e, c->events) {
    sp_err_t err = sp_sys_event_open(&events[e]);
    if (err == SP_ERR_SYS_UNSUPPORTED) {
      sp_for(closed, e) sp_sys_close(events[closed].fd);
      return sp_test_skip(t, "events unsupported");
    }
    sp_must_ok(t, err);
    fds[n++] = events[e].fd;
  }

  sp_carr_for(c->steps, it) {
    const step_t* step = &c->steps[it];
    if (step->kind == STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "{}", sp_fmt_uint(it)));

    switch (step->kind) {
      case STEP_NONE: break;

      case STEP_SIGNAL: {
        sp_expect_ok(t, sp_sys_event_signal(events[step->signal.slot]));
        break;
      }
      case STEP_CLEAR: {
        sp_expect_ok(t, sp_sys_event_clear(events[step->clear.slot]));
        break;
      }
      case STEP_SPAWN: {
#if defined(SP_FREESTANDING)
        sp_for(e, c->events) sp_sys_close(events[e].fd);
        return sp_test_skip(t, "threads are unsupported on freestanding");
#else
        sp_thread_init(&thread, signaler, &events[step->spawn.slot]);
        spawned = true;
        break;
#endif
      }
      case STEP_WAIT: {
        u64 signaled = EVENT_SLOTS + 1;
        sp_tm_timer_t timer = sp_tm_start_timer();
        sp_err_t err = sp_sys_wait(fds, n, step->wait.timeout_ms, &signaled);
        u64 elapsed = sp_tm_read_timer(&timer);

        sp_expect_err_eq(t, err, step->wait.expect.err);
        if (!err && !step->wait.expect.err) {
          sp_expect_eq(t, signaled, step->wait.expect.index);
        }
        sp_expect_ge(t, elapsed, (u64)step->wait.expect.min_elapsed_ms * SP_TM_MS_TO_NS);
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");
  if (spawned) sp_thread_join(&thread);
  sp_for(e, c->events) sp_sys_close(events[e].fd);
  return SP_OK;
}

#endif
