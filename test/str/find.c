#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* str;
  const c8* needle;
  s32 expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "prefix",
    .str = "hello world",
    .needle = "hello"
  },
  {
    .name = "suffix",
    .str = "hello world",
    .needle = "world",
    .expect = 6
  },
  {
    .name = "middle",
    .str = "hello world",
    .needle = "lo w",
    .expect = 3
  },
  {
    .name = "last_char",
    .str = "abc",
    .needle = "c",
    .expect = 2
  },
  {
    .name = "first_of_repeated",
    .str = "banana",
    .needle = "ana",
    .expect = 1
  },
  {
    .name = "overlapping",
    .str = "abababab",
    .needle = "abab"
  },
  {
    .name = "partial_prefix_before_match",
    .str = "la_ap_lap",
    .needle = "lap",
    .expect = 6
  },
  {
    .name = "empty_needle",
    .str = "hello",
    .needle = ""
  },
  {
    .name = "both_empty",
    .str = "",
    .needle = ""
  },
  {
    .name = "empty_str",
    .str = "",
    .needle = "hello",
    .expect = SP_STR_NO_MATCH
  },
  {
    .name = "absent",
    .str = "hello",
    .needle = "world",
    .expect = SP_STR_NO_MATCH
  },
  {
    .name = "needle_longer",
    .str = "hi",
    .needle = "hello",
    .expect = SP_STR_NO_MATCH
  },
  {
    .name = "case_sensitive",
    .str = "Hello",
    .needle = "hello",
    .expect = SP_STR_NO_MATCH
  },
};

sp_test_each(str, find, test_t, tests) {
  sp_expect_eq(t, sp_str_find(sp_cstr_as_str(it->str), sp_cstr_as_str(it->needle)), it->expect);
  return SP_OK;
}
