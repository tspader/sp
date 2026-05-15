#define SP_PRIVATE_HEADER // We need a few internal parser structs and functions
#include "sp.h"

#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

UTEST_EMPTY_FIXTURE(format)
UTEST_EMPTY_FIXTURE(format_specifier)

static sp_ht(sp_str_t, sp_fmt_directive_t) sp_fmt_directives = SP_NULLPTR;

void sp_fmt_directive_reset() {
  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_str_ht_free(tls->format.directives);
  sp_str_ht_init(tls->mem, tls->format.directives);
  sp_fmt_register_builtins();
}

typedef struct {
  const c8* str;
  sp_fmt_spec_t expected;
  sp_err_t err;
} format_test_t;

static sp_err_t parse_spec(const c8* str, sp_fmt_spec_t* spec) {
  sp_fmt_parser_t parser = { .str = sp_str_view(str), .i = 0 };
  return sp_fmt_parse_specifier(&parser, spec);
}

void run_case(s32* utest_result, format_test_t test) {
  sp_fmt_spec_t specifier = sp_zero;
  sp_err_t err = parse_spec(test.str, &specifier);
  EXPECT_EQ(err, test.err);
  if (err != SP_OK) return;
  EXPECT_EQ(specifier.align, test.expected.align);
  EXPECT_EQ(specifier.fill, test.expected.fill);
  EXPECT_EQ(specifier.precision.some, test.expected.precision.some);
  EXPECT_EQ(specifier.precision.value, test.expected.precision.value);
  EXPECT_EQ(specifier.width, test.expected.width);
  EXPECT_EQ(specifier.directive.num, test.expected.directive.num);
  EXPECT_TRUE(specifier.dynamic.fill == test.expected.dynamic.fill);
  EXPECT_TRUE(specifier.dynamic.width == test.expected.dynamic.width);
  EXPECT_TRUE(specifier.dynamic.precision == test.expected.dynamic.precision);
}

UTEST_F(format_specifier, smoke) {
  run_case(utest_result, (format_test_t) {
    .str = "{:*^9}",
    .expected = {
      .width = 9,
      .align = SP_FMT_ALIGN_CENTER,
      .fill =  '*',
    },
  });
}

UTEST_F(format_specifier, empty) {
  run_case(utest_result, (format_test_t) {
    .str = "{}",
    .expected = { .align = SP_FMT_ALIGN_NONE },
  });
}

UTEST_F(format_specifier, empty_with_colon) {
  run_case(utest_result, (format_test_t) {
    .str = "{:}",
    .expected = { .align = SP_FMT_ALIGN_NONE },
  });
}

UTEST_F(format_specifier, width_only) {
  run_case(utest_result, (format_test_t) {
    .str = "{:42}",
    .expected = { .width = 42 },
  });
}

UTEST_F(format_specifier, precision_only) {
  run_case(utest_result, (format_test_t) {
    .str = "{:.5}",
    .expected = { .precision = sp_opt_some(5) },
  });
}

UTEST_F(format_specifier, width_and_precision) {
  run_case(utest_result, (format_test_t) {
    .str = "{:10.3}",
    .expected = { .width = 10, .precision = sp_opt_some(3) },
  });
}

UTEST_F(format_specifier, bare_align_left) {
  run_case(utest_result, (format_test_t) {
    .str = "{:<7}",
    .expected = { .width = 7, .align = SP_FMT_ALIGN_LEFT },
  });
}

UTEST_F(format_specifier, bare_align_right) {
  run_case(utest_result, (format_test_t) {
    .str = "{:>4}",
    .expected = { .width = 4, .align = SP_FMT_ALIGN_RIGHT },
  });
}

UTEST_F(format_specifier, fill_with_left_align) {
  run_case(utest_result, (format_test_t) {
    .str = "{:-<8}",
    .expected = { .width = 8, .align = SP_FMT_ALIGN_LEFT, .fill = '-' },
  });
}

UTEST_F(format_specifier, everything) {
  run_case(utest_result, (format_test_t) {
    .str = "{:*^12.4}",
    .expected = {
      .width = 12,
      .align = SP_FMT_ALIGN_CENTER,
      .fill = '*',
      .precision = sp_opt_some(4),
    },
  });
}

UTEST_F(format_specifier, zero_leading_width) {
  run_case(utest_result, (format_test_t) {
    .str = "{:05}",
    .expected = { .width = 5 },
  });
}

UTEST_F(format_specifier, dynamic_width) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$}",
    .expected = { .dynamic = { .width = 1 } },
  });
}

UTEST_F(format_specifier, dynamic_precision) {
  run_case(utest_result, (format_test_t) {
    .str = "{:.$}",
    .expected = { .dynamic = { .precision = 1 } },
  });
}

UTEST_F(format_specifier, dynamic_fill_center) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$^9}",
    .expected = {
      .width = 9,
      .align = SP_FMT_ALIGN_CENTER,
      .dynamic = { .fill = 1 },
    },
  });
}

UTEST_F(format_specifier, dynamic_fill_and_width) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$^$}",
    .expected = {
      .align = SP_FMT_ALIGN_CENTER,
      .dynamic = { .fill = 1, .width = 1 },
    },
  });
}

UTEST_F(format_specifier, dynamic_everything) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$<$.$}",
    .expected = {
      .align = SP_FMT_ALIGN_LEFT,
      .dynamic = { .fill = 1, .width = 1, .precision = 1 },
    },
  });
}

UTEST_F(format_specifier, dynamic_width_with_literal_precision) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$.4}",
    .expected = {
      .precision = sp_opt_some(4),
      .dynamic = { .width = 1 },
    },
  });
}

UTEST_F(format_specifier, err_dynamic_precision_without_dot) {
  run_case(utest_result, (format_test_t) {
    .str = "{:5$}",
    .err = SP_ERR_FMT_BAD_PLACEHOLDER
  });
}

UTEST_F(format, err_missing_open_brace) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec(":5}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, err_missing_close_brace) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:5", &spec), SP_ERR_FMT_UNTERMINATED_PLACEHOLDER);
}

UTEST_F(format, err_dot_without_digits) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:5.}", &spec), SP_ERR_FMT_BAD_PRECISION);
}

UTEST_F(format, single) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.red}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "red"));
}

UTEST_F(format, after_width) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:10 .red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 10);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "red"));
}

UTEST_F(format, full_spec) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:*^9.2 .red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 9);
  EXPECT_EQ(spec.precision.value, 2);
  EXPECT_EQ(spec.precision.some, SP_OPT_SOME);
  EXPECT_EQ(spec.align, SP_FMT_ALIGN_CENTER);
  EXPECT_EQ(spec.fill, '*');
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "red"));
}

UTEST_F(format, multiple) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.red .bold .italic}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 3);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "red"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[1], "bold"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[2], "italic"));
}

UTEST_F(format, max_count) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.a .b .c .d .e .f .g .h}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, SP_FMT_MAX_DIRECTIVES);
}

UTEST_F(format, err_too_many) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.a .b .c .d .e .f .g .h .i}", &spec), SP_ERR_FMT_TOO_MANY_DIRECTIVES);
}

UTEST_F(format, err_empty_name) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:10 .}", &spec), SP_ERR_FMT_BAD_DIRECTIVE);
}

UTEST_F(format, err_bad_char) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:10 .red!}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, err_no_space_between_directives) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.red.bold}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, err_no_dot) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:10 red}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, digit_in_tail) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.base64}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "base64"));
}

UTEST_F(format, hyphen_in_name) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.utf-8}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "utf-8"));
}

UTEST_F(format, underscore_in_name) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.http_url}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "http_url"));
}

UTEST_F(format, err_leading_digit) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{: .4red}", &spec), SP_ERR_FMT_BAD_DIRECTIVE);
}

UTEST_F(format, err_leading_hyphen) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.-red}", &spec), SP_ERR_FMT_BAD_DIRECTIVE);
}

UTEST_F(format, no_width_just_directive) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{: .red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 0);
  EXPECT_EQ(spec.directive.num, 1);
}

static sp_str_t render_value_to_str(sp_fmt_argv_t argv) {
  sp_fmt_arg_t arg = sp_fmt_arg_from_argv(argv);
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(sp_mem_get_scratch(), &io);
  sp_fmt_render_default(&io.base, &arg, SP_NULLPTR);
  return sp_io_dyn_mem_writer_as_str(&io);
}

UTEST_F(format, u64_zero) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(0));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0"));
}

UTEST_F(format, u64_large) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(1234567));
  EXPECT_TRUE(sp_str_equal_cstr(got, "1234567"));
}

UTEST_F(format, s64_negative) {
  sp_str_t got = render_value_to_str(sp_fmt_int(-42));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-42"));
}

UTEST_F(format, s64_positive) {
  sp_str_t got = render_value_to_str(sp_fmt_int(42));
  EXPECT_TRUE(sp_str_equal_cstr(got, "42"));
}

UTEST_F(format, str) {
  sp_str_t got = render_value_to_str(sp_fmt_cstr("hello"));
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello"));
}

UTEST_F(format, empty_str) {
  sp_str_t got = render_value_to_str(sp_fmt_cstr(""));
  EXPECT_TRUE(sp_str_equal_cstr(got, ""));
}

UTEST_F(format, u64_ten) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(10));
  EXPECT_TRUE(sp_str_equal_cstr(got, "10"));
}

UTEST_F(format, u64_ninety_nine) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(99));
  EXPECT_TRUE(sp_str_equal_cstr(got, "99"));
}

UTEST_F(format, u64_one_hundred) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(100));
  EXPECT_TRUE(sp_str_equal_cstr(got, "100"));
}

UTEST_F(format, u64_max) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(0xffffffffffffffffULL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "18446744073709551615"));
}

UTEST_F(format, s64_min) {
  sp_str_t got = render_value_to_str(sp_fmt_int((s64)0x8000000000000000LL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-9223372036854775808"));
}

UTEST_F(format, s64_max) {
  sp_str_t got = render_value_to_str(sp_fmt_int((s64)0x7fffffffffffffffLL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "9223372036854775807"));
}

UTEST_F(format, f64_zero) {
  sp_str_t got = render_value_to_str(sp_fmt_float(0.0));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0.000000"));
}

UTEST_F(format, f64_one) {
  sp_str_t got = render_value_to_str(sp_fmt_float(1.0));
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.000000"));
}

UTEST_F(format, f64_half) {
  sp_str_t got = render_value_to_str(sp_fmt_float(0.5));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0.500000"));
}

UTEST_F(format, f64_negative) {
  sp_str_t got = render_value_to_str(sp_fmt_float(-3.25));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-3.250000"));
}

UTEST_F(format, f64_carry) {
  sp_str_t got = render_value_to_str(sp_fmt_float(0.9999995));
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.000000"));
}

UTEST_F(format, f64_nan) {
  union { f64 f; u64 u; } nan_bits = { .u = 0x7ff8000000000000ULL };
  sp_str_t got = render_value_to_str(sp_fmt_float(nan_bits.f));
  EXPECT_TRUE(sp_str_equal_cstr(got, "nan"));
}

UTEST_F(format, f64_pos_inf) {
  union { f64 f; u64 u; } inf_bits = { .u = 0x7ff0000000000000ULL };
  sp_str_t got = render_value_to_str(sp_fmt_float(inf_bits.f));
  EXPECT_TRUE(sp_str_equal_cstr(got, "inf"));
}

UTEST_F(format, f64_neg_inf) {
  union { f64 f; u64 u; } inf_bits = { .u = 0xfff0000000000000ULL };
  sp_str_t got = render_value_to_str(sp_fmt_float(inf_bits.f));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-inf"));
}

UTEST_F(format, f64_custom_precision_via_spec) {
  sp_fmt_arg_t arg = sp_fmt_arg_from_argv(sp_fmt_float(3.14159));
  sp_opt_set(arg.spec.precision, 2);
  sp_io_dyn_mem_writer_t io = sp_zero;
  sp_io_dyn_mem_writer_init(sp_mem_get_scratch(), &io);
  sp_fmt_render_default(&io.base, &arg, SP_NULLPTR);
  sp_str_t got = sp_io_dyn_mem_writer_as_str(&io);
  EXPECT_TRUE(sp_str_equal_cstr(got, "3.14"));
}

UTEST_F(format, ptr_null) {
  sp_str_t got = render_value_to_str(sp_fmt_ptr(SP_NULLPTR));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0x0"));
}

UTEST_F(format, ptr_nonzero) {
  sp_str_t got = render_value_to_str(sp_fmt_ptr((void*)(uintptr_t)0xdeadbeefULL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xdeadbeef"));
}

static void _test_before_lt(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "<", SP_NULLPTR);
}

static void _test_after_gt(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, ">", SP_NULLPTR);
}

static void _test_render_x(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "X", SP_NULLPTR);
}

static sp_io_dyn_mem_writer_t _test_log;
static u32 _test_render_y_calls;

static void _test_before_a(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "[a", SP_NULLPTR);
  sp_io_write_cstr(&_test_log.base, "ba", SP_NULLPTR);
}

static void _test_after_a(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "a]", SP_NULLPTR);
  sp_io_write_cstr(&_test_log.base, "aa", SP_NULLPTR);
}

static void _test_before_b(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "[b", SP_NULLPTR);
  sp_io_write_cstr(&_test_log.base, "bb", SP_NULLPTR);
}

static void _test_after_b(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "b]", SP_NULLPTR);
  sp_io_write_cstr(&_test_log.base, "ab", SP_NULLPTR);
}

static void _test_render_y(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  _test_render_y_calls++;
  sp_io_write_cstr(io, "Y", SP_NULLPTR);
}


UTEST_F(format, register_and_lookup) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);

  sp_fmt_directive_t* got = sp_fmt_directive_lookup(sp_str_lit("wrap"));
  EXPECT_TRUE(got != SP_NULLPTR);
  EXPECT_EQ(got->kind, sp_fmt_directive_decorator);
  EXPECT_TRUE(got->decorator.before == _test_before_lt);
  EXPECT_TRUE(got->decorator.after == _test_after_gt);

  sp_str_t rendered = sp_fmt(sp_mem_get_scratch(), "{.wrap}", sp_fmt_cstr("ok")).value;
  EXPECT_TRUE(sp_str_equal_cstr(rendered, "<ok>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, lookup_unknown_returns_null) {
  sp_fmt_directive_reset();
  sp_fmt_directive_t* got = sp_fmt_directive_lookup(sp_str_lit("nope"));
  EXPECT_EQ(got, SP_NULLPTR);
}

UTEST_F(format, reset_clears) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("foo", _test_before_lt, SP_NULLPTR);
  EXPECT_TRUE(sp_fmt_directive_lookup(sp_str_lit("foo")) != SP_NULLPTR);
  sp_fmt_directive_reset();
  EXPECT_EQ(sp_fmt_directive_lookup(sp_str_lit("foo")), SP_NULLPTR);
}

UTEST_F(format, wraps_content) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.wrap}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<hi>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, render_replaces_value) {
  sp_fmt_directive_reset();
  sp_fmt_register_renderer("x", _test_render_x, sp_fmt_id_none);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.x}", sp_fmt_int(999)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "X"));
  sp_fmt_directive_reset();
}

UTEST_F(format, err_unknown_directive) {
  sp_fmt_directive_reset();
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.missing}", sp_fmt_int(42)).err, SP_ERR_FMT_UNKNOWN_DIRECTIVE);
}

UTEST_F(format, ordering_bracket_nested) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("a", _test_before_a, _test_after_a);
  sp_fmt_register_decorator("b", _test_before_b, _test_after_b);

  _test_log = (sp_io_dyn_mem_writer_t)sp_zero;
  sp_io_dyn_mem_writer_init(sp_mem_get_scratch(), &_test_log);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.a .b}", sp_fmt_cstr("x")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[a[bxb]a]"));

  sp_str_t log_str = sp_io_dyn_mem_writer_as_str(&_test_log);
  EXPECT_TRUE(sp_str_equal_cstr(log_str, "babbabaa"));
  sp_fmt_directive_reset();
}

UTEST_F(format, err_multiple_renders) {
  sp_fmt_directive_reset();
  _test_render_y_calls = 0;
  sp_fmt_register_renderer("x", _test_render_x, sp_fmt_id_none);
  sp_fmt_register_renderer("y", _test_render_y, sp_fmt_id_none);
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.x .y}", sp_fmt_int(0)).err, SP_ERR_FMT_TOO_MANY_RENDERERS);
  EXPECT_EQ(_test_render_y_calls, 0);
  sp_fmt_directive_reset();
}

UTEST_F(format, padding_outside_wrappers) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:6 .wrap}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    <42>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, padding_with_center_and_wrapper) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^8 .wrap}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***<hi>***"));
  sp_fmt_directive_reset();
}

UTEST_F(format, literal_only) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "hello, world").value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello, world"));
}

UTEST_F(format, empty_placeholder_int) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "42"));
}

UTEST_F(format, empty_placeholder_str) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{}", sp_fmt_cstr("world")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "world"));
}

UTEST_F(format, multi_arg) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{} + {} = {}",
    sp_fmt_int(2), sp_fmt_int(3), sp_fmt_int(5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "2 + 3 = 5"));
}

UTEST_F(format, literals_around_placeholder) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "hello, {}!", sp_fmt_cstr("thomas")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello, thomas!"));
}

UTEST_F(format, width_right_align) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:6}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    42"));
}

UTEST_F(format, fill_center) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^9}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***42****"));
}

UTEST_F(format, brace_escapes) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{{{}}}", sp_fmt_int(7)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "{7}"));
}

UTEST_F(format, close_brace_escape) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "hello }} world").value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello } world"));
}

UTEST_F(format, err_lone_close_brace) {
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "oops } here").err, SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, str_with_padding) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "[{:->8}]", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[------hi]"));
}

UTEST_F(format, custom_fn_fallback) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  u32 value = 0;
  sp_fmt_argv_t arg = sp_fmt_custom(u32, _test_render_x, value);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.wrap}", arg).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<X>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, default_render_with_wrappers_on_int) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.wrap}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<42>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, content_wider_than_width_with_wrapper) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:3 .wrap}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<hello>"));
  sp_fmt_directive_reset();
}

static void _test_render_prefixed(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "rendered", SP_NULLPTR);
}

UTEST_F(format, err_alpha_after_digit) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:5red}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, escaped_braces_around_placeholder) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{{{:5}}}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "{   42}"));
}

UTEST_F(format, dynamic_width) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$}", sp_fmt_int(6), sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    42"));
}

UTEST_F(format, dynamic_fill_center) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$^9}", sp_fmt_int('*'), sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***42****"));
}

UTEST_F(format, dynamic_fill_and_width) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$^$}",
    sp_fmt_int('-'),
    sp_fmt_int(8),
    sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "---hi---"));
}

UTEST_F(format, dynamic_precision) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:.$}", sp_fmt_int(3), sp_fmt_float(3.14159)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "3.142"));
}

UTEST_F(format, dynamic_width_with_literal_precision) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$.2}", sp_fmt_int(8), sp_fmt_float(1.5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    1.50"));
}

UTEST_F(format, err_parse_stops_formatting) {
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "a {:5.} b {}", sp_fmt_int(99)).err, SP_ERR_FMT_BAD_PRECISION);
}

UTEST_F(format, err_unterminated_placeholder) {
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "hi {nope", sp_fmt_int(1)).err, SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, err_dynamic_fill_wrong_kind) {
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{:$^5}", sp_fmt_float(1.0), sp_fmt_int(42)).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
}

UTEST_F(format, err_dynamic_width_wrong_kind) {
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{:$}", sp_fmt_cstr("oops"), sp_fmt_int(42)).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
}

UTEST_F(format, err_dynamic_precision_wrong_kind) {
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{:.$}", sp_fmt_float(3.0), sp_fmt_float(3.14)).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
}

UTEST_F(format, err_stops_subsequent_placeholders) {
  sp_fmt_directive_reset();
  sp_str_t str = sp_zero;
  sp_err_t err = sp_fmt(sp_mem_get_scratch(), "{} {.nope} {}", sp_fmt_int(1), sp_fmt_int(2), sp_fmt_int(3)).err;
  EXPECT_EQ(err, SP_ERR_FMT_UNKNOWN_DIRECTIVE);
}

UTEST_F(format, str_precision_truncates) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:.3}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hel"));
}

UTEST_F(format, str_precision_longer_than_string) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:.10}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hi"));
}

UTEST_F(format, str_dynamic_precision_truncates) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:.$}", sp_fmt_int(2), sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "he"));
}

UTEST_F(format, str_precision_with_width) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "[{:>6.3}]", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[   hel]"));
}

UTEST_F(format, f64_precision_zero_means_zero) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:.0}", sp_fmt_float(3.7)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "4"));
}

UTEST_F(format, f64_dynamic_precision_zero) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:.$}", sp_fmt_int(0), sp_fmt_float(3.7)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "4"));
}

UTEST_F(format, f64_no_precision_defaults_to_six) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{}", sp_fmt_float(1.5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.500000"));
}

UTEST_F(format, width_clamped_literal) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:99999}", sp_fmt_cstr("x")).value;
  EXPECT_EQ(got.len, SP_FMT_WIDTH_MAX);
}

UTEST_F(format, width_clamped_dynamic_huge) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$}", sp_fmt_uint(999999999ULL), sp_fmt_cstr("x")).value;
  EXPECT_EQ(got.len, SP_FMT_WIDTH_MAX);
}

UTEST_F(format, width_clamped_dynamic_negative) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$}", sp_fmt_int(-5), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hi"));
}

static void _test_render_u64_only(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)param;
  sp_fmt_write_u64(io, arg->value.u);
}

UTEST_F(format, kinds_single_accepts_match) {
  sp_fmt_directive_reset();
  sp_fmt_register_renderer("only_u64", _test_render_u64_only, sp_fmt_id_u64);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.only_u64}", sp_fmt_uint(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "42"));
  sp_fmt_directive_reset();
}

UTEST_F(format, kinds_single_rejects_mismatch) {
  sp_fmt_directive_reset();
  sp_fmt_register_renderer("only_u64", _test_render_u64_only, sp_fmt_id_u64);
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.only_u64}", sp_fmt_float(1.5)).err, SP_ERR_FMT_WRONG_PARAM_KIND);
  sp_fmt_directive_reset();
}

UTEST_F(format, kinds_multiple_accepts_either) {
  sp_fmt_directive_reset();
  sp_fmt_directive_register("num", (sp_fmt_directive_t){
    .kind = sp_fmt_directive_decorator,
    .args = sp_cast(sp_fmt_arg_kind_t, sp_fmt_id_u64 | sp_fmt_id_s64),
  });
  sp_str_t a = sp_fmt(sp_mem_get_scratch(), "{.num}", sp_fmt_uint(7)).value;
  sp_str_t b = sp_fmt(sp_mem_get_scratch(), "{.num}", sp_fmt_int(-3)).value;
  EXPECT_TRUE(sp_str_equal_cstr(a, "7"));
  EXPECT_TRUE(sp_str_equal_cstr(b, "-3"));
  sp_fmt_directive_reset();
}

UTEST_F(format, kinds_multiple_rejects_outsider) {
  sp_fmt_directive_reset();
  sp_fmt_directive_register("num", (sp_fmt_directive_t){
    .kind = sp_fmt_directive_decorator,
    .args = sp_cast(sp_fmt_arg_kind_t, sp_fmt_id_u64 | sp_fmt_id_s64),
  });
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.num}", sp_fmt_cstr("nope")).err, SP_ERR_FMT_WRONG_PARAM_KIND);
  sp_fmt_directive_reset();
}

UTEST_F(format, kinds_unset_accepts_all) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("any", _test_before_lt, _test_after_gt);
  sp_str_t a = sp_fmt(sp_mem_get_scratch(), "{.any}", sp_fmt_int(1)).value;
  sp_str_t b = sp_fmt(sp_mem_get_scratch(), "{.any}", sp_fmt_cstr("hi")).value;
  sp_str_t c = sp_fmt(sp_mem_get_scratch(), "{.any}", sp_fmt_float(2.0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(a, "<1>"));
  EXPECT_TRUE(sp_str_equal_cstr(b, "<hi>"));
  EXPECT_TRUE(sp_str_equal_cstr(c, "<2.000000>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, literal) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg red}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "fg"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[0], "red"));
  EXPECT_EQ(spec.dynamic.directive, 0);
}

UTEST_F(format, dynamic) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg $}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "fg"));
  EXPECT_EQ(spec.directive.args[0].len, 0);
  EXPECT_EQ(spec.dynamic.directive, 1);
}

UTEST_F(format, literal_then_directive) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg red .bold}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 2);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[0], "fg"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[0], "red"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[1], "bold"));
  EXPECT_EQ(spec.directive.args[1].len, 0);
}

UTEST_F(format, dynamic_then_directive) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg $ .bold}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 2);
  EXPECT_EQ(spec.dynamic.directive, 1);
}

UTEST_F(format, two_literals) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg red .bg blue}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 2);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[0], "red"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[1], "blue"));
  EXPECT_EQ(spec.dynamic.directive, 0);
}

UTEST_F(format, two_dynamics) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg $ .bg $}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 2);
  EXPECT_EQ(spec.dynamic.directive, 0b11);
}

UTEST_F(format, mixed) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg red .bg $ .bold}", &spec), SP_OK);
  EXPECT_EQ(spec.directive.num, 3);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[0], "red"));
  EXPECT_EQ(spec.directive.args[1].len, 0);
  EXPECT_EQ(spec.dynamic.directive, 0b010);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[2], "bold"));
}

UTEST_F(format, with_full_spec) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{:*^9.2 .fg red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 9);
  EXPECT_EQ(spec.directive.num, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[0], "red"));
}

UTEST_F(format, arg_with_digits_and_symbols) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.hex 0xff}", &spec), SP_OK);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive.args[0], "0xff"));
}

UTEST_F(format, err_dollar_inside_literal) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg r$d}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, err_dollar_followed_by_literal) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg $red}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST_F(format, err_space_in_arg) {
  sp_fmt_spec_t spec = sp_zero;
  EXPECT_EQ(parse_spec("{.fg red blue}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

static sp_str_t _last_fg_param;
static bool _last_fg_had_param;
static void _test_fg_before(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg;
  _last_fg_had_param = (param != SP_NULLPTR);
  if (param && param->id == sp_fmt_id_str) {
    _last_fg_param = param->value.s;
    sp_io_write_cstr(io, "<fg=", SP_NULLPTR);
    sp_io_write_str(io, param->value.s, SP_NULLPTR);
    sp_io_write_cstr(io, ">", SP_NULLPTR);
  }
  else {
    sp_io_write_cstr(io, "<fg=?>", SP_NULLPTR);
  }
}

static void _test_fg_after(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "</fg>", SP_NULLPTR);
}

UTEST_F(format, literal_passed_as_str) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg red}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=red>hello</fg>"));
  EXPECT_TRUE(_last_fg_had_param);
  EXPECT_TRUE(sp_str_equal_cstr(_last_fg_param, "red"));
  sp_fmt_directive_reset();
}

UTEST_F(format, dynamic_passed_as_str) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg $}", sp_fmt_cstr("blue"), sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=blue>hello</fg>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, dynamic_accepts_u64_with_mask) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str | sp_fmt_id_u64);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg $}", sp_fmt_uint(31), sp_fmt_cstr("x")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=?>x</fg>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, err_missing_arg) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.fg}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_MISSING);
  sp_fmt_directive_reset();
}

UTEST_F(format, err_unexpected_arg) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("bold", _test_before_lt, _test_after_gt);
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.bold red}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_UNEXPECTED);
  sp_fmt_directive_reset();
}

UTEST_F(format, err_wrong_literal_kind) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("numpad", _test_before_lt, _test_after_gt, sp_fmt_id_u64);
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.numpad abc}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
  sp_fmt_directive_reset();
}

UTEST_F(format, err_wrong_dynamic_kind) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t str = sp_zero;
  EXPECT_EQ(sp_fmt(sp_mem_get_scratch(), "{.fg $}", sp_fmt_uint(5), sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
  sp_fmt_directive_reset();
}

UTEST_F(format, chain_of_literal_and_dynamic) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_fmt_register_decorator("bold", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg $ .bold}", sp_fmt_cstr("cyan"), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=cyan><hi></fg>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, literal_color) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[31mhi\033[0m"));
}

UTEST_F(format, literal_bright_cyan) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg brightcyan}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[96mhi\033[0m"));
}

UTEST_F(format, dynamic_color) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.fg $}", sp_fmt_cstr("green"), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[32mhi\033[0m"));
}

UTEST_F(format, composes_with_padding) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^6 .fg red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "**\033[31mhi\033[0m**"));
}

UTEST_F(format, dynamic_param_interleaved_with_width) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:$ .fg $}", sp_fmt_int(4), sp_fmt_cstr("red"), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "  <fg=red>hi</fg>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, unsigned_integers) {
  // sp_parse_u8
  ASSERT_EQ(sp_parse_u8(sp_str_lit("0")), 0);
  ASSERT_EQ(sp_parse_u8(sp_str_lit("255")), 255);
  ASSERT_EQ(sp_parse_u8(sp_str_lit("128")), 128);
  ASSERT_EQ(sp_parse_u8(sp_str_lit("42")), 42);
  // Would assert: "256", "-1", "abc", ""

  // sp_parse_u16
  ASSERT_EQ(sp_parse_u16(sp_str_lit("0")), 0);
  ASSERT_EQ(sp_parse_u16(sp_str_lit("65535")), 65535);
  ASSERT_EQ(sp_parse_u16(sp_str_lit("32768")), 32768);
  ASSERT_EQ(sp_parse_u16(sp_str_lit("1234")), 1234);
  // Would assert: "65536", "-1", "text"

  // sp_parse_u32
  ASSERT_EQ(sp_parse_u32(sp_str_lit("0")), 0);
  ASSERT_EQ(sp_parse_u32(sp_str_lit("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u32(sp_str_lit("2147483648")), 2147483648U);
  ASSERT_EQ(sp_parse_u32(sp_str_lit("123456789")), 123456789U);
  // Would assert: "4294967296", "-1", "not_a_number"

  // sp_parse_u64
  ASSERT_EQ(sp_parse_u64(sp_str_lit("0")), 0ULL);
  ASSERT_EQ(sp_parse_u64(sp_str_lit("18446744073709551615")), 18446744073709551615ULL);
  ASSERT_EQ(sp_parse_u64(sp_str_lit("9223372036854775808")), 9223372036854775808ULL);
  ASSERT_EQ(sp_parse_u64(sp_str_lit("1234567890123")), 1234567890123ULL);
  // Would assert: "18446744073709551616", "-1", "invalid"
}

UTEST_F(format, signed_integers) {
  // sp_parse_s8
  ASSERT_EQ(sp_parse_s8(sp_str_lit("0")), 0);
  ASSERT_EQ(sp_parse_s8(sp_str_lit("127")), 127);
  ASSERT_EQ(sp_parse_s8(sp_str_lit("-128")), -128);
  ASSERT_EQ(sp_parse_s8(sp_str_lit("-42")), -42);
  ASSERT_EQ(sp_parse_s8(sp_str_lit("42")), 42);
  // Would assert: "128", "-129", "text"

  // sp_parse_s16
  ASSERT_EQ(sp_parse_s16(sp_str_lit("0")), 0);
  ASSERT_EQ(sp_parse_s16(sp_str_lit("32767")), 32767);
  ASSERT_EQ(sp_parse_s16(sp_str_lit("-32768")), -32768);
  ASSERT_EQ(sp_parse_s16(sp_str_lit("-1234")), -1234);
  ASSERT_EQ(sp_parse_s16(sp_str_lit("1234")), 1234);
  // Would assert: "32768", "-32769", "invalid"

  // sp_parse_s32
  ASSERT_EQ(sp_parse_s32(sp_str_lit("0")), 0);
  ASSERT_EQ(sp_parse_s32(sp_str_lit("2147483647")), 2147483647);
  ASSERT_EQ(sp_parse_s32(sp_str_lit("-2147483648")), INT32_MIN);
  ASSERT_EQ(sp_parse_s32(sp_str_lit("-123456789")), -123456789);
  ASSERT_EQ(sp_parse_s32(sp_str_lit("123456789")), 123456789);
  // Would assert: "2147483648", "-2147483649", "not_number"

  // sp_parse_s64
  ASSERT_EQ(sp_parse_s64(sp_str_lit("0")), 0LL);
  ASSERT_EQ(sp_parse_s64(sp_str_lit("9223372036854775807")), 9223372036854775807LL);
  ASSERT_EQ(sp_parse_s64(sp_str_lit("-9223372036854775808")), INT64_MIN);
  ASSERT_EQ(sp_parse_s64(sp_str_lit("-1234567890123")), -1234567890123LL);
  ASSERT_EQ(sp_parse_s64(sp_str_lit("1234567890123")), 1234567890123LL);
  // Would assert: "9223372036854775808", "-9223372036854775809", "abc"
}

UTEST_F(format, floating_point) {
  // sp_parse_f32
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("0.0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("3.14159")), 3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("-3.14159")), -3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("1.23e2")), 123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("1.23e-2")), 0.0123f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("-1.23e2")), -123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("42")), 42.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("-42")), -42.0f, 1e-5f);
  // Would assert: "nan", "inf", "text", ""

  // sp_parse_f64 - NOT IMPLEMENTED (SP_UNIMPLEMENTED)
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("0.0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("3.141592653589793")), 3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("-3.141592653589793")), -3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("1.23e10")), 1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("1.23e-10")), 1.23e-10, 1e-20);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("-1.23e10")), -1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("42.0")), 42.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(sp_str_lit("-42.0")), -42.0, 1e-10);
  // Would assert: "nan", "inf", "invalid", ""
}

UTEST_F(format, hex) {
  ASSERT_EQ(sp_parse_hex(sp_str_lit("0")), 0ULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("F")), 0xFULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("f")), 0xfULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("FF")), 0xFFULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("ff")), 0xffULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("deadbeef")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("123ABC")), 0x123ABCULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("FFFFFFFFFFFFFFFF")), 0xFFFFFFFFFFFFFFFFULL);
  // Would assert: "G", "xyz", "-F", "", "0x" prefix, "0123" octal notation
}

UTEST_F(format, hash) {
  ASSERT_EQ(sp_parse_hash(sp_str_lit("0")), 0U);
  ASSERT_EQ(sp_parse_hash(sp_str_lit("FFFFFFFF")), 0xFFFFFFFFU);
  ASSERT_EQ(sp_parse_hash(sp_str_lit("12345678")), 0x12345678U);
  ASSERT_EQ(sp_parse_hash(sp_str_lit("DEADBEEF")), 0xDEADBEEFU);
  ASSERT_EQ(sp_parse_hash(sp_str_lit("deadbeef")), 0xdeadbeefU);
  ASSERT_EQ(sp_parse_hash(sp_str_lit("ABCD")), 0xABCDU);
  // Would assert: "G", "12345678901", "-1", ""
}

UTEST_F(format, boolean) {
  ASSERT_EQ(sp_parse_bool(sp_str_lit("true")), true);
  ASSERT_EQ(sp_parse_bool(sp_str_lit("false")), false);
  ASSERT_EQ(sp_parse_bool(sp_str_lit("1")), true);
  ASSERT_EQ(sp_parse_bool(sp_str_lit("0")), false);
  // yes/no, on/off not supported - only true/false and 1/0
  // Would assert: "maybe", "2", "TRUE", "", "yes", "no", "on", "off"
}

UTEST_F(format, characters) {
  // sp_parse_c8 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c8(sp_str_lit("'A'")), 'A');
  ASSERT_EQ(sp_parse_c8(sp_str_lit("'z'")), 'z');
  ASSERT_EQ(sp_parse_c8(sp_str_lit("'0'")), '0');
  ASSERT_EQ(sp_parse_c8(sp_str_lit("' '")), ' ');
  ASSERT_EQ(sp_parse_c8(sp_str_lit("'!'")), '!');
  // Would assert: "AB", "", "abc", "A" (no quotes)

  // sp_parse_c16 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c16(sp_str_lit("'A'")), L'A');
  ASSERT_EQ(sp_parse_c16(sp_str_lit("'z'")), L'z');
  ASSERT_EQ(sp_parse_c16(sp_str_lit("'0'")), L'0');
  ASSERT_EQ(sp_parse_c16(sp_str_lit("' '")), L' ');
  ASSERT_EQ(sp_parse_c16(sp_str_lit("'!'")), L'!');
  // Would assert: "AB", "", "abc", "A" (no quotes)
}

UTEST_F(format, extended) {
  u32 u32_val;
  ASSERT_TRUE(sp_parse_u32_ex(sp_str_lit("42"), &u32_val));
  ASSERT_EQ(u32_val, 42U);
  ASSERT_TRUE(sp_parse_u32_ex(sp_str_lit("0"), &u32_val));
  ASSERT_EQ(u32_val, 0U);
  ASSERT_TRUE(sp_parse_u32_ex(sp_str_lit("4294967295"), &u32_val));
  ASSERT_EQ(u32_val, 4294967295U);
  ASSERT_FALSE(sp_parse_u32_ex(sp_str_lit("4294967296"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(sp_str_lit("-1"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(sp_str_lit("abc"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(sp_str_lit(""), &u32_val));

  // sp_parse_s32_ex
  s32 s32_val;
  ASSERT_TRUE(sp_parse_s32_ex(sp_str_lit("42"), &s32_val));
  ASSERT_EQ(s32_val, 42);
  ASSERT_TRUE(sp_parse_s32_ex(sp_str_lit("-42"), &s32_val));
  ASSERT_EQ(s32_val, -42);
  ASSERT_TRUE(sp_parse_s32_ex(sp_str_lit("0"), &s32_val));
  ASSERT_EQ(s32_val, 0);
  ASSERT_TRUE(sp_parse_s32_ex(sp_str_lit("2147483647"), &s32_val));
  ASSERT_EQ(s32_val, 2147483647);
  ASSERT_TRUE(sp_parse_s32_ex(sp_str_lit("-2147483648"), &s32_val));
  ASSERT_EQ(s32_val, INT32_MIN);
  ASSERT_FALSE(sp_parse_s32_ex(sp_str_lit("2147483648"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(sp_str_lit("-2147483649"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(sp_str_lit("text"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(sp_str_lit(""), &s32_val));

  // sp_parse_f32_ex
  f32 f32_val;
  ASSERT_TRUE(sp_parse_f32_ex(sp_str_lit("3.14"), &f32_val));
  ASSERT_NEAR(f32_val, 3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(sp_str_lit("-3.14"), &f32_val));
  ASSERT_NEAR(f32_val, -3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(sp_str_lit("0"), &f32_val));
  ASSERT_NEAR(f32_val, 0.0f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(sp_str_lit("1.23e2"), &f32_val));
  ASSERT_NEAR(f32_val, 123.0f, 1e-5f);
  ASSERT_FALSE(sp_parse_f32_ex(sp_str_lit("abc"), &f32_val));
  ASSERT_FALSE(sp_parse_f32_ex(sp_str_lit(""), &f32_val));

  // sp_parse_f64_ex - NOT IMPLEMENTED (SP_UNIMPLEMENTED)
  // f64 f64_val;
  // ASSERT_TRUE(sp_parse_f64_ex(sp_str_lit("3.14"), &f64_val));

  // sp_parse_bool_ex
  bool bool_val;
  ASSERT_TRUE(sp_parse_bool_ex(sp_str_lit("true"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(sp_str_lit("false"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_TRUE(sp_parse_bool_ex(sp_str_lit("1"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(sp_str_lit("0"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_FALSE(sp_parse_bool_ex(sp_str_lit("maybe"), &bool_val));
  ASSERT_FALSE(sp_parse_bool_ex(sp_str_lit(""), &bool_val));

  // sp_parse_hex_ex
  u64 hex_val;
  ASSERT_TRUE(sp_parse_hex_ex(sp_str_lit("DEADBEEF"), &hex_val));
  ASSERT_EQ(hex_val, 0xDEADBEEFULL);
  ASSERT_TRUE(sp_parse_hex_ex(sp_str_lit("0"), &hex_val));
  ASSERT_EQ(hex_val, 0ULL);
  ASSERT_TRUE(sp_parse_hex_ex(sp_str_lit("FF"), &hex_val));
  ASSERT_EQ(hex_val, 0xFFULL);
  ASSERT_FALSE(sp_parse_hex_ex(sp_str_lit("XYZ"), &hex_val));
  ASSERT_FALSE(sp_parse_hex_ex(sp_str_lit(""), &hex_val));

  // sp_parse_hash_ex
  sp_hash_t hash_val;
  ASSERT_TRUE(sp_parse_hash_ex(sp_str_lit("DEADBEEF"), &hash_val));
  ASSERT_EQ(hash_val, 0xDEADBEEF);
  ASSERT_TRUE(sp_parse_hash_ex(sp_str_lit("0"), &hash_val));
  ASSERT_EQ(hash_val, 0);
  ASSERT_FALSE(sp_parse_hash_ex(sp_str_lit("GHIJKLMN"), &hash_val));
  ASSERT_FALSE(sp_parse_hash_ex(sp_str_lit(""), &hash_val));

  // sp_parse_c8_ex
  c8 c8_val;
  ASSERT_TRUE(sp_parse_c8_ex(sp_str_lit("'A'"), &c8_val));
  ASSERT_EQ(c8_val, 'A');
  ASSERT_TRUE(sp_parse_c8_ex(sp_str_lit("' '"), &c8_val));
  ASSERT_EQ(c8_val, ' ');
  ASSERT_FALSE(sp_parse_c8_ex(sp_str_lit("AB"), &c8_val));
  ASSERT_FALSE(sp_parse_c8_ex(sp_str_lit(""), &c8_val));

  // sp_parse_c16_ex
  u16 c16_val;
  ASSERT_TRUE(sp_parse_c16_ex(sp_str_lit("'Z'"), &c16_val));
  ASSERT_EQ(c16_val, L'Z');
  ASSERT_TRUE(sp_parse_c16_ex(sp_str_lit("'!'"), &c16_val));
  ASSERT_EQ(c16_val, L'!');
  ASSERT_FALSE(sp_parse_c16_ex(sp_str_lit("XY"), &c16_val));
  ASSERT_FALSE(sp_parse_c16_ex(sp_str_lit(""), &c16_val));

  // Additional extended tests for completeness
  u8 u8_val;
  ASSERT_TRUE(sp_parse_u8_ex(sp_str_lit("255"), &u8_val));
  ASSERT_EQ(u8_val, 255);
  ASSERT_FALSE(sp_parse_u8_ex(sp_str_lit("256"), &u8_val));

  u16 u16_val;
  ASSERT_TRUE(sp_parse_u16_ex(sp_str_lit("65535"), &u16_val));
  ASSERT_EQ(u16_val, 65535);
  ASSERT_FALSE(sp_parse_u16_ex(sp_str_lit("65536"), &u16_val));

  u64 u64_val;
  ASSERT_TRUE(sp_parse_u64_ex(sp_str_lit("18446744073709551615"), &u64_val));
  ASSERT_EQ(u64_val, 18446744073709551615ULL);
  ASSERT_FALSE(sp_parse_u64_ex(sp_str_lit("not_a_number"), &u64_val));

  s8 s8_val;
  ASSERT_TRUE(sp_parse_s8_ex(sp_str_lit("-128"), &s8_val));
  ASSERT_EQ(s8_val, -128);
  ASSERT_FALSE(sp_parse_s8_ex(sp_str_lit("-129"), &s8_val));

  s16 s16_val;
  ASSERT_TRUE(sp_parse_s16_ex(sp_str_lit("32767"), &s16_val));
  ASSERT_EQ(s16_val, 32767);
  ASSERT_FALSE(sp_parse_s16_ex(sp_str_lit("32768"), &s16_val));

  s64 s64_val;
  ASSERT_TRUE(sp_parse_s64_ex(sp_str_lit("9223372036854775807"), &s64_val));
  ASSERT_EQ(s64_val, 9223372036854775807LL);
  ASSERT_FALSE(sp_parse_s64_ex(sp_str_lit("invalid"), &s64_val));
}

UTEST_F(format, parse_edge_cases) {
  ASSERT_EQ(sp_parse_u32(sp_str_lit("00042")), 42U);
  ASSERT_EQ(sp_parse_s32(sp_str_lit("-00042")), -42);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("003.14")), 3.14f, 1e-5f);

  ASSERT_EQ(sp_parse_s32(sp_str_lit("+42")), 42);
  ASSERT_NEAR(sp_parse_f32(sp_str_lit("+3.14")), 3.14f, 1e-5f);

  ASSERT_EQ(sp_parse_hex(sp_str_lit("DeAdBeEf")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(sp_str_lit("deadbeef")), 0xdeadbeefULL);

  ASSERT_EQ(sp_parse_u8(sp_str_lit("255")), 255);
  ASSERT_EQ(sp_parse_u16(sp_str_lit("65535")), 65535);
  ASSERT_EQ(sp_parse_u32(sp_str_lit("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u64(sp_str_lit("18446744073709551615")), 18446744073709551615ULL);
}

UTEST_F(format, hex_zero) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_uint(0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0x0"));
}

UTEST_F(format, hex_small) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_uint(0xa)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xA"));
}

UTEST_F(format, hex_no_pad) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_uint(0xa5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xA5"));
}

UTEST_F(format, hex_word) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_uint(0xdeadbeef)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xDEADBEEF"));
}

UTEST_F(format, hex_u64_max) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_uint(0xffffffffffffffffULL)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xFFFFFFFFFFFFFFFF"));
}

UTEST_F(format, hex_signed_negative) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_int(-1)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xFFFFFFFFFFFFFFFF"));
}

UTEST_F(format, hex_mixed_digits) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hex}", sp_fmt_uint(0x1234abcd)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0x1234ABCD"));
}

UTEST_F(format, italic) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.italic}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[3mhi\033[0m"));
}

UTEST_F(format, gray) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.gray}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[90mhi\033[0m"));
}

UTEST_F(format, bright_red) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.br_red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[91mhi\033[0m"));
}

UTEST_F(format, bold_int) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.bold}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[1m42\033[0m"));
}

UTEST_F(format, hyperlink) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.hyperlink}", sp_fmt_cstr("https://x")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033]8;;https://x\033\\https://x\033[0m"));
}

UTEST_F(format, quote_basic) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.quote}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\"hi\""));
}

UTEST_F(format, quote_with_color) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.quote .red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\"\033[31mhi\033[0m\""));
}

UTEST_F(format, red_with_width_pads_outside_ansi) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:>10 .red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "        \033[31mhi\033[0m"));
}

UTEST_F(format, cyan_with_fill_center_pads_outside_ansi) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^9 .cyan}", sp_fmt_cstr("x")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "****\033[36mx\033[0m****"));
}

UTEST_F(format, bold_with_width_on_int) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:>6 .bold}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    \033[1m42\033[0m"));
}

UTEST_F(format, quote_with_width_pads_outside_quotes) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^8 .quote}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***\"hi\"***"));
}

static void _test_before_only(sp_io_writer_t* io, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_io_write_cstr(io, "[", SP_NULLPTR);
}

UTEST_F(format, null_after_callback) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("openonly", _test_before_only, SP_NULLPTR);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.openonly}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[hi"));
  sp_fmt_directive_reset();
}

UTEST_F(format, null_before_callback) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("closeonly", SP_NULLPTR, _test_after_gt);
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.closeonly}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hi>"));
  sp_fmt_directive_reset();
}

UTEST_F(format, no_allocator) {
  c8 buffer[64] = sp_zero;
  sp_str_t got = sp_fmt_buf(buffer, 64, "hello, {}", sp_fmt_cstr("world")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello, world"));
}

UTEST_F(format, with_decorator_and_width) {
  c8 buffer[64] = sp_zero;
  sp_str_t got = sp_fmt_buf(buffer, 64, "{:>6 .red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    \033[31mhi\033[0m"));
}

UTEST_F(format, multiple_calls_same_writer) {
  c8 buffer[64] = sp_zero;
  sp_io_mem_writer_t w = sp_zero;
  sp_io_mem_writer_from_buffer(&w, buffer, sizeof(buffer));
  sp_fmt_io(&w.base, "[{}]", sp_fmt_cstr("a"));
  sp_io_write_c8(&w.base, ' ');
  sp_fmt_io(&w.base, "[{}]", sp_fmt_cstr("b"));
  sp_str_t got = sp_io_mem_writer_as_str(&w);
  EXPECT_TRUE(sp_str_equal_cstr(got, "[a] [b]"));
}

UTEST_F(format, pointer) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(),
    "{:>16}", sp_fmt_ptr((void*)(uintptr_t)0xabcdULL)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "          0xabcd"));
}

UTEST_F(format, negative_int_right_align) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:>8}", sp_fmt_int(-42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "     -42"));
}

UTEST_F(format, negative_int_zero_fill) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:0>8}", sp_fmt_int(-42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "00000-42"));
}

UTEST_F(format, empty_string_center_fill) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^6}", sp_fmt_cstr("")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "******"));
}

UTEST_F(format, float_precision_and_width) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:>10.2}", sp_fmt_float(3.14159)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "      3.14"));
}

UTEST_F(format, float_precision_center_fill) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{:*^10.2}", sp_fmt_float(1.5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***1.50***"));
}

UTEST_F(format, bytes_zero) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.bytes}", sp_fmt_uint(0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0 B"));
}

UTEST_F(format, bytes_below_kb) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.bytes}", sp_fmt_uint(1023)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1023 B"));
}

UTEST_F(format, bytes_exact_kb) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.bytes}", sp_fmt_uint(1024)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1 KB"));
}

UTEST_F(format, bytes_exact_mb) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.bytes}", sp_fmt_uint(1024ULL * 1024ULL)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1 MB"));
}

UTEST_F(format, bytes_fractional) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.bytes}", sp_fmt_uint(1536)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.5 KB"));
}

UTEST_F(format, ordinal_1st) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(1)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1st"));
}

UTEST_F(format, ordinal_2nd) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(2)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "2nd"));
}

UTEST_F(format, ordinal_3rd) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(3)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "3rd"));
}

UTEST_F(format, ordinal_4th) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(4)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "4th"));
}

UTEST_F(format, ordinal_11th) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(11)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "11th"));
}

UTEST_F(format, ordinal_12th) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(12)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "12th"));
}

UTEST_F(format, ordinal_13th) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(13)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "13th"));
}

UTEST_F(format, ordinal_21st) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(21)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "21st"));
}

UTEST_F(format, ordinal_negative) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.ordinal}", sp_fmt_int(-1)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "-1st"));
}

UTEST_F(format, duration_sub_us) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.duration}", sp_fmt_uint(999)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "999 ns"));
}

UTEST_F(format, duration_exact_us) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.duration}", sp_fmt_uint(1000)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1 us"));
}

UTEST_F(format, duration_fractional_ms) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.duration}", sp_fmt_uint(1500000)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.5 ms"));
}

UTEST_F(format, duration_exact_s) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.duration}", sp_fmt_uint(1000000000)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1 s"));
}

UTEST_F(format, iso_epoch_zero) {
  sp_str_t got = sp_fmt(sp_mem_get_scratch(), "{.iso}", sp_fmt_uint(0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1970-01-01T00:00:00.000Z"));
}

