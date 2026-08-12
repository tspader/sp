#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  bool valid;
} test_t;

static const test_t tests [] = {
  { .name = "empty",              .input = "",             .valid = true },
  { .name = "ascii",              .input = "abc",          .valid = true },
  { .name = "two_byte",           .input = "caf\xC3\xA9",  .valid = true },
  { .name = "unpaired_surrogate", .input = "\xED\xA0\x80", .valid = true },
  { .name = "overlong",           .input = "\xC0\x80",     .valid = false },
  { .name = "truncated",          .input = "\xC3",         .valid = false },
  { .name = "bad_continuation",   .input = "\xC3\x28",     .valid = false },
};

sp_test_each(fs, wtf8_validate, test_t, tests) {
  sp_expect_eq(t, sp_wtf8_validate(sp_cstr_as_str(it->input)), it->valid);
  return SP_OK;
}
