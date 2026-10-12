#include "sp/sp_test.h"

typedef struct {
  const c8* concat;
  const c8* join;
} expect_t;

typedef struct {
  const c8* name;
  const c8* a;
  const c8* b;
  const c8* joiner;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "both",
    .a = "Jerry",
    .b = "Garcia",
    .joiner = " - ",
    .expect = { .concat = "JerryGarcia", .join = "Jerry - Garcia" }
  },
  {
    .name = "empty_joiner",
    .a = "hello",
    .b = "world",
    .joiner = "",
    .expect = { .concat = "helloworld", .join = "helloworld" }
  },
  {
    .name = "empty_b",
    .a = "Jerry",
    .b = "",
    .joiner = "-",
    .expect = { .concat = "Jerry", .join = "Jerry-" }
  },
  {
    .name = "empty_a",
    .a = "",
    .b = "Jerry",
    .joiner = "-",
    .expect = { .concat = "Jerry", .join = "-Jerry" }
  },
  {
    .name = "all_empty",
    .a = "",
    .b = "",
    .joiner = "",
    .expect = { .concat = "", .join = "" }
  },
  {
    .name = "only_joiner",
    .a = "",
    .b = "",
    .joiner = " ",
    .expect = { .concat = "", .join = " " }
  },
};

sp_test_each(str, join, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t a = sp_cstr_as_str(it->a);
  sp_str_t b = sp_cstr_as_str(it->b);
  sp_str_t joiner = sp_cstr_as_str(it->joiner);
  sp_expect_str_eq_c(t, sp_str_concat(mem, a, b), it->expect.concat);
  sp_expect_str_eq_c(t, sp_str_join(mem, a, b, joiner), it->expect.join);
  return SP_OK;
}
