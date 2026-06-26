#include "format.h"

UTEST(format_builtin, color_style) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{.red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\033[31mhi\033[0m"
    },
    {
      .fmt = "{.br_cyan}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\033[96mhi\033[0m"
    },
    {
      .fmt = "{.italic}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\033[3mhi\033[0m"
    },
    {
      .fmt = "{.gray}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\033[90mhi\033[0m"
    },
    {
      .fmt = "{.br_red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\033[91mhi\033[0m"
    },
    {
      .fmt = "{.bold}",
      .args = { sp_fmt_int(42) },
      .expect = "\033[1m42\033[0m"
    },
    {
      .fmt = "{.hyperlink}",
      .args = { sp_fmt_cstr("https://x") },
      .expect = "\033]8;;https://x\033\\https://x\033[0m"
    },
    {
      .fmt = "{.quote}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\"hi\""
    },
    {
      .fmt = "{.quote .red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "\"\033[31mhi\033[0m\""
    },
    {
      .fmt = "{:*^8 .red}",
      .args = { sp_fmt_cstr("hi") },
      .expect = "***\033[31mhi\033[0m***"
    },
    {
      .fmt = "{.$}",
      .args = { sp_fmt_red(), sp_fmt_cstr("hi") },
      .expect = "\033[31mhi\033[0m"
    },
    {
      .fmt = "{.$}",
      .args = { sp_fmt_bold(), sp_fmt_int(42) },
      .expect = "\033[1m42\033[0m"
    },
    {
      .fmt = "{.$ .bold}",
      .args = { sp_fmt_green(), sp_fmt_cstr("hi") },
      .expect = "\033[32m\033[1mhi\033[0m\033[0m"
    },
    {
      .fmt = "{:*^8 .$}",
      .args = { sp_fmt_cyan(), sp_fmt_cstr("hi") },
      .expect = "***\033[36mhi\033[0m***"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_builtin, bytes) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{:B}",
      .args = { sp_fmt_uint(2560) },
      .expect = "2.5 KB"
    },
    {
      .fmt = "{:9B}",
      .args = { sp_fmt_uint(2560) },
      .expect = "   2.5 KB"
    },
    {
      .fmt = "{:B}",
      .args = { sp_fmt_cstr("hello") },
      .expect = "5 B"
    },
    {
      .fmt = "{:B}",
      .args = { sp_fmt_int(2560) },
      .expect = "2.5 KB"
    },
    {
      .fmt = "{:B}",
      .args = { sp_fmt_int(-5) },
      .expect = "0 B"
    },
    {
      .fmt = "{:B}",
      .args = { sp_fmt_float(2560.0) },
      .expect = "2.5 KB"
    },
    {
      .fmt = "{:B}",
      .args = { sp_fmt_float(-1.0) },
      .expect = "0 B"
    },
    {
      .fmt = "{:B}",
      .args = { sp_fmt_ptr(SP_NULLPTR) },
      .err = SP_ERR_FMT_BAD_ARG
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_builtin, radix) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{:x}",
      .args = { sp_fmt_uint(255) },
      .expect = "ff"
    },
    {
      .fmt = "{:X}",
      .args = { sp_fmt_uint(0xdeadbeef) },
      .expect = "DEADBEEF"
    },
    {
      .fmt = "0x{:0>8x}",
      .args = { sp_fmt_uint(0xabcd) },
      .expect = "0x0000abcd"
    },
    {
      .fmt = "{:b}",
      .args = { sp_fmt_uint(5) },
      .expect = "101"
    },
    {
      .fmt = "{:o}",
      .args = { sp_fmt_uint(8) },
      .expect = "10"
    },
    {
      .fmt = "{:x}",
      .args = { sp_fmt_int(255) },
      .expect = "ff"
    },
    {
      .fmt = "{:x}",
      .args = { sp_fmt_int(-1) },
      .expect = "ffffffffffffffff"
    },
    {
      .fmt = "{:>6x .cyan}",
      .args = { sp_fmt_uint(255) },
      .expect = "    \033[36mff\033[0m"
    },
    {
      .fmt = "{:x}",
      .args = { sp_fmt_cstr("nope") },
      .err = SP_ERR_FMT_BAD_ARG
    },
    {
      .fmt = "{:x}",
      .args = { sp_fmt_float(1.0) },
      .err = SP_ERR_FMT_BAD_ARG
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_builtin, character) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{:c}",
      .args = { sp_fmt_uint(65) },
      .expect = "A"
    },
    {
      .fmt = "{:c}{:c}{:c}",
      .args = { sp_fmt_uint('a'), sp_fmt_uint('b'), sp_fmt_uint('c') },
      .expect = "abc"
    },
    {
      .fmt = "{:c}",
      .args = { sp_fmt_int(122) },
      .expect = "z"
    },
    {
      .fmt = "[{:*^5c}]",
      .args = { sp_fmt_uint('Q') },
      .expect = "[**Q**]"
    },
    {
      .fmt = "{:c}",
      .args = { sp_fmt_cstr("x") },
      .err = SP_ERR_FMT_BAD_ARG
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_builtin, unknown_tag) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{.bogus}",
      .args = { sp_fmt_cstr("hi") },
      .err = SP_ERR_FMT_UNKNOWN_DIRECTIVE
    },
    {
      .fmt = "{.$}",
      .args = { sp_fmt_style(9999), sp_fmt_cstr("hi") },
      .err = SP_ERR_FMT_UNKNOWN_DIRECTIVE
    },
    {
      .fmt = "{.$}",
      .args = { sp_fmt_style(-1), sp_fmt_cstr("hi") },
      .err = SP_ERR_FMT_UNKNOWN_DIRECTIVE
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}
