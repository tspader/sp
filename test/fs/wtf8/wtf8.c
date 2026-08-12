#include "sp.h"
#include "sp/sp_test.h"

#define MAX_UNITS 8

typedef struct {
  const c8* name;
  const c8* input;
  sp_err_t err;
  u32 len;
  u16 wtf16 [MAX_UNITS];
} test_t;

static const test_t tests [] = {
  { .name = "empty",                      .input = "" },
  { .name = "ascii_single",               .input = "a",                        .len = 1, .wtf16 = { 0x0061 } },
  { .name = "ascii_run",                  .input = "abc",                      .len = 3, .wtf16 = { 0x0061, 0x0062, 0x0063 } },
  { .name = "slash",                      .input = "/",                        .len = 1, .wtf16 = { 0x002F } },
  { .name = "drive_prefix",               .input = "C:\\",                     .len = 3, .wtf16 = { 0x0043, 0x003A, 0x005C } },
  { .name = "two_byte",                   .input = "\xC3\xA9",                 .len = 1, .wtf16 = { 0x00E9 } },
  { .name = "three_byte",                 .input = "\xE2\x82\xAC",             .len = 1, .wtf16 = { 0x20AC } },
  { .name = "four_byte_surrogate_pair",   .input = "\xF0\x9F\x98\x80",         .len = 2, .wtf16 = { 0xD83D, 0xDE00 } },
  { .name = "unpaired_high_surrogate",    .input = "\xED\xA0\x80",             .len = 1, .wtf16 = { 0xD800 } },
  { .name = "unpaired_low_surrogate",     .input = "\xED\xBF\xBF",             .len = 1, .wtf16 = { 0xDFFF } },
  { .name = "surrogate_pair_via_3_bytes", .input = "\xED\xA0\x80\xED\xB0\x80", .len = 2, .wtf16 = { 0xD800, 0xDC00 } },
  { .name = "mixed_widths",               .input = "a\xC3\xA9\xE2\x82\xACz",   .len = 4, .wtf16 = { 0x0061, 0x00E9, 0x20AC, 0x007A } },
  { .name = "truncated_two_byte",         .input = "\xC0",                     .err = SP_ERR_SYS_INVALID },
  { .name = "truncated_two_byte_lead",    .input = "\xC3",                     .err = SP_ERR_SYS_INVALID },
  { .name = "bad_continuation",           .input = "\xC3\x28",                 .err = SP_ERR_SYS_INVALID },
  { .name = "truncated_three_byte",       .input = "\xE2\x82",                 .err = SP_ERR_SYS_INVALID },
  { .name = "overlong_two_byte",          .input = "\xC0\x80",                 .err = SP_ERR_SYS_INVALID },
  { .name = "overlong_three_byte",        .input = "\xE0\x80\x80",             .err = SP_ERR_SYS_INVALID },
  { .name = "overlong_four_byte",         .input = "\xF0\x80\x80\x80",         .err = SP_ERR_SYS_INVALID },
};

sp_test_each(fs, wtf8, test_t, tests) {
  sp_wide_str_t w;
  sp_must_err_eq(t, sp_wtf8_to_wtf16(sp_test_arena(t), sp_cstr_as_str(it->input), &w), it->err);
  if (it->err || !it->len) {
    sp_expect(t, !w.data);
    sp_expect_eq(t, w.len, 0u);
    return SP_OK;
  }
  sp_must_eq(t, w.len, it->len);
  sp_expect_arr_eq(t, w.data, it->wtf16, it->len);
  sp_expect_eq(t, w.data[w.len], 0);
  return SP_OK;
}
