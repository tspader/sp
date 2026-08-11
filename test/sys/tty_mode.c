#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM)

typedef struct {
  bool changes_attributes;
} expect_t;

typedef struct {
  const c8* name;
  sp_sys_tty_mode_t mode;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "raw_round_trips",
    .mode = SP_SYS_TTY_MODE_RAW,
    .expect = { .changes_attributes = true },
  },
  {
    .name = "no_echo_round_trips",
    .mode = SP_SYS_TTY_MODE_NO_ECHO,
    .expect = { .changes_attributes = true },
  },
  {
    .name = "cooked_round_trips",
    .mode = SP_SYS_TTY_MODE_COOKED,
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  if (!sp_sys_is_tty(sp_sys_stdin)) {
    return sp_test_skip(t, "stdin is not a tty");
  }

  sp_sys_tty_attr_t before = sp_zero;
  sp_must_ok(t, sp_sys_tty_get(sp_sys_stdin, &before));
  sp_must(t, before.present);

  sp_sys_tty_state_t saved = sp_zero;
  sp_must_ok(t, sp_tty_set_mode(sp_sys_stdin, sp_sys_stdout, c->mode, &saved));
  sp_expect(t, saved.in.present);
  sp_expect_eq(t, sp_sys_memcmp(saved.in.opaque, before.opaque, sizeof(before.opaque)), 0);

  sp_sys_tty_attr_t during = sp_zero;
  sp_expect_ok(t, sp_sys_tty_get(sp_sys_stdin, &during));
  if (c->expect.changes_attributes) {
    sp_expect_ne(t, sp_sys_memcmp(during.opaque, before.opaque, sizeof(before.opaque)), 0);
  }

  sp_must_ok(t, sp_tty_restore(sp_sys_stdin, sp_sys_stdout, &saved));

  sp_sys_tty_attr_t after = sp_zero;
  sp_expect_ok(t, sp_sys_tty_get(sp_sys_stdin, &after));
  sp_expect_eq(t, sp_sys_memcmp(after.opaque, before.opaque, sizeof(before.opaque)), 0);
  return SP_OK;
}

sp_test_each_fn(sys, tty_mode, test_t, tests, run, .serial = true);

#endif
