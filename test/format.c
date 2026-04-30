#define SP_PRIVATE_HEADER // We need a few internal parser structs and functions
#include "sp.h"

#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

static sp_str_ht(sp_fmt_directive_t) sp_fmt_directives = SP_NULLPTR;

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
  sp_fmt_spec_t specifier = SP_ZERO_INITIALIZE();
  sp_err_t err = parse_spec(test.str, &specifier);
  EXPECT_EQ(err, test.err);
  if (err != SP_OK) return;
  EXPECT_EQ(specifier.align, test.expected.align);
  EXPECT_EQ(specifier.fill, test.expected.fill);
  EXPECT_EQ(specifier.precision.some, test.expected.precision.some);
  EXPECT_EQ(specifier.precision.value, test.expected.precision.value);
  EXPECT_EQ(specifier.width, test.expected.width);
  EXPECT_EQ(specifier.directive_count, test.expected.directive_count);
  EXPECT_TRUE(specifier.fill_dynamic == test.expected.fill_dynamic);
  EXPECT_TRUE(specifier.width_dynamic == test.expected.width_dynamic);
  EXPECT_TRUE(specifier.precision_dynamic == test.expected.precision_dynamic);
}

UTEST(sp_fmt_parse, smoke) {
  run_case(utest_result, (format_test_t) {
    .str = "{:*^9}",
    .expected = {
      .width = 9,
      .align = SP_FMT_ALIGN_CENTER,
      .fill =  '*',
    },
  });
}

UTEST(sp_fmt_parse, empty) {
  run_case(utest_result, (format_test_t) {
    .str = "{}",
    .expected = { .align = SP_FMT_ALIGN_NONE },
  });
}

UTEST(sp_fmt_parse, empty_with_colon) {
  run_case(utest_result, (format_test_t) {
    .str = "{:}",
    .expected = { .align = SP_FMT_ALIGN_NONE },
  });
}

UTEST(sp_fmt_parse, width_only) {
  run_case(utest_result, (format_test_t) {
    .str = "{:42}",
    .expected = { .width = 42 },
  });
}

UTEST(sp_fmt_parse, precision_only) {
  run_case(utest_result, (format_test_t) {
    .str = "{:.5}",
    .expected = { .precision = sp_opt_some(5) },
  });
}

UTEST(sp_fmt_parse, width_and_precision) {
  run_case(utest_result, (format_test_t) {
    .str = "{:10.3}",
    .expected = { .width = 10, .precision = sp_opt_some(3) },
  });
}

UTEST(sp_fmt_parse, bare_align_left) {
  run_case(utest_result, (format_test_t) {
    .str = "{:<7}",
    .expected = { .width = 7, .align = SP_FMT_ALIGN_LEFT },
  });
}

UTEST(sp_fmt_parse, bare_align_right) {
  run_case(utest_result, (format_test_t) {
    .str = "{:>4}",
    .expected = { .width = 4, .align = SP_FMT_ALIGN_RIGHT },
  });
}

UTEST(sp_fmt_parse, fill_with_left_align) {
  run_case(utest_result, (format_test_t) {
    .str = "{:-<8}",
    .expected = { .width = 8, .align = SP_FMT_ALIGN_LEFT, .fill = '-' },
  });
}

UTEST(sp_fmt_parse, everything) {
  run_case(utest_result, (format_test_t) {
    .str = "{:*^12.4}",
    .expected = {
      .width = 12,
      .precision = sp_opt_some(4),
      .align = SP_FMT_ALIGN_CENTER,
      .fill = '*',
    },
  });
}

UTEST(sp_fmt_parse, zero_leading_width) {
  run_case(utest_result, (format_test_t) {
    .str = "{:05}",
    .expected = { .width = 5 },
  });
}

UTEST(sp_fmt_parse, dynamic_width) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$}",
    .expected = { .width_dynamic = 1 },
  });
}

UTEST(sp_fmt_parse, dynamic_precision) {
  run_case(utest_result, (format_test_t) {
    .str = "{:.$}",
    .expected = { .precision_dynamic = 1 },
  });
}

UTEST(sp_fmt_parse, dynamic_fill_center) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$^9}",
    .expected = {
      .width = 9,
      .align = SP_FMT_ALIGN_CENTER,
      .fill_dynamic = 1,
    },
  });
}

UTEST(sp_fmt_parse, dynamic_fill_and_width) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$^$}",
    .expected = {
      .align = SP_FMT_ALIGN_CENTER,
      .fill_dynamic = 1,
      .width_dynamic = 1,
    },
  });
}

UTEST(sp_fmt_parse, dynamic_everything) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$<$.$}",
    .expected = {
      .align = SP_FMT_ALIGN_LEFT,
      .fill_dynamic = 1,
      .width_dynamic = 1,
      .precision_dynamic = 1,
    },
  });
}

UTEST(sp_fmt_parse, dynamic_width_with_literal_precision) {
  run_case(utest_result, (format_test_t) {
    .str = "{:$.4}",
    .expected = {
      .precision = sp_opt_some(4),
      .width_dynamic = 1,
    },
  });
}

UTEST(sp_fmt_parse, err_dynamic_precision_without_dot) {
  run_case(utest_result, (format_test_t) {
    .str = "{:5$}",
    .err = SP_ERR_FMT_BAD_PLACEHOLDER
  });
}

UTEST(sp_fmt_parse, err_missing_open_brace) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec(":5}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_parse, err_missing_close_brace) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:5", &spec), SP_ERR_FMT_UNTERMINATED_PLACEHOLDER);
}

UTEST(sp_fmt_parse, err_dot_without_digits) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:5.}", &spec), SP_ERR_FMT_BAD_PRECISION);
}

UTEST(sp_fmt_parse_directive, single) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.red}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "red"));
}

UTEST(sp_fmt_parse_directive, after_width) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:10 .red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 10);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "red"));
}

UTEST(sp_fmt_parse_directive, full_spec) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:*^9.2 .red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 9);
  EXPECT_EQ(spec.precision.value, 2);
  EXPECT_EQ(spec.precision.some, SP_OPT_SOME);
  EXPECT_EQ(spec.align, SP_FMT_ALIGN_CENTER);
  EXPECT_EQ(spec.fill, '*');
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "red"));
}

UTEST(sp_fmt_parse_directive, multiple) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.red .bold .upper}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 3);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "red"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[1], "bold"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[2], "upper"));
}

UTEST(sp_fmt_parse_directive, max_count) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.a .b .c .d .e .f .g .h}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, SP_FMT_MAX_DIRECTIVES);
}

UTEST(sp_fmt_parse_directive, err_too_many) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.a .b .c .d .e .f .g .h .i}", &spec), SP_ERR_FMT_TOO_MANY_DIRECTIVES);
}

UTEST(sp_fmt_parse_directive, err_empty_name) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:10 .}", &spec), SP_ERR_FMT_BAD_DIRECTIVE);
}

UTEST(sp_fmt_parse_directive, err_bad_char) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:10 .red!}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_parse_directive, err_no_space_between_directives) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.red.bold}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_parse_directive, err_no_dot) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:10 red}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_parse_directive, digit_in_tail) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.base64}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "base64"));
}

UTEST(sp_fmt_parse_directive, hyphen_in_name) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.utf-8}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "utf-8"));
}

UTEST(sp_fmt_parse_directive, underscore_in_name) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.http_url}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "http_url"));
}

UTEST(sp_fmt_parse_directive, err_leading_digit) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{: .4red}", &spec), SP_ERR_FMT_BAD_DIRECTIVE);
}

UTEST(sp_fmt_parse_directive, err_leading_hyphen) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.-red}", &spec), SP_ERR_FMT_BAD_DIRECTIVE);
}

UTEST(sp_fmt_parse_directive, no_width_just_directive) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{: .red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 0);
  EXPECT_EQ(spec.directive_count, 1);
}

static sp_str_t render_value_to_str(sp_fmt_arg_t arg) {
  sp_str_builder_t b = SP_ZERO_INITIALIZE();
  sp_fmt_render_default(&b, &arg, SP_NULLPTR);
  return sp_str_builder_to_str(&b);
}

UTEST(sp_fmt_render, u64_zero) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(0));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0"));
}

UTEST(sp_fmt_render, u64_large) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(1234567));
  EXPECT_TRUE(sp_str_equal_cstr(got, "1234567"));
}

UTEST(sp_fmt_render, s64_negative) {
  sp_str_t got = render_value_to_str(sp_fmt_int(-42));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-42"));
}

UTEST(sp_fmt_render, s64_positive) {
  sp_str_t got = render_value_to_str(sp_fmt_int(42));
  EXPECT_TRUE(sp_str_equal_cstr(got, "42"));
}

UTEST(sp_fmt_render, str) {
  sp_str_t got = render_value_to_str(sp_fmt_cstr("hello"));
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello"));
}

UTEST(sp_fmt_render, empty_str) {
  sp_str_t got = render_value_to_str(sp_fmt_cstr(""));
  EXPECT_TRUE(sp_str_equal_cstr(got, ""));
}

UTEST(sp_fmt_render, u64_ten) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(10));
  EXPECT_TRUE(sp_str_equal_cstr(got, "10"));
}

UTEST(sp_fmt_render, u64_ninety_nine) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(99));
  EXPECT_TRUE(sp_str_equal_cstr(got, "99"));
}

UTEST(sp_fmt_render, u64_one_hundred) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(100));
  EXPECT_TRUE(sp_str_equal_cstr(got, "100"));
}

UTEST(sp_fmt_render, u64_max) {
  sp_str_t got = render_value_to_str(sp_fmt_uint(0xffffffffffffffffULL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "18446744073709551615"));
}

UTEST(sp_fmt_render, s64_min) {
  sp_str_t got = render_value_to_str(sp_fmt_int((s64)0x8000000000000000LL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-9223372036854775808"));
}

UTEST(sp_fmt_render, s64_max) {
  sp_str_t got = render_value_to_str(sp_fmt_int((s64)0x7fffffffffffffffLL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "9223372036854775807"));
}

UTEST(sp_fmt_render, f64_zero) {
  sp_str_t got = render_value_to_str(sp_fmt_float(0.0));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0.000000"));
}

UTEST(sp_fmt_render, f64_one) {
  sp_str_t got = render_value_to_str(sp_fmt_float(1.0));
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.000000"));
}

UTEST(sp_fmt_render, f64_half) {
  sp_str_t got = render_value_to_str(sp_fmt_float(0.5));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0.500000"));
}

UTEST(sp_fmt_render, f64_negative) {
  sp_str_t got = render_value_to_str(sp_fmt_float(-3.25));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-3.250000"));
}

UTEST(sp_fmt_render, f64_carry) {
  sp_str_t got = render_value_to_str(sp_fmt_float(0.9999995));
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.000000"));
}

UTEST(sp_fmt_render, f64_nan) {
  union { f64 f; u64 u; } nan_bits = { .u = 0x7ff8000000000000ULL };
  sp_str_t got = render_value_to_str(sp_fmt_float(nan_bits.f));
  EXPECT_TRUE(sp_str_equal_cstr(got, "nan"));
}

UTEST(sp_fmt_render, f64_pos_inf) {
  union { f64 f; u64 u; } inf_bits = { .u = 0x7ff0000000000000ULL };
  sp_str_t got = render_value_to_str(sp_fmt_float(inf_bits.f));
  EXPECT_TRUE(sp_str_equal_cstr(got, "inf"));
}

UTEST(sp_fmt_render, f64_neg_inf) {
  union { f64 f; u64 u; } inf_bits = { .u = 0xfff0000000000000ULL };
  sp_str_t got = render_value_to_str(sp_fmt_float(inf_bits.f));
  EXPECT_TRUE(sp_str_equal_cstr(got, "-inf"));
}

UTEST(sp_fmt_render, f64_custom_precision_via_spec) {
  sp_fmt_arg_t arg = sp_fmt_float(3.14159);
  sp_opt_set(arg.spec.precision, 2);
  sp_str_builder_t b = SP_ZERO_INITIALIZE();
  sp_fmt_render_default(&b, &arg, SP_NULLPTR);
  sp_str_t got = sp_str_builder_to_str(&b);
  EXPECT_TRUE(sp_str_equal_cstr(got, "3.14"));
}

UTEST(sp_fmt_render, ptr_null) {
  sp_str_t got = render_value_to_str(sp_fmt_ptr(SP_NULLPTR));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0x0"));
}

UTEST(sp_fmt_render, ptr_nonzero) {
  sp_str_t got = render_value_to_str(sp_fmt_ptr((void*)(uintptr_t)0xdeadbeefULL));
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xdeadbeef"));
}

static sp_str_t apply_spec_to_str(const c8* content, sp_fmt_spec_t spec) {
  sp_str_builder_t b = SP_ZERO_INITIALIZE();
  sp_fmt_apply_spec(&b, sp_str_view(content), spec);
  return sp_str_builder_to_str(&b);
}

UTEST(sp_fmt_pad, no_width) {
  sp_str_t got = apply_spec_to_str("hello", SP_ZERO_STRUCT(sp_fmt_spec_t));
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello"));
}

UTEST(sp_fmt_pad, content_longer_than_width) {
  sp_str_t got = apply_spec_to_str("hello", (sp_fmt_spec_t){ .width = 3 });
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello"));
}

UTEST(sp_fmt_pad, right_align_default) {
  sp_str_t got = apply_spec_to_str("42", (sp_fmt_spec_t){ .width = 6 });
  EXPECT_TRUE(sp_str_equal_cstr(got, "    42"));
}

UTEST(sp_fmt_pad, left_align_fill) {
  sp_str_t got = apply_spec_to_str("42", (sp_fmt_spec_t){
    .width = 6, .align = SP_FMT_ALIGN_LEFT, .fill = '-',
  });
  EXPECT_TRUE(sp_str_equal_cstr(got, "42----"));
}

UTEST(sp_fmt_pad, center_even) {
  sp_str_t got = apply_spec_to_str("hi", (sp_fmt_spec_t){
    .width = 8, .align = SP_FMT_ALIGN_CENTER, .fill = '*',
  });
  EXPECT_TRUE(sp_str_equal_cstr(got, "***hi***"));
}

UTEST(sp_fmt_pad, center_odd) {
  sp_str_t got = apply_spec_to_str("hi", (sp_fmt_spec_t){
    .width = 9, .align = SP_FMT_ALIGN_CENTER, .fill = '*',
  });
  EXPECT_TRUE(sp_str_equal_cstr(got, "***hi****"));
}

UTEST(sp_fmt_pad, wrapped_padding_outside) {
  sp_str_builder_t b = SP_ZERO_INITIALIZE();
  sp_fmt_apply_spec_wrapped(&b,
    sp_str_view("<"),
    sp_str_view("42"),
    sp_str_view(">"),
    (sp_fmt_spec_t){ .width = 6 }
  );
  sp_str_t got = sp_str_builder_to_str(&b);
  EXPECT_TRUE(sp_str_equal_cstr(got, "    <42>"));
}

UTEST(sp_fmt_pad, wrapped_center) {
  sp_str_builder_t b = SP_ZERO_INITIALIZE();
  sp_fmt_apply_spec_wrapped(&b,
    sp_str_view("["),
    sp_str_view("hi"),
    sp_str_view("]"),
    (sp_fmt_spec_t){ .width = 8, .align = SP_FMT_ALIGN_CENTER, .fill = '*' }
  );
  sp_str_t got = sp_str_builder_to_str(&b);
  EXPECT_TRUE(sp_str_equal_cstr(got, "***[hi]***"));
}

static void _test_before_lt(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "<");
}

static void _test_after_gt(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, ">");
}

static void _test_render_x(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "X");
}

static void _test_transform_upper(sp_str_builder_t* out, sp_str_t content, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_for(k, content.len) {
    c8 c = content.data[k];
    sp_str_builder_append_c8(out, (c >= 'a' && c <= 'z') ? (c8)(c - 32) : c);
  }
}

static sp_str_builder_t _test_log;
static u32 _test_render_y_calls;

static void _test_before_a(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "[a");
  sp_str_builder_append_cstr(&_test_log, "ba");
}

static void _test_after_a(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "a]");
  sp_str_builder_append_cstr(&_test_log, "aa");
}

static void _test_before_b(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "[b");
  sp_str_builder_append_cstr(&_test_log, "bb");
}

static void _test_after_b(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "b]");
  sp_str_builder_append_cstr(&_test_log, "ab");
}

static void _test_render_y(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  _test_render_y_calls++;
  sp_str_builder_append_cstr(b, "Y");
}


UTEST(sp_fmt_directive, register_and_lookup) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);

  sp_fmt_directive_t* got = sp_fmt_directive_lookup(sp_str_view("wrap"));
  EXPECT_TRUE(got != SP_NULLPTR);
  EXPECT_EQ(got->kind, sp_fmt_directive_decorator);
  EXPECT_TRUE(got->decorator.before == _test_before_lt);
  EXPECT_TRUE(got->decorator.after == _test_after_gt);

  sp_str_t rendered = sp_fmt_a(sp_context_get_allocator(), "{.wrap}", sp_fmt_cstr("ok")).value;
  EXPECT_TRUE(sp_str_equal_cstr(rendered, "<ok>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, lookup_unknown_returns_null) {
  sp_fmt_directive_reset();
  sp_fmt_directive_t* got = sp_fmt_directive_lookup(sp_str_view("nope"));
  EXPECT_EQ(got, SP_NULLPTR);
}

UTEST(sp_fmt_directive, reset_clears) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("foo", _test_before_lt, SP_NULLPTR);
  EXPECT_TRUE(sp_fmt_directive_lookup(sp_str_view("foo")) != SP_NULLPTR);
  sp_fmt_directive_reset();
  EXPECT_EQ(sp_fmt_directive_lookup(sp_str_view("foo")), SP_NULLPTR);
}

UTEST(sp_fmt_directive, wraps_content) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.wrap}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<hi>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, render_replaces_value) {
  sp_fmt_directive_reset();
  sp_fmt_register_renderer("x", _test_render_x, 0);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.x}", sp_fmt_int(999)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "X"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, transform_uppercase) {
  sp_fmt_directive_reset();
  sp_fmt_register_transformer("upper", _test_transform_upper);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.upper}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "HELLO"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, err_unknown_directive) {
  sp_fmt_directive_reset();
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.missing}", sp_fmt_int(42)).err, SP_ERR_FMT_UNKNOWN_DIRECTIVE);
}

UTEST(sp_fmt_directive, ordering_bracket_nested) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("a", _test_before_a, _test_after_a);
  sp_fmt_register_decorator("b", _test_before_b, _test_after_b);

  _test_log = (sp_str_builder_t)SP_ZERO_INITIALIZE();
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.a .b}", sp_fmt_cstr("x")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[a[bxb]a]"));

  sp_str_t log_str = sp_str_builder_to_str(&_test_log);
  EXPECT_TRUE(sp_str_equal_cstr(log_str, "babbabaa"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, err_multiple_renders) {
  sp_fmt_directive_reset();
  _test_render_y_calls = 0;
  sp_fmt_register_renderer("x", _test_render_x, 0);
  sp_fmt_register_renderer("y", _test_render_y, 0);
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.x .y}", sp_fmt_int(0)).err, SP_ERR_FMT_TOO_MANY_RENDERERS);
  EXPECT_EQ(_test_render_y_calls, 0);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, padding_outside_wrappers) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:6 .wrap}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    <42>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, padding_with_center_and_wrapper) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:*^8 .wrap}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***<hi>***"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_v, literal_only) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "hello, world").value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello, world"));
}

UTEST(sp_fmt_v, empty_placeholder_int) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "42"));
}

UTEST(sp_fmt_v, empty_placeholder_str) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{}", sp_fmt_cstr("world")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "world"));
}

UTEST(sp_fmt_v, multi_arg) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{} + {} = {}",
    sp_fmt_int(2), sp_fmt_int(3), sp_fmt_int(5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "2 + 3 = 5"));
}

UTEST(sp_fmt_v, literals_around_placeholder) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "hello, {}!", sp_fmt_cstr("thomas")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello, thomas!"));
}

UTEST(sp_fmt_v, width_right_align) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:6}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    42"));
}

UTEST(sp_fmt_v, fill_center) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:*^9}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***42****"));
}

UTEST(sp_fmt_v, brace_escapes) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{{{}}}", sp_fmt_int(7)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "{7}"));
}

UTEST(sp_fmt_v, close_brace_escape) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "hello }} world").value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hello } world"));
}

UTEST(sp_fmt_v, err_lone_close_brace) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "oops } here").err, SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_v, str_with_padding) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "[{:->8}]", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[------hi]"));
}

UTEST(sp_fmt_directive, custom_fn_fallback) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  u32 value = 0;
  sp_fmt_arg_t arg = sp_fmt_custom(u32, _test_render_x, value);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.wrap}", arg).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<X>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, default_render_with_wrappers_on_int) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.wrap}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<42>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, content_wider_than_width_with_wrapper) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:3 .wrap}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<hello>"));
  sp_fmt_directive_reset();
}

static void _test_render_prefixed(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "rendered");
}

UTEST(sp_fmt_directive, before_render_then_transform) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, SP_NULLPTR);
  sp_fmt_register_renderer("prefix", _test_render_prefixed, 0);
  sp_fmt_register_transformer("upper", _test_transform_upper);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.wrap .prefix .upper}", sp_fmt_int(0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<RENDERED"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_parse_directive, err_alpha_after_digit) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:5red}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_pad, wrapped_content_overflow) {
  sp_str_builder_t b = SP_ZERO_INITIALIZE();
  sp_fmt_apply_spec_wrapped(&b,
    sp_str_view("<"),
    sp_str_view("hello"),
    sp_str_view(">"),
    (sp_fmt_spec_t){ .width = 3 }
  );
  sp_str_t got = sp_str_builder_to_str(&b);
  EXPECT_TRUE(sp_str_equal_cstr(got, "<hello>"));
}

UTEST(sp_fmt_v, escaped_braces_around_placeholder) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{{{:5}}}", sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "{   42}"));
}

UTEST(sp_fmt_v, dynamic_width) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$}", sp_fmt_int(6), sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    42"));
}

UTEST(sp_fmt_v, dynamic_fill_center) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$^9}", sp_fmt_int('*'), sp_fmt_int(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "***42****"));
}

UTEST(sp_fmt_v, dynamic_fill_and_width) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$^$}",
    sp_fmt_int('-'),
    sp_fmt_int(8),
    sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "---hi---"));
}

UTEST(sp_fmt_v, dynamic_precision) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:.$}", sp_fmt_int(3), sp_fmt_float(3.14159)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "3.142"));
}

UTEST(sp_fmt_v, dynamic_width_with_literal_precision) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$.2}", sp_fmt_int(8), sp_fmt_float(1.5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "    1.50"));
}

UTEST(sp_fmt_v, err_parse_stops_formatting) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "a {:5.} b {}", sp_fmt_int(99)).err, SP_ERR_FMT_BAD_PRECISION);
}

UTEST(sp_fmt_v, err_unterminated_placeholder) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "hi {nope", sp_fmt_int(1)).err, SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_v, err_dynamic_fill_wrong_kind) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{:$^5}", sp_fmt_float(1.0), sp_fmt_int(42)).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
}

UTEST(sp_fmt_v, err_dynamic_width_wrong_kind) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{:$}", sp_fmt_cstr("oops"), sp_fmt_int(42)).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
}

UTEST(sp_fmt_v, err_dynamic_precision_wrong_kind) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{:.$}", sp_fmt_float(3.0), sp_fmt_float(3.14)).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
}

UTEST(sp_fmt_v, err_stops_subsequent_placeholders) {
  sp_fmt_directive_reset();
  sp_str_t str = sp_zero();
  sp_err_t err = sp_fmt_a(sp_context_get_allocator(), "{} {.nope} {}", sp_fmt_int(1), sp_fmt_int(2), sp_fmt_int(3)).err;
  EXPECT_EQ(err, SP_ERR_FMT_UNKNOWN_DIRECTIVE);
}

UTEST(sp_fmt_v, str_precision_truncates) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:.3}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hel"));
}

UTEST(sp_fmt_v, str_precision_longer_than_string) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:.10}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hi"));
}

UTEST(sp_fmt_v, str_dynamic_precision_truncates) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:.$}", sp_fmt_int(2), sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "he"));
}

UTEST(sp_fmt_v, str_precision_with_width) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "[{:>6.3}]", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "[   hel]"));
}

UTEST(sp_fmt_v, f64_precision_zero_means_zero) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:.0}", sp_fmt_float(3.7)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "4"));
}

UTEST(sp_fmt_v, f64_dynamic_precision_zero) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:.$}", sp_fmt_int(0), sp_fmt_float(3.7)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "4"));
}

UTEST(sp_fmt_v, f64_no_precision_defaults_to_six) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{}", sp_fmt_float(1.5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "1.500000"));
}

UTEST(sp_fmt_v, width_clamped_literal) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:99999}", sp_fmt_cstr("x")).value;
  EXPECT_EQ(got.len, SP_FMT_WIDTH_MAX);
}

UTEST(sp_fmt_v, width_clamped_dynamic_huge) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$}", sp_fmt_uint(999999999ULL), sp_fmt_cstr("x")).value;
  EXPECT_EQ(got.len, SP_FMT_WIDTH_MAX);
}

UTEST(sp_fmt_v, width_clamped_dynamic_negative) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$}", sp_fmt_int(-5), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "hi"));
}

static void _test_render_u64_only(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)param;
  sp_fmt_write_u64(b, arg->u);
}

UTEST(sp_fmt_directive, kinds_single_accepts_match) {
  sp_fmt_directive_reset();
  sp_fmt_register_renderer("only_u64", _test_render_u64_only, sp_fmt_id_u64);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.only_u64}", sp_fmt_uint(42)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "42"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, kinds_single_rejects_mismatch) {
  sp_fmt_directive_reset();
  sp_fmt_register_renderer("only_u64", _test_render_u64_only, sp_fmt_id_u64);
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.only_u64}", sp_fmt_float(1.5)).err, SP_ERR_FMT_WRONG_PARAM_KIND);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, kinds_multiple_accepts_either) {
  sp_fmt_directive_reset();
  sp_fmt_directive_register("num", (sp_fmt_directive_t){
    .kind = sp_fmt_directive_decorator,
    .arg_kinds = sp_fmt_id_u64 | sp_fmt_id_s64,
  });
  sp_str_t a = sp_fmt_a(sp_context_get_allocator(), "{.num}", sp_fmt_uint(7)).value;
  sp_str_t b = sp_fmt_a(sp_context_get_allocator(), "{.num}", sp_fmt_int(-3)).value;
  EXPECT_TRUE(sp_str_equal_cstr(a, "7"));
  EXPECT_TRUE(sp_str_equal_cstr(b, "-3"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, kinds_multiple_rejects_outsider) {
  sp_fmt_directive_reset();
  sp_fmt_directive_register("num", (sp_fmt_directive_t){
    .kind = sp_fmt_directive_decorator,
    .arg_kinds = sp_fmt_id_u64 | sp_fmt_id_s64,
  });
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.num}", sp_fmt_cstr("nope")).err, SP_ERR_FMT_WRONG_PARAM_KIND);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive, kinds_unset_accepts_all) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("any", _test_before_lt, _test_after_gt);
  sp_str_t a = sp_fmt_a(sp_context_get_allocator(), "{.any}", sp_fmt_int(1)).value;
  sp_str_t b = sp_fmt_a(sp_context_get_allocator(), "{.any}", sp_fmt_cstr("hi")).value;
  sp_str_t c = sp_fmt_a(sp_context_get_allocator(), "{.any}", sp_fmt_float(2.0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(a, "<1>"));
  EXPECT_TRUE(sp_str_equal_cstr(b, "<hi>"));
  EXPECT_TRUE(sp_str_equal_cstr(c, "<2.000000>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_parse_directive_arg, literal) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg red}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "fg"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[0], "red"));
  EXPECT_EQ(spec.directive_arg_dynamic, 0);
}

UTEST(sp_fmt_parse_directive_arg, dynamic) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg $}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "fg"));
  EXPECT_EQ(spec.directive_args[0].len, 0);
  EXPECT_EQ(spec.directive_arg_dynamic, 1);
}

UTEST(sp_fmt_parse_directive_arg, literal_then_directive) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg red .bold}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 2);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[0], "fg"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[0], "red"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[1], "bold"));
  EXPECT_EQ(spec.directive_args[1].len, 0);
}

UTEST(sp_fmt_parse_directive_arg, dynamic_then_directive) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg $ .bold}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 2);
  EXPECT_EQ(spec.directive_arg_dynamic, 1);
}

UTEST(sp_fmt_parse_directive_arg, two_literals) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg red .bg blue}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 2);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[0], "red"));
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[1], "blue"));
  EXPECT_EQ(spec.directive_arg_dynamic, 0);
}

UTEST(sp_fmt_parse_directive_arg, two_dynamics) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg $ .bg $}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 2);
  EXPECT_EQ(spec.directive_arg_dynamic, 0x3);
}

UTEST(sp_fmt_parse_directive_arg, mixed) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg red .bg $ .bold}", &spec), SP_OK);
  EXPECT_EQ(spec.directive_count, 3);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[0], "red"));
  EXPECT_EQ(spec.directive_args[1].len, 0);
  EXPECT_EQ(spec.directive_arg_dynamic, 0x2);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_names[2], "bold"));
}

UTEST(sp_fmt_parse_directive_arg, with_full_spec) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{:*^9.2 .fg red}", &spec), SP_OK);
  EXPECT_EQ(spec.width, 9);
  EXPECT_EQ(spec.directive_count, 1);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[0], "red"));
}

UTEST(sp_fmt_parse_directive_arg, arg_with_digits_and_symbols) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.hex 0xff}", &spec), SP_OK);
  EXPECT_TRUE(sp_str_equal_cstr(spec.directive_args[0], "0xff"));
}

UTEST(sp_fmt_parse_directive_arg, err_dollar_inside_literal) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg r$d}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_parse_directive_arg, err_dollar_followed_by_literal) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg $red}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

UTEST(sp_fmt_parse_directive_arg, err_space_in_arg) {
  sp_fmt_spec_t spec = SP_ZERO_INITIALIZE();
  EXPECT_EQ(parse_spec("{.fg red blue}", &spec), SP_ERR_FMT_BAD_PLACEHOLDER);
}

static sp_str_t _last_fg_param;
static bool _last_fg_had_param;
static void _test_fg_before(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg;
  _last_fg_had_param = (param != SP_NULLPTR);
  if (param && param->id == sp_fmt_id_str) {
    _last_fg_param = param->s;
    sp_str_builder_append_cstr(b, "<fg=");
    sp_str_builder_append(b, param->s);
    sp_str_builder_append_cstr(b, ">");
  }
  else {
    sp_str_builder_append_cstr(b, "<fg=?>");
  }
}

static void _test_fg_after(sp_str_builder_t* b, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_str_builder_append_cstr(b, "</fg>");
}

UTEST(sp_fmt_directive_arg, literal_passed_as_str) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg red}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=red>hello</fg>"));
  EXPECT_TRUE(_last_fg_had_param);
  EXPECT_TRUE(sp_str_equal_cstr(_last_fg_param, "red"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, dynamic_passed_as_str) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg $}", sp_fmt_cstr("blue"), sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=blue>hello</fg>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, dynamic_accepts_u64_with_mask) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str | sp_fmt_id_u64);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg $}", sp_fmt_uint(31), sp_fmt_cstr("x")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=?>x</fg>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, err_missing_arg) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.fg}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_MISSING);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, err_unexpected_arg) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("bold", _test_before_lt, _test_after_gt);
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.bold red}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_UNEXPECTED);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, err_wrong_literal_kind) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("numpad", _test_before_lt, _test_after_gt, sp_fmt_id_u64);
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.numpad abc}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, err_wrong_dynamic_kind) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.fg $}", sp_fmt_uint(5), sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_WRONG_KIND);
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_directive_arg, chain_of_literal_and_dynamic) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_fmt_register_decorator("bold", _test_before_lt, _test_after_gt);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg $ .bold}", sp_fmt_cstr("cyan"), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<fg=cyan><hi></fg>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_builtin_fg, literal_color) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[31mhi\033[0m"));
}

UTEST(sp_fmt_builtin_fg, literal_bright_cyan) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg brightcyan}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[96mhi\033[0m"));
}

UTEST(sp_fmt_builtin_fg, dynamic_color) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.fg $}", sp_fmt_cstr("green"), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "\033[32mhi\033[0m"));
}

UTEST(sp_fmt_builtin_fg, composes_with_padding) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:*^6 .fg red}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "**\033[31mhi\033[0m**"));
}

UTEST(sp_fmt_builtin_fg, err_missing_arg) {
  sp_str_t str = sp_zero();
  EXPECT_EQ(sp_fmt_a(sp_context_get_allocator(), "{.fg}", sp_fmt_cstr("hi")).err, SP_ERR_FMT_DIRECTIVE_ARG_MISSING);
}

UTEST(sp_fmt_directive_arg, dynamic_param_interleaved_with_width) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator_p("fg", _test_fg_before, _test_fg_after, sp_fmt_id_str);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:$ .fg $}", sp_fmt_int(4), sp_fmt_cstr("red"), sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "  <fg=red>hi</fg>"));
  sp_fmt_directive_reset();
}

static void _test_transform_redact(sp_str_builder_t* out, sp_str_t content, sp_fmt_arg_t* arg, sp_fmt_arg_t* param) {
  (void)arg; (void)param;
  sp_for(i, content.len) sp_str_builder_append_c8(out, '*');
}

UTEST(sp_fmt_transform, composes_with_wrappers) {
  sp_fmt_directive_reset();
  sp_fmt_register_decorator("wrap", _test_before_lt, _test_after_gt);
  sp_fmt_register_transformer("upper", _test_transform_upper);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.wrap .upper}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "<HI>"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_transform, stacked_innermost_first) {
  sp_fmt_directive_reset();
  sp_fmt_register_transformer("upper", _test_transform_upper);
  sp_fmt_register_transformer("redact", _test_transform_redact);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.upper .redact}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "**"));
  sp_str_t got2 = sp_fmt_a(sp_context_get_allocator(), "{.redact .upper}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got2, "**"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_transform, measures_post_transform_width) {
  sp_fmt_directive_reset();
  sp_fmt_register_transformer("redact", _test_transform_redact);
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{:10 .redact}", sp_fmt_cstr("hi")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "        **"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_transform, into_writer_backed_builder) {
  sp_fmt_directive_reset();
  sp_fmt_register_transformer("upper", _test_transform_upper);

  c8 buf[64] = SP_ZERO_INITIALIZE();
  sp_io_writer_t w = SP_ZERO_INITIALIZE();
  sp_io_writer_from_mem(&w, buf, sizeof(buf));

  sp_fmt_io(&w, "{.upper}", sp_fmt_cstr("hello"));
  sp_str_t got = { .data = buf, .len = 5 };
  EXPECT_TRUE(sp_str_equal_cstr(got, "HELLO"));
  sp_fmt_directive_reset();
}

UTEST(sp_fmt_builtin_transform, upper) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.upper}", sp_fmt_cstr("hello")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "HELLO"));
}

UTEST(sp_fmt_builtin_transform, redact) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.redact}", sp_fmt_cstr("secret")).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "******"));
}

UTEST(fmt, unsigned_integers) {
  // sp_parse_u8
  ASSERT_EQ(sp_parse_u8(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u8(SP_LIT("255")), 255);
  ASSERT_EQ(sp_parse_u8(SP_LIT("128")), 128);
  ASSERT_EQ(sp_parse_u8(SP_LIT("42")), 42);
  // Would assert: "256", "-1", "abc", ""

  // sp_parse_u16
  ASSERT_EQ(sp_parse_u16(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u16(SP_LIT("65535")), 65535);
  ASSERT_EQ(sp_parse_u16(SP_LIT("32768")), 32768);
  ASSERT_EQ(sp_parse_u16(SP_LIT("1234")), 1234);
  // Would assert: "65536", "-1", "text"

  // sp_parse_u32
  ASSERT_EQ(sp_parse_u32(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u32(SP_LIT("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u32(SP_LIT("2147483648")), 2147483648U);
  ASSERT_EQ(sp_parse_u32(SP_LIT("123456789")), 123456789U);
  // Would assert: "4294967296", "-1", "not_a_number"

  // sp_parse_u64
  ASSERT_EQ(sp_parse_u64(SP_LIT("0")), 0ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("18446744073709551615")), 18446744073709551615ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("9223372036854775808")), 9223372036854775808ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("1234567890123")), 1234567890123ULL);
  // Would assert: "18446744073709551616", "-1", "invalid"
}

UTEST(fmt, signed_integers) {
  // sp_parse_s8
  ASSERT_EQ(sp_parse_s8(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s8(SP_LIT("127")), 127);
  ASSERT_EQ(sp_parse_s8(SP_LIT("-128")), -128);
  ASSERT_EQ(sp_parse_s8(SP_LIT("-42")), -42);
  ASSERT_EQ(sp_parse_s8(SP_LIT("42")), 42);
  // Would assert: "128", "-129", "text"

  // sp_parse_s16
  ASSERT_EQ(sp_parse_s16(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s16(SP_LIT("32767")), 32767);
  ASSERT_EQ(sp_parse_s16(SP_LIT("-32768")), -32768);
  ASSERT_EQ(sp_parse_s16(SP_LIT("-1234")), -1234);
  ASSERT_EQ(sp_parse_s16(SP_LIT("1234")), 1234);
  // Would assert: "32768", "-32769", "invalid"

  // sp_parse_s32
  ASSERT_EQ(sp_parse_s32(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s32(SP_LIT("2147483647")), 2147483647);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-2147483648")), INT32_MIN);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-123456789")), -123456789);
  ASSERT_EQ(sp_parse_s32(SP_LIT("123456789")), 123456789);
  // Would assert: "2147483648", "-2147483649", "not_number"

  // sp_parse_s64
  ASSERT_EQ(sp_parse_s64(SP_LIT("0")), 0LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("9223372036854775807")), 9223372036854775807LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("-9223372036854775808")), INT64_MIN);
  ASSERT_EQ(sp_parse_s64(SP_LIT("-1234567890123")), -1234567890123LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("1234567890123")), 1234567890123LL);
  // Would assert: "9223372036854775808", "-9223372036854775809", "abc"
}

UTEST(fmt, floating_point) {
  // sp_parse_f32
  ASSERT_NEAR(sp_parse_f32(SP_LIT("0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("0.0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("3.14159")), 3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-3.14159")), -3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("1.23e2")), 123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("1.23e-2")), 0.0123f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-1.23e2")), -123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("42")), 42.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-42")), -42.0f, 1e-5f);
  // Would assert: "nan", "inf", "text", ""

  // sp_parse_f64 - NOT IMPLEMENTED (SP_UNIMPLEMENTED)
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("0.0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("3.141592653589793")), 3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-3.141592653589793")), -3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("1.23e10")), 1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("1.23e-10")), 1.23e-10, 1e-20);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-1.23e10")), -1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("42.0")), 42.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-42.0")), -42.0, 1e-10);
  // Would assert: "nan", "inf", "invalid", ""
}

UTEST(fmt, hex) {
  ASSERT_EQ(sp_parse_hex(SP_LIT("0")), 0ULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("F")), 0xFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("f")), 0xfULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("FF")), 0xFFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("ff")), 0xffULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("deadbeef")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("123ABC")), 0x123ABCULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("FFFFFFFFFFFFFFFF")), 0xFFFFFFFFFFFFFFFFULL);
  // Would assert: "G", "xyz", "-F", "", "0x" prefix, "0123" octal notation
}

UTEST(fmt, hash) {
  ASSERT_EQ(sp_parse_hash(SP_LIT("0")), 0U);
  ASSERT_EQ(sp_parse_hash(SP_LIT("FFFFFFFF")), 0xFFFFFFFFU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("12345678")), 0x12345678U);
  ASSERT_EQ(sp_parse_hash(SP_LIT("DEADBEEF")), 0xDEADBEEFU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("deadbeef")), 0xdeadbeefU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("ABCD")), 0xABCDU);
  // Would assert: "G", "12345678901", "-1", ""
}

UTEST(fmt, boolean) {
  ASSERT_EQ(sp_parse_bool(SP_LIT("true")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("false")), false);
  ASSERT_EQ(sp_parse_bool(SP_LIT("1")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("0")), false);
  // yes/no, on/off not supported - only true/false and 1/0
  // Would assert: "maybe", "2", "TRUE", "", "yes", "no", "on", "off"
}

UTEST(fmt, characters) {
  // sp_parse_c8 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c8(SP_LIT("'A'")), 'A');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'z'")), 'z');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'0'")), '0');
  ASSERT_EQ(sp_parse_c8(SP_LIT("' '")), ' ');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'!'")), '!');
  // Would assert: "AB", "", "abc", "A" (no quotes)

  // sp_parse_c16 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c16(SP_LIT("'A'")), L'A');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'z'")), L'z');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'0'")), L'0');
  ASSERT_EQ(sp_parse_c16(SP_LIT("' '")), L' ');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'!'")), L'!');
  // Would assert: "AB", "", "abc", "A" (no quotes)
}

UTEST(fmt, extended) {
  u32 u32_val;
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("42"), &u32_val));
  ASSERT_EQ(u32_val, 42U);
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("0"), &u32_val));
  ASSERT_EQ(u32_val, 0U);
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("4294967295"), &u32_val));
  ASSERT_EQ(u32_val, 4294967295U);
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("4294967296"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("-1"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("abc"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT(""), &u32_val));

  // sp_parse_s32_ex
  s32 s32_val;
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("42"), &s32_val));
  ASSERT_EQ(s32_val, 42);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("-42"), &s32_val));
  ASSERT_EQ(s32_val, -42);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("0"), &s32_val));
  ASSERT_EQ(s32_val, 0);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("2147483647"), &s32_val));
  ASSERT_EQ(s32_val, 2147483647);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("-2147483648"), &s32_val));
  ASSERT_EQ(s32_val, INT32_MIN);
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("2147483648"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("-2147483649"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("text"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT(""), &s32_val));

  // sp_parse_f32_ex
  f32 f32_val;
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("3.14"), &f32_val));
  ASSERT_NEAR(f32_val, 3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("-3.14"), &f32_val));
  ASSERT_NEAR(f32_val, -3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("0"), &f32_val));
  ASSERT_NEAR(f32_val, 0.0f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("1.23e2"), &f32_val));
  ASSERT_NEAR(f32_val, 123.0f, 1e-5f);
  ASSERT_FALSE(sp_parse_f32_ex(SP_LIT("abc"), &f32_val));
  ASSERT_FALSE(sp_parse_f32_ex(SP_LIT(""), &f32_val));

  // sp_parse_f64_ex - NOT IMPLEMENTED (SP_UNIMPLEMENTED)
  // f64 f64_val;
  // ASSERT_TRUE(sp_parse_f64_ex(SP_LIT("3.14"), &f64_val));

  // sp_parse_bool_ex
  bool bool_val;
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("true"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("false"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("1"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("0"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_FALSE(sp_parse_bool_ex(SP_LIT("maybe"), &bool_val));
  ASSERT_FALSE(sp_parse_bool_ex(SP_LIT(""), &bool_val));

  // sp_parse_hex_ex
  u64 hex_val;
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("DEADBEEF"), &hex_val));
  ASSERT_EQ(hex_val, 0xDEADBEEFULL);
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("0"), &hex_val));
  ASSERT_EQ(hex_val, 0ULL);
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("FF"), &hex_val));
  ASSERT_EQ(hex_val, 0xFFULL);
  ASSERT_FALSE(sp_parse_hex_ex(SP_LIT("XYZ"), &hex_val));
  ASSERT_FALSE(sp_parse_hex_ex(SP_LIT(""), &hex_val));

  // sp_parse_hash_ex
  sp_hash_t hash_val;
  ASSERT_TRUE(sp_parse_hash_ex(SP_LIT("DEADBEEF"), &hash_val));
  ASSERT_EQ(hash_val, 0xDEADBEEF);
  ASSERT_TRUE(sp_parse_hash_ex(SP_LIT("0"), &hash_val));
  ASSERT_EQ(hash_val, 0);
  ASSERT_FALSE(sp_parse_hash_ex(SP_LIT("GHIJKLMN"), &hash_val));
  ASSERT_FALSE(sp_parse_hash_ex(SP_LIT(""), &hash_val));

  // sp_parse_c8_ex
  c8 c8_val;
  ASSERT_TRUE(sp_parse_c8_ex(SP_LIT("'A'"), &c8_val));
  ASSERT_EQ(c8_val, 'A');
  ASSERT_TRUE(sp_parse_c8_ex(SP_LIT("' '"), &c8_val));
  ASSERT_EQ(c8_val, ' ');
  ASSERT_FALSE(sp_parse_c8_ex(SP_LIT("AB"), &c8_val));
  ASSERT_FALSE(sp_parse_c8_ex(SP_LIT(""), &c8_val));

  // sp_parse_c16_ex
  u16 c16_val;
  ASSERT_TRUE(sp_parse_c16_ex(SP_LIT("'Z'"), &c16_val));
  ASSERT_EQ(c16_val, L'Z');
  ASSERT_TRUE(sp_parse_c16_ex(SP_LIT("'!'"), &c16_val));
  ASSERT_EQ(c16_val, L'!');
  ASSERT_FALSE(sp_parse_c16_ex(SP_LIT("XY"), &c16_val));
  ASSERT_FALSE(sp_parse_c16_ex(SP_LIT(""), &c16_val));

  // Additional extended tests for completeness
  u8 u8_val;
  ASSERT_TRUE(sp_parse_u8_ex(SP_LIT("255"), &u8_val));
  ASSERT_EQ(u8_val, 255);
  ASSERT_FALSE(sp_parse_u8_ex(SP_LIT("256"), &u8_val));

  u16 u16_val;
  ASSERT_TRUE(sp_parse_u16_ex(SP_LIT("65535"), &u16_val));
  ASSERT_EQ(u16_val, 65535);
  ASSERT_FALSE(sp_parse_u16_ex(SP_LIT("65536"), &u16_val));

  u64 u64_val;
  ASSERT_TRUE(sp_parse_u64_ex(SP_LIT("18446744073709551615"), &u64_val));
  ASSERT_EQ(u64_val, 18446744073709551615ULL);
  ASSERT_FALSE(sp_parse_u64_ex(SP_LIT("not_a_number"), &u64_val));

  s8 s8_val;
  ASSERT_TRUE(sp_parse_s8_ex(SP_LIT("-128"), &s8_val));
  ASSERT_EQ(s8_val, -128);
  ASSERT_FALSE(sp_parse_s8_ex(SP_LIT("-129"), &s8_val));

  s16 s16_val;
  ASSERT_TRUE(sp_parse_s16_ex(SP_LIT("32767"), &s16_val));
  ASSERT_EQ(s16_val, 32767);
  ASSERT_FALSE(sp_parse_s16_ex(SP_LIT("32768"), &s16_val));

  s64 s64_val;
  ASSERT_TRUE(sp_parse_s64_ex(SP_LIT("9223372036854775807"), &s64_val));
  ASSERT_EQ(s64_val, 9223372036854775807LL);
  ASSERT_FALSE(sp_parse_s64_ex(SP_LIT("invalid"), &s64_val));
}

UTEST(fmt, parse_edge_cases) {
  ASSERT_EQ(sp_parse_u32(SP_LIT("00042")), 42U);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-00042")), -42);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("003.14")), 3.14f, 1e-5f);

  ASSERT_EQ(sp_parse_s32(SP_LIT("+42")), 42);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("+3.14")), 3.14f, 1e-5f);

  ASSERT_EQ(sp_parse_hex(SP_LIT("DeAdBeEf")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("deadbeef")), 0xdeadbeefULL);

  ASSERT_EQ(sp_parse_u8(SP_LIT("255")), 255);
  ASSERT_EQ(sp_parse_u16(SP_LIT("65535")), 65535);
  ASSERT_EQ(sp_parse_u32(SP_LIT("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u64(SP_LIT("18446744073709551615")), 18446744073709551615ULL);
}

UTEST(fmt, hex_zero) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_uint(0)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0x0"));
}

UTEST(fmt, hex_small) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_uint(0xa)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xA"));
}

UTEST(fmt, hex_no_pad) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_uint(0xa5)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xA5"));
}

UTEST(fmt, hex_word) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_uint(0xdeadbeef)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xDEADBEEF"));
}

UTEST(fmt, hex_u64_max) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_uint(0xffffffffffffffffULL)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xFFFFFFFFFFFFFFFF"));
}

UTEST(fmt, hex_signed_negative) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_int(-1)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0xFFFFFFFFFFFFFFFF"));
}

UTEST(fmt, hex_mixed_digits) {
  sp_str_t got = sp_fmt_a(sp_context_get_allocator(), "{.hex}", sp_fmt_uint(0x1234abcd)).value;
  EXPECT_TRUE(sp_str_equal_cstr(got, "0x1234ABCD"));
}

