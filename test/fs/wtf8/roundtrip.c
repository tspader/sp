#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
} test_t;

static const test_t tests [] = {
  { .name = "ascii_path",              .input = "hello/world.txt" },
  { .name = "mixed_widths",            .input = "caf\xC3\xA9/\xE2\x82\xAC" },
  { .name = "four_byte",               .input = "\xF0\x9F\x98\x80" },
  { .name = "unpaired_high_surrogate", .input = "\xED\xA0\x80" },
  { .name = "unpaired_low_surrogate",  .input = "\xED\xBF\xBF" },
};

sp_test_each(fs, wtf8_roundtrip, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_wide_str_t w;
  sp_must_ok(t, sp_wtf8_to_wtf16(mem, input, &w));
  sp_str_t back;
  sp_must_ok(t, sp_wtf16_to_wtf8(mem, w, &back));
  sp_expect_str_eq(t, back, input);
  return SP_OK;
}
