#include "sp/sp_test.h"

#define MAX_CODEPOINTS 8

typedef struct {
  const c8* name;
  const c8* input;
  u32 codepoints [MAX_CODEPOINTS];
} test_t;

static const test_t tests [] = {
  {
    .name = "empty",
    .input = ""
  },
  {
    .name = "ascii",
    .input = "abc",
    .codepoints = { 'a', 'b', 'c' }
  },
  {
    .name = "two_byte",
    .input = "\xC2\xA2",
    .codepoints = { 0xA2 }
  },
  {
    .name = "four_byte",
    .input = "\xF0\x9F\x98\x80",
    .codepoints = { 0x1F600 }
  },
  {
    .name = "mixed_widths",
    .input = "a\xC2\xA2\xE2\x82\xAC\xF0\x9F\x98\x80z",
    .codepoints = { 'a', 0xA2, 0x20AC, 0x1F600, 'z' }
  },
};

sp_test_each(utf8, iterate, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);

  u32 count = 0;
  sp_carr_for(it->codepoints, i) {
    if (!it->codepoints[i]) break;
    count++;
  }

  sp_expect_eq(t, sp_utf8_num_codepoints(input), count);

  u32 n = 0;
  sp_str_for_utf8(input, cp) {
    sp_must_lt(t, n, count);
    sp_expect_eq(t, cp.codepoint, it->codepoints[n]);
    sp_expect_eq(t, cp.codepoint_len, sp_utf8_num_bytes_from_codepoint(cp.codepoint));
    n++;
  }
  sp_expect_eq(t, n, count);

  n = 0;
  sp_str_rfor_utf8(input, cp) {
    sp_must_lt(t, n, count);
    sp_expect_eq(t, cp.codepoint, it->codepoints[count - n - 1]);
    n++;
  }
  sp_expect_eq(t, n, count);

  return SP_OK;
}
