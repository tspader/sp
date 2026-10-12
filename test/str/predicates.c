#include "sp/sp_test.h"

typedef struct {
  bool starts;
  bool ends;
  bool contains;
  bool equal;
} expect_t;

typedef struct {
  const c8* name;
  const c8* str;
  const c8* needle;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "prefix",
    .str = "hello world",
    .needle = "hello",
    .expect = { .starts = true, .contains = true }
  },
  {
    .name = "suffix",
    .str = "hello world",
    .needle = "world",
    .expect = { .ends = true, .contains = true }
  },
  {
    .name = "middle",
    .str = "hello world",
    .needle = "lo w",
    .expect = { .contains = true }
  },
  {
    .name = "whole",
    .str = "hello world",
    .needle = "hello world",
    .expect = { .starts = true, .ends = true, .contains = true, .equal = true }
  },
  {
    .name = "absent",
    .str = "hello",
    .needle = "world"
  },
  {
    .name = "needle_longer",
    .str = "hi",
    .needle = "hello"
  },
  {
    .name = "empty_needle",
    .str = "hello",
    .needle = "",
    .expect = { .starts = true, .ends = true, .contains = true }
  },
  {
    .name = "both_empty",
    .str = "",
    .needle = "",
    .expect = { .starts = true, .ends = true, .contains = true, .equal = true }
  },
  {
    .name = "empty_str",
    .str = "",
    .needle = "hello"
  },
  {
    .name = "case_sensitive",
    .str = "Hello",
    .needle = "hello"
  },
  {
    .name = "single_first",
    .str = "hello",
    .needle = "h",
    .expect = { .starts = true, .contains = true }
  },
  {
    .name = "single_last",
    .str = "hello",
    .needle = "o",
    .expect = { .ends = true, .contains = true }
  },
  {
    .name = "single_absent",
    .str = "abc",
    .needle = "z"
  },
  {
    .name = "repeated",
    .str = "banana",
    .needle = "ana",
    .expect = { .ends = true, .contains = true }
  },
  {
    .name = "partial_prefix_before_match",
    .str = "la_ap_lap",
    .needle = "lap",
    .expect = { .ends = true, .contains = true }
  },
  {
    .name = "overlapping",
    .str = "aaaa",
    .needle = "aa",
    .expect = { .starts = true, .ends = true, .contains = true }
  },
};

sp_test_each(str, predicates, test_t, tests) {
  sp_str_t str = sp_cstr_as_str(it->str);
  sp_str_t needle = sp_cstr_as_str(it->needle);
  sp_expect_eq(t, sp_str_starts_with(str, needle), it->expect.starts);
  sp_expect_eq(t, sp_str_ends_with(str, needle), it->expect.ends);
  sp_expect_eq(t, sp_str_contains(str, needle), it->expect.contains);
  sp_expect_eq(t, sp_str_equal(str, needle), it->expect.equal);
  sp_expect_eq(t, sp_str_equal_cstr(str, it->needle), it->expect.equal);
  return SP_OK;
}
