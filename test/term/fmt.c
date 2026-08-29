#include "term.h"

typedef struct {
  const c8* text;
  sp_err_t err;
} term_fmt_expect_t;

typedef struct {
  sp_tty_color_t color;
  const c8* fmt;
  sp_fmt_argv_t args [4];
  term_fmt_expect_t expect;
} term_fmt_test_t;

static void run_term_fmt(s32* utest_result, term_fmt_test_t t) {
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(sp_mem_get_scratch(), &io);
  sp_tty_t term = { .io = &io.base, .color = t.color };

  sp_err_t err = sp_tty_fmt(&term, t.fmt, t.args[0], t.args[1], t.args[2], t.args[3]);
  EXPECT_EQ(err, t.expect.err);
  if (t.expect.err != SP_OK) return;
  SP_EXPECT_STR_EQ_CSTR(sp_io_dyn_mem_writer_as_str(&io), t.expect.text);
}

UTEST(term_fmt, ansi) {
  term_fmt_test_t cases[] = {
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "hello, {}",
      .args = { sp_fmt_cstr("world") },
      .expect = { "hello, world" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\033[31mhi\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.br_cyan}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\033[96mhi\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.italic}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\033[3mhi\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.gray}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\033[90mhi\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.bold}",
      .args = { sp_fmt_int(42) },
      .expect = { "\033[1m42\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.hyperlink}",
      .args = { sp_fmt_cstr("https://x") },
      .expect = { "\033]8;;https://x\033\\https://x\033]8;;\033\\" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.quote}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\"hi\"" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.quote .red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\"\033[31mhi\033[0m\"" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{:*^8 .red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "***\033[31mhi\033[0m***" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.$}",
      .args = { sp_fmt_red(), sp_fmt_cstr("hi") },
      .expect = { "\033[31mhi\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.$}",
      .args = { sp_fmt_style(sp_fmt_style_none), sp_fmt_cstr("hi") },
      .expect = { "hi" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.$ .bold}",
      .args = { sp_fmt_green(), sp_fmt_cstr("hi") },
      .expect = { "\033[32m\033[1mhi\033[0m" }
    },
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{:*^8 .$}",
      .args = { sp_fmt_cyan(), sp_fmt_cstr("hi") },
      .expect = { "***\033[36mhi\033[0m***" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_fmt(utest_result, cases[i]);
}

UTEST(term_fmt, plain) {
  term_fmt_test_t cases[] = {
    {
      .fmt = "hello, {}",
      .args = { sp_fmt_cstr("world") },
      .expect = { "hello, world" }
    },
    {
      .fmt = "{.red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "hi" }
    },
    {
      .fmt = "{.hyperlink}",
      .args = { sp_fmt_cstr("https://x") },
      .expect = { "https://x" }
    },
    {
      .fmt = "{.quote .red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "\"hi\"" }
    },
    {
      .fmt = "{:>6 .red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { "    hi" }
    },
    {
      .fmt = "{.$}",
      .args = { sp_fmt_red(), sp_fmt_cstr("hi") },
      .expect = { "hi" }
    },
  };
  SP_CARR_FOR(cases, i) run_term_fmt(utest_result, cases[i]);
}

UTEST(term_fmt, errors) {
  term_fmt_test_t cases[] = {
    {
      .color = SP_TTY_COLOR_ANSI,
      .fmt = "{.bogus}",
      .args = { sp_fmt_cstr("hi") },
      .expect = { .err = SP_ERR_FMT_UNKNOWN_DIRECTIVE }
    },
    {
      .fmt = "{.$}",
      .args = { sp_fmt_style(9999), sp_fmt_cstr("hi") },
      .expect = { .err = SP_ERR_FMT_UNKNOWN_DIRECTIVE }
    },
  };
  SP_CARR_FOR(cases, i) run_term_fmt(utest_result, cases[i]);
}
