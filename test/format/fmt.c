#include "format.h"

UTEST(format_fmt, basic) {
  format_fmt_test_t cases[] = {
    {
      "hello, world",
      sp_zero,
      "hello, world"
    },
    {
      "{}",
      { sp_fmt_int(42) },
      "42"
    },
    {
      "{}",
      { sp_fmt_cstr("world") },
      "world"
    },
    {
      "{} + {} = {}",
      { sp_fmt_int(2), sp_fmt_int(3), sp_fmt_int(5) },
      "2 + 3 = 5"
    },
    {
      "hello, {}!",
      { sp_fmt_cstr("thomas") },
      "hello, thomas!"
    },
    {
      "{:6}",
      { sp_fmt_int(42) },
      "    42"
    },
    {
      "{:*^9}",
      { sp_fmt_int(42) },
      "***42****"
    },
    {
      "{{{}}}",
      { sp_fmt_int(7) },
      "{7}"
    },
    {
      "hello }} world",
      sp_zero,
      "hello } world"
    },
    {
      "[{:->8}]",
      { sp_fmt_cstr("hi") },
      "[------hi]"
    },
    {
      "{{{:5}}}",
      { sp_fmt_int(42) },
      "{   42}"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_fmt, dynamic) {
  format_fmt_test_t cases[] = {
    {
      "{:$}",
      { sp_fmt_int(6), sp_fmt_int(42) },
      "    42"
    },
    {
      "{:$^9}",
      { sp_fmt_int('*'), sp_fmt_int(42) },
      "***42****"
    },
    {
      "{:$^$}",
      { sp_fmt_int('-'), sp_fmt_int(8), sp_fmt_cstr("hi") },
      "---hi---"
    },
    {
      "{:.$}",
      { sp_fmt_int(3), sp_fmt_float(3.14159) },
      "3.142"
    },
    {
      "{:$.2}",
      { sp_fmt_int(8), sp_fmt_float(1.5) },
      "    1.50"
    },
    {
      "{:$<$.$}",
      { sp_fmt_int('*'), sp_fmt_int(8), sp_fmt_int(2), sp_fmt_float(1.5) },
      "1.50****"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_fmt, precision) {
  format_fmt_test_t cases[] = {
    {
      "{:.3}",
      { sp_fmt_cstr("hello") },
      "hel"
    },
    {
      "{:.10}",
      { sp_fmt_cstr("hi") },
      "hi"
    },
    {
      "{:.$}",
      { sp_fmt_int(2), sp_fmt_cstr("hello") },
      "he"
    },
    {
      "[{:>6.3}]",
      { sp_fmt_cstr("hello") },
      "[   hel]"
    },
    {
      "{:.0}",
      { sp_fmt_float(3.7) },
      "4"
    },
    {
      "{:.$}",
      { sp_fmt_int(0), sp_fmt_float(3.7) },
      "4"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_fmt, alignment) {
  format_fmt_test_t cases[] = {
    {
      "{:>16}",
      { sp_fmt_ptr((void*)(uintptr_t)0xabcdULL) },
      "          0xabcd"
    },
    {
      "{:>8}",
      { sp_fmt_int(-42) },
      "     -42"
    },
    {
      "{:0>8}",
      { sp_fmt_int(-42) },
      "00000-42"
    },
    {
      "{:*^6}",
      { sp_fmt_cstr("") },
      "******"
    },
    {
      "{:>10.2}",
      { sp_fmt_float(3.14159) },
      "      3.14"
    },
    {
      "{:*^10.2}",
      { sp_fmt_float(1.5) },
      "***1.50***"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_fmt, width_clamp) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{:99999}",
      .args = { sp_fmt_cstr("x") },
      .expect_len = SP_FMT_WIDTH_MAX
    },
    {
      .fmt = "{:$}",
      .args = { sp_fmt_uint(999999999ULL), sp_fmt_cstr("x") },
      .expect_len = SP_FMT_WIDTH_MAX
    },
    {
      .fmt = "{:$}",
      .args = { sp_fmt_int(-5), sp_fmt_cstr("hi") },
      .expect = "hi"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_fmt, errors) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "oops } here",
      .args = sp_zero,
      .err = SP_ERR_FMT_BAD_PLACEHOLDER
    },
    {
      .fmt = "a {:5.} b {}",
      .args = { sp_fmt_int(99) },
      .err = SP_ERR_FMT_BAD_PRECISION
    },
    {
      .fmt = "hi {nope",
      .args = { sp_fmt_int(1) },
      .err = SP_ERR_FMT_BAD_PLACEHOLDER
    },
    {
      .fmt = "{:$^5}",
      .args = { sp_fmt_float(1.0), sp_fmt_int(42) },
      .err = SP_ERR_FMT_WRONG_FILL_KIND
    },
    {
      .fmt = "{:$}",
      .args = { sp_fmt_cstr("oops"), sp_fmt_int(42) },
      .err = SP_ERR_FMT_WRONG_WIDTH_KIND
    },
    {
      .fmt = "{:.$}",
      .args = { sp_fmt_float(3.0), sp_fmt_float(3.14) },
      .err = SP_ERR_FMT_WRONG_PRECISION_KIND
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

UTEST(format_fmt, writer_variants) {
  c8 buffer[64] = sp_zero;
  SP_EXPECT_STR_EQ_CSTR(__sp_fmt_buf(buffer, 64, "hello, {}", sp_fmt_cstr("world")).value, "hello, world");

  c8 decorated[64] = sp_zero;
  SP_EXPECT_STR_EQ_CSTR(__sp_fmt_buf(decorated, 64, "{:>6 .red}", sp_fmt_cstr("hi")).value, "    \033[31mhi\033[0m");

  c8 multi[64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, multi, sizeof(multi));
  sp_fmt_io(&w.base, "[{}]", sp_fmt_cstr("a"));
  sp_io_write_c8(&w.base, ' ');
  sp_fmt_io(&w.base, "[{}]", sp_fmt_cstr("b"));
  SP_EXPECT_STR_EQ_CSTR(sp_io_mem_writer_as_str(&w), "[a] [b]");
}

static f64 format_f64_from_bits(u64 bits) {
  union { f64 f; u64 u; } b = { .u = bits };
  return b.f;
}

UTEST(format_fmt, float_default) {
  format_fmt_test_t cases[] = {
    { "{}", { sp_fmt_float(1.5) }, "1.500000" },
    { "{}", { sp_fmt_float(0.9999995) }, "1.000000" },
    { "{}", { sp_fmt_float(format_f64_from_bits(0x7ff8000000000000ULL)) }, "nan" },
    { "{}", { sp_fmt_float(format_f64_from_bits(0x7ff0000000000000ULL)) }, "inf" },
    { "{}", { sp_fmt_float(format_f64_from_bits(0xfff0000000000000ULL)) }, "-inf" },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}
