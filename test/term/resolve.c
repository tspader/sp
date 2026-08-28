#include "term.h"

typedef struct {
  sp_term_color_t color;
} term_resolve_expect_t;

typedef struct {
  sp_term_hints_t hints;
  term_resolve_expect_t expect;
} term_resolve_test_t;

static void run_term_resolve(s32* utest_result, term_resolve_test_t t) {
  EXPECT_EQ(sp_term_color_resolve(t.hints), t.expect.color);
}

UTEST(term_resolve, resolve) {
  term_resolve_test_t cases[] = {
    {
      .expect = { SP_TERM_COLOR_NONE }
    },
    {
      .hints = { .tty = true },
      .expect = { SP_TERM_COLOR_ANSI }
    },
    {
      .hints = { .no_color = true, .tty = true },
      .expect = { SP_TERM_COLOR_NONE }
    },
    {
      .hints = { .force_color = true },
      .expect = { SP_TERM_COLOR_ANSI }
    },
    {
      .hints = { .force_color = true, .tty = true },
      .expect = { SP_TERM_COLOR_ANSI }
    },
    {
      .hints = { .no_color = true, .force_color = true, .tty = true },
      .expect = { SP_TERM_COLOR_NONE }
    },
    {
      .hints = { .no_color = true, .force_color = true },
      .expect = { SP_TERM_COLOR_NONE }
    },
    {
      .hints = { .no_color = true },
      .expect = { SP_TERM_COLOR_NONE }
    },
  };
  SP_CARR_FOR(cases, i) run_term_resolve(utest_result, cases[i]);
}
