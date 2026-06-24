#include "format.h"

typedef struct {
  const c8* str;
  sp_err_t err;
  sp_fmt_spec_t spec;
  const c8* names [SP_FMT_MAX_DIRECTIVES];
} format_spec_test_t;

static sp_err_t format_parse_spec(const c8* str, sp_fmt_spec_t* spec) {
  sp_fmt_parser_t parser = { .str = sp_str_view(str), .i = 0 };
  return sp_fmt_parse_specifier(&parser, spec);
}

static void run_format_spec(s32* utest_result, format_spec_test_t t) {
  sp_fmt_spec_t spec = sp_zero;
  sp_err_t err = format_parse_spec(t.str, &spec);
  EXPECT_EQ(err, t.err);
  if (err != SP_OK) return;

  EXPECT_EQ(spec.align, t.spec.align);
  EXPECT_EQ(spec.fill, t.spec.fill);
  EXPECT_EQ(spec.precision.some, t.spec.precision.some);
  EXPECT_EQ(spec.precision.value, t.spec.precision.value);
  EXPECT_EQ(spec.width, t.spec.width);
  EXPECT_EQ(spec.renderer, t.spec.renderer);

  u32 num = 0;
  while (num < SP_FMT_MAX_DIRECTIVES && (t.names[num] || (t.spec.dynamic & sp_fmt_dynamic_directive(num)))) num++;
  EXPECT_EQ(spec.directive.num, num);
  EXPECT_EQ(spec.dynamic, t.spec.dynamic);

  sp_for(i, spec.directive.num) {
    if (t.names[i]) {
      EXPECT_TRUE(sp_str_equal_cstr(spec.directive.names[i], t.names[i]));
    }
  }
}

UTEST(format_spec, fields) {
  format_spec_test_t cases[] = {
    { .str = "{:*^9}", .spec = { .width = 9, .align = SP_FMT_ALIGN_CENTER, .fill = '*' } },
    { .str = "{:.5}", .spec = { .precision = sp_opt_some(5) } },
    { .str = "{:10.3}", .spec = { .width = 10, .precision = sp_opt_some(3) } },
    { .str = "{:<7}", .spec = { .width = 7, .align = SP_FMT_ALIGN_LEFT } },
    { .str = "{:>4}", .spec = { .width = 4, .align = SP_FMT_ALIGN_RIGHT } },
    { .str = "{:-<8}", .spec = { .width = 8, .align = SP_FMT_ALIGN_LEFT, .fill = '-' } },
    { .str = "{:*^12.4}", .spec = { .width = 12, .align = SP_FMT_ALIGN_CENTER, .fill = '*', .precision = sp_opt_some(4) } },
    { .str = "{:05}", .spec = { .width = 5 } },
    { .str = "{:$}", .spec = { .dynamic = SP_FMT_DYNAMIC_WIDTH } },
    { .str = "{:.$}", .spec = { .dynamic = SP_FMT_DYNAMIC_PRECISION } },
    { .str = "{:$^9}", .spec = { .width = 9, .align = SP_FMT_ALIGN_CENTER, .dynamic = SP_FMT_DYNAMIC_FILL } },
    { .str = "{:$^$}", .spec = { .align = SP_FMT_ALIGN_CENTER, .dynamic = SP_FMT_DYNAMIC_FILL | SP_FMT_DYNAMIC_WIDTH } },
    { .str = "{:$<$.$}", .spec = { .align = SP_FMT_ALIGN_LEFT, .dynamic = SP_FMT_DYNAMIC_FILL | SP_FMT_DYNAMIC_WIDTH | SP_FMT_DYNAMIC_PRECISION } },
    { .str = "{:$.4}", .spec = { .precision = sp_opt_some(4), .dynamic = SP_FMT_DYNAMIC_WIDTH } },
    { .str = "{}" },
    { .str = "{:}" },
    { .str = "{:B}", .spec = { .renderer = 'B' } },
    { .str = "{:9B}", .spec = { .renderer = 'B', .width = 9 } },
    { .str = "{:x}", .spec = { .renderer = 'x' } },
    { .str = "{:X}", .spec = { .renderer = 'X' } },
    { .str = "{:b}", .spec = { .renderer = 'b' } },
    { .str = "{:o}", .spec = { .renderer = 'o' } },
    { .str = "{:c}", .spec = { .renderer = 'c' } },
    { .str = "{:0>8x}", .spec = { .renderer = 'x', .width = 8, .align = SP_FMT_ALIGN_RIGHT, .fill = '0' } },
    { .str = "{:x .red}", .spec = { .renderer = 'x' }, .names = { "red" } },
    { .str = "{:>6x .red}", .spec = { .renderer = 'x', .width = 6, .align = SP_FMT_ALIGN_RIGHT }, .names = { "red" } },
    { .str = "{.red}", .names = { "red" } },
    { .str = "{:10 .red}", .spec = { .width = 10 }, .names = { "red" } },
    {
      .str = "{:*^9.2 .red}",
      .spec = { .width = 9, .align = SP_FMT_ALIGN_CENTER, .fill = '*', .precision = sp_opt_some(2) },
      .names = { "red" },
    },
    { .str = "{.$}", .spec = { .dynamic = sp_fmt_dynamic_directive(0) } },
    { .str = "{.red .$}", .spec = { .dynamic = sp_fmt_dynamic_directive(1) }, .names = { "red" } },
    { .str = "{.$ .red}", .spec = { .dynamic = sp_fmt_dynamic_directive(0) }, .names = { SP_NULLPTR, "red" } },
    { .str = "{:>6x .$}", .spec = { .renderer = 'x', .width = 6, .align = SP_FMT_ALIGN_RIGHT, .dynamic = sp_fmt_dynamic_directive(0) } },
    { .str = "{.$x}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = "{.$ .$}", .spec = { .dynamic = sp_fmt_dynamic_directive(0) | sp_fmt_dynamic_directive(1) } },
    { .str = "{.red .bold .italic}", .names = { "red", "bold", "italic" } },
    { .str = "{.a .b .c .d .e .f .g .h}", .names = { "a", "b", "c", "d", "e", "f", "g", "h" } },
    { .str = "{.da-sh}", .names = { "da-sh" } },
    { .str = "{.under_score}", .names = { "under_score" } },
    { .str = "{: .red}", .names = { "red" } },
    { .str = "{:5$}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = ":5}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = "{:5", .err = SP_ERR_FMT_UNTERMINATED_PLACEHOLDER },
    { .str = "{:5.}", .err = SP_ERR_FMT_BAD_PRECISION },
    { .str = "{.a .b .c .d .e .f .g .h .i}", .err = SP_ERR_FMT_TOO_MANY_DIRECTIVES },
    { .str = "{:10 .}", .err = SP_ERR_FMT_BAD_DIRECTIVE },
    { .str = "{:10 .red!}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = "{.red.bold}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = "{:10 red}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = "{: .4red}", .err = SP_ERR_FMT_BAD_DIRECTIVE },
    { .str = "{.-red}", .err = SP_ERR_FMT_BAD_DIRECTIVE },
    { .str = "{:5red}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
    { .str = "{.red junk}", .err = SP_ERR_FMT_BAD_PLACEHOLDER },
  };

  sp_carr_for(cases, it) {
    run_format_spec(utest_result, cases[it]);
  }
}
