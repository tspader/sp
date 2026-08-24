#include "../http.h"

typedef struct {
  const c8* value;
} expect_t;

typedef struct {
  const c8* name;
  const c8* query;
  const c8* key;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "single_pair",
    .query = "token=abc",
    .key = "token",
    .expect = { .value = "abc" },
  },
  {
    .name = "second_pair",
    .query = "a=1&token=abc",
    .key = "token",
    .expect = { .value = "abc" },
  },
  {
    .name = "first_of_duplicates",
    .query = "token=1&token=2",
    .key = "token",
    .expect = { .value = "1" },
  },
  {
    .name = "empty_value",
    .query = "token=&a=1",
    .key = "token",
    .expect = { .value = "" },
  },
  {
    .name = "key_without_equals",
    .query = "token&a=1",
    .key = "token",
    .expect = { .value = "" },
  },
  {
    .name = "missing_key",
    .query = "a=1&b=2",
    .key = "token",
    .expect = { .value = "" },
  },
  {
    .name = "key_prefix_does_not_match",
    .query = "tokens=abc",
    .key = "token",
    .expect = { .value = "" },
  },
  {
    .name = "raw_value_not_decoded",
    .query = "q=a%20b",
    .key = "q",
    .expect = { .value = "a%20b" },
  },
  {
    .name = "empty_query",
    .query = "",
    .key = "token",
    .expect = { .value = "" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_str_t value = sp_http_query_find(sp_cstr_as_str(c->query), sp_cstr_as_str(c->key));
  sp_expect_str_eq_c(t, value, c->expect.value);
  return SP_OK;
}

sp_test_each_fn(http, query, test_t, tests, run);
