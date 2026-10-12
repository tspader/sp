#include "sp/sp_test.h"

#define MAX_STRS 4

typedef struct {
  const c8* name;
  const c8* input [MAX_STRS];
  const c8* expect [MAX_STRS];
} test_t;

static const test_t tests [] = {
  {
    .name = "none"
  },
  {
    .name = "single",
    .input = { "hello" },
    .expect = { "hello" }
  },
  {
    .name = "ascending",
    .input = { "hi", "hello", "world!" },
    .expect = { "hi    ", "hello ", "world!" }
  },
  {
    .name = "all_same",
    .input = { "aaa", "bbb", "ccc" },
    .expect = { "aaa", "bbb", "ccc" }
  },
  {
    .name = "empties",
    .input = { "", "hello", "" },
    .expect = { "     ", "hello", "     " }
  },
};

sp_test_each(str, pad_to_longest, test_t, tests) {
  u32 count = 0;
  sp_str_t strs [MAX_STRS] = sp_zero;
  sp_carr_for(it->input, i) {
    if (!it->input[i]) break;
    strs[i] = sp_cstr_as_str(it->input[i]);
    count++;
  }

  sp_da(sp_str_t) padded = sp_str_pad_to_longest(sp_test_arena(t), strs, count);
  sp_must_eq(t, sp_da_size(padded), count);
  sp_expect_strs_eq(t, padded, count, it->expect);
  return SP_OK;
}
