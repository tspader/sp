#include "sp.h"
#include "sp/sp_test.h"

#define EVENT_SLOTS 2
#define EVENT_MAX_STEPS 8

typedef enum {
  STEP_NONE,
  STEP_SIGNAL,
  STEP_CLEAR,
  STEP_SPAWN,
  STEP_WRITE,
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
  bool pipe;
  step_t steps [EVENT_MAX_STEPS];
} test_t;

static const test_t tests [] = {
  {
    .name = "signaled_until_cleared",
    .events = 1,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
      { .kind = STEP_CLEAR },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .expect = { .err = SP_ERR_SYS_TIMED_OUT } } },
    },
  },
  {
    .name = "double_signal_ok",
    .events = 1,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_SIGNAL },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
    },
  },
  {
    .name = "double_clear_ok",
    .events = 1,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_CLEAR },
      { .kind = STEP_CLEAR },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .expect = { .err = SP_ERR_SYS_TIMED_OUT } } },
    },
  },
  {
    .name = "thread_signal_wakes_wait",
    .events = 1,
    .steps = {
      { .kind = STEP_SPAWN },
      { .kind = STEP_WAIT },
    },
  },
};

static const test_t wait_tests [] = {
  {
    .name = "timeout_expires_when_never_signaled",
    .events = 1,
    .steps = {
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 50, .expect = { .err = SP_ERR_SYS_TIMED_OUT, .min_elapsed_ms = 30 } } },
    },
  },
  {
    .name = "reports_lowest_signaled_index",
    .events = 2,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_SIGNAL, .signal = { .slot = 1 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
    },
  },
  {
    .name = "reports_only_signaled_index",
    .events = 2,
    .steps = {
      { .kind = STEP_SIGNAL, .signal = { .slot = 1 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000, .expect = { .index = 1 } } },
    },
  },
  {
    .name = "clear_moves_wait_to_next_signaled",
    .events = 2,
    .steps = {
      { .kind = STEP_SIGNAL },
      { .kind = STEP_SIGNAL, .signal = { .slot = 1 } },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
      { .kind = STEP_CLEAR },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000, .expect = { .index = 1 } } },
    },
  },
};

// sp_sys_wait takes platform-waitable objects only; a Win32 sync pipe is not
// one, so the mixed event+pipe sets are POSIX-only
#if !defined(SP_WIN32)
static const test_t mixed_tests [] = {
  {
    .name = "pipe_data_signals_its_index",
    .events = 1,
    .pipe = true,
    .steps = {
      { .kind = STEP_WRITE },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000, .expect = { .index = 1 } } },
    },
  },
  {
    .name = "lowest_index_wins_across_kinds",
    .events = 1,
    .pipe = true,
    .steps = {
      { .kind = STEP_WRITE },
      { .kind = STEP_SIGNAL },
      { .kind = STEP_WAIT, .wait = { .timeout_ms = 1000 } },
    },
  },
};
#endif

static s32 signaler(void* userdata) {
  sp_sys_event_signal(*(sp_sys_event_t*)userdata);
  return 0;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_event_t events [EVENT_SLOTS];
  sp_sys_fd_t fds [EVENT_SLOTS + 1];
  sp_sys_pipe_t p = { SP_SYS_INVALID_FD, SP_SYS_INVALID_FD };
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
  if (c->pipe) {
    sp_must_ok(t, sp_sys_pipe(&p, sp_zero_s(sp_sys_pipe_desc_t)));
    fds[n++] = p.r;
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
      case STEP_WRITE: {
        u64 written = 0;
        sp_must_ok(t, sp_sys_write(p.w, "A", 1, &written));
        break;
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
  if (p.r != SP_SYS_INVALID_FD) sp_sys_close(p.r);
  if (p.w != SP_SYS_INVALID_FD) sp_sys_close(p.w);
  return SP_OK;
}

sp_test_each_fn(sys, event, test_t, tests, run);
sp_test_each_fn(sys, wait, test_t, wait_tests, run);
#if !defined(SP_WIN32)
sp_test_each_fn(sys, wait_mixed, test_t, mixed_tests, run);
#endif
