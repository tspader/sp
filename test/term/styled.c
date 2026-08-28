#include "term.h"

#define TERM_STYLED_MAX_SPANS 8

typedef struct {
  const c8* text;
} term_styled_expect_t;

typedef struct {
  sp_term_color_t color;
  const c8* text;
  sp_fmt_span_t spans [TERM_STYLED_MAX_SPANS];
  term_styled_expect_t expect;
} term_styled_test_t;

static void run_term_styled(s32* utest_result, term_styled_test_t t) {
  u64 num_spans = 0;
  sp_carr_for(t.spans, it) {
    if (!t.spans[it].len) break;
    num_spans++;
  }

  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(sp_mem_get_scratch(), &io);
  sp_term_t term = { .io = &io.base, .color = t.color };

  EXPECT_EQ(sp_term_styled(&term, sp_cstr_as_str(t.text), t.spans, num_spans), SP_OK);
  SP_EXPECT_STR_EQ_CSTR(sp_io_dyn_mem_writer_as_str(&io), t.expect.text);
}

UTEST(term_styled, ansi) {
  term_styled_test_t cases[] = {
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "abc",
      .expect = { "abc" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "ab cd",
      .spans = { { 3, 2, sp_fmt_style_red } },
      .expect = { "ab \033[31mcd\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "abc",
      .spans = { { 0, 1, sp_fmt_style_green } },
      .expect = { "\033[32ma\033[0mbc" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "ab",
      .spans = { { 1, 1, sp_fmt_style_red } },
      .expect = { "a\033[31mb\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "ab",
      .spans = { { 0, 1, sp_fmt_style_red }, { 1, 1, sp_fmt_style_green } },
      .expect = { "\033[31ma\033[0m\033[32mb\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "x",
      .spans = { { 0, 1, sp_fmt_style_red }, { 0, 1, sp_fmt_style_green } },
      .expect = { "\033[31m\033[32mx\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "abcdef",
      .spans = { { 0, 6, sp_fmt_style_red }, { 2, 2, sp_fmt_style_bold } },
      .expect = { "\033[31mab\033[1mcd\033[0m\033[31mef\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "abcd",
      .spans = { { 0, 3, sp_fmt_style_red }, { 2, 2, sp_fmt_style_green } },
      .expect = { "\033[31mab\033[32mc\033[0m\033[32md\033[0m" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "ab",
      .spans = { { 1, 1, sp_fmt_style_green }, { 0, 1, sp_fmt_style_red } },
      .expect = { "\033[31ma\033[0m\033[32mb\033[0m" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_styled(utest_result, cases[i]);
}

UTEST(term_styled, plain) {
  term_styled_test_t cases[] = {
    {
      .text = "abc",
      .expect = { "abc" }
    },
    {
      .text = "ab cd",
      .spans = { { 3, 2, sp_fmt_style_red } },
      .expect = { "ab cd" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_styled(utest_result, cases[i]);
}

UTEST(term_styled, unstyled_spans) {
  term_styled_test_t cases[] = {
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "abc",
      .spans = { { 0, 3, sp_fmt_style_quote } },
      .expect = { "abc" }
    },
    {
      .color = SP_TERM_COLOR_ANSI,
      .text = "abc",
      .spans = { { 0, 3, sp_fmt_style_none } },
      .expect = { "abc" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_styled(utest_result, cases[i]);
}
