#include "sp/sp_test.h"

#define MAX_PARTS 8

typedef struct {
  const c8* name;
  const c8* parts [MAX_PARTS];
  const c8* joiner;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "none",
    .joiner = ", ",
    .expect = ""
  },
  {
    .name = "single",
    .parts = { "apple" },
    .joiner = ", ",
    .expect = "apple"
  },
  {
    .name = "three",
    .parts = { "apple", "banana", "cherry" },
    .joiner = ", ",
    .expect = "apple, banana, cherry"
  },
  {
    .name = "word_joiner",
    .parts = { "jerry", "bobby", "phil" },
    .joiner = " and ",
    .expect = "jerry and bobby and phil"
  },
  {
    .name = "empty_joiner",
    .parts = { "a", "b", "c" },
    .joiner = "",
    .expect = "abc"
  },
  {
    .name = "only_empty",
    .parts = { "" },
    .joiner = ",",
    .expect = ""
  },
  {
    .name = "empty_first",
    .parts = { "", "a" },
    .joiner = ",",
    .expect = ",a"
  },
  {
    .name = "empty_middle",
    .parts = { "a", "", "b" },
    .joiner = ",",
    .expect = "a,,b"
  },
  {
    .name = "empty_last",
    .parts = { "a", "" },
    .joiner = ",",
    .expect = "a,"
  },
};

sp_test_each(str, join_n, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t joiner = sp_cstr_as_str(it->joiner);

  u32 count = 0;
  sp_str_t strs [MAX_PARTS] = sp_zero;
  sp_carr_for(it->parts, i) {
    if (!it->parts[i]) break;
    strs[i] = sp_cstr_as_str(it->parts[i]);
    count++;
  }

  sp_expect_str_eq_c(t, sp_str_join_n(mem, strs, count, joiner), it->expect);
  sp_expect_str_eq_c(t, sp_str_join_cstr_n(mem, it->parts, count, joiner), it->expect);
  return SP_OK;
}
