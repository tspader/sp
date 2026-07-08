#include "format.h"

#define FORMAT_STYLED_MAX_SPANS 4

typedef struct {
  u32 start;
  u32 len;
  sp_fmt_style_t style;
} format_styled_span_t;

typedef struct {
  const c8* fmt;
  sp_fmt_argv_t args [4];
  const c8* expect;
  format_styled_span_t spans [FORMAT_STYLED_MAX_SPANS];
  sp_err_t err;
} format_styled_test_t;

static void run_format_styled(s32* utest_result, format_styled_test_t t) {
  sp_fmt_styled_r result = sp_fmt_styled(sp_mem_get_scratch(), t.fmt, t.args[0], t.args[1], t.args[2], t.args[3]);
  EXPECT_EQ(result.err, t.err);
  if (t.err != SP_OK) return;

  SP_EXPECT_STR_EQ_CSTR(result.text, t.expect);

  u32 num_expected = 0;
  sp_carr_for(t.spans, it) {
    if (!t.spans[it].len) break;
    num_expected++;
  }

  EXPECT_EQ(sp_da_size(result.spans), num_expected);
  sp_for(it, num_expected) {
    if (it >= sp_da_size(result.spans)) break;
    EXPECT_EQ(result.spans[it].start, t.spans[it].start);
    EXPECT_EQ(result.spans[it].len, t.spans[it].len);
    EXPECT_EQ(result.spans[it].style, t.spans[it].style);
  }
}

UTEST(format_styled, plain) {
  format_styled_test_t cases[] = {
    {
      "hello, world",
      sp_zero,
      "hello, world",
    },
    {
      "hello, {}",
      { sp_fmt_cstr("world") },
      "hello, world",
    },
    {
      "{} + {} = {}",
      { sp_fmt_int(2), sp_fmt_int(3), sp_fmt_int(5) },
      "2 + 3 = 5",
    },
  };
  SP_CARR_FOR(cases, i) run_format_styled(utest_result, cases[i]);
}

UTEST(format_styled, spans) {
  format_styled_test_t cases[] = {
    {
      "hello {.cyan}",
      { sp_fmt_cstr("world") },
      "hello world",
      { { 6, 5, sp_fmt_style_cyan } },
    },
    {
      "{.red} {.green}",
      { sp_fmt_cstr("a"), sp_fmt_cstr("bb") },
      "a bb",
      { { 0, 1, sp_fmt_style_red }, { 2, 2, sp_fmt_style_green } },
    },
    {
      "{.bold}",
      { sp_fmt_cstr("x") },
      "x",
      { { 0, 1, sp_fmt_style_bold } },
    },
    {
      "{.$}",
      { sp_fmt_style(sp_fmt_style_yellow), sp_fmt_cstr("dyn") },
      "dyn",
      { { 0, 3, sp_fmt_style_yellow } },
    },
    {
      "{.red .green}",
      { sp_fmt_cstr("x") },
      "x",
      { { 0, 1, sp_fmt_style_red }, { 0, 1, sp_fmt_style_green } },
    },
    {
      "{.cyan}",
      { sp_fmt_cstr("") },
      "",
    },
  };
  SP_CARR_FOR(cases, i) run_format_styled(utest_result, cases[i]);
}

UTEST(format_styled, quote_and_padding) {
  format_styled_test_t cases[] = {
    {
      "{.quote}",
      { sp_fmt_cstr("x") },
      "\"x\"",
    },
    {
      "{.quote .red}",
      { sp_fmt_cstr("x") },
      "\"x\"",
      { { 1, 1, sp_fmt_style_red } },
    },
    {
      "{.red .quote}",
      { sp_fmt_cstr("x") },
      "\"x\"",
      { { 0, 3, sp_fmt_style_red } },
    },
    {
      "{:>6 .cyan}",
      { sp_fmt_cstr("ab") },
      "    ab",
      { { 4, 2, sp_fmt_style_cyan } },
    },
    {
      "{:<6 .cyan}",
      { sp_fmt_cstr("ab") },
      "ab    ",
      { { 0, 2, sp_fmt_style_cyan } },
    },
  };
  SP_CARR_FOR(cases, i) run_format_styled(utest_result, cases[i]);
}

UTEST(format_styled, errors) {
  format_styled_test_t cases[] = {
    {
      .fmt = "{.nope}",
      .args = { sp_fmt_cstr("x") },
      .err = SP_ERR_FMT_UNKNOWN_DIRECTIVE,
    },
    {
      .fmt = "{",
      .err = SP_ERR_FMT_UNTERMINATED_PLACEHOLDER,
    },
  };
  SP_CARR_FOR(cases, i) run_format_styled(utest_result, cases[i]);
}
