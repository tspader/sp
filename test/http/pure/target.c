#include "../http.h"

typedef struct {
  const c8* path;
  const c8* query;
} expect_t;

typedef struct {
  const c8* name;
  const c8* target;
  expect_t  expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "path_only",
    .target = "/act",
    .expect = { .path = "/act", .query = "" },
  },
  {
    .name = "path_and_query",
    .target = "/events?token=abc",
    .expect = { .path = "/events", .query = "token=abc" },
  },
  {
    .name = "empty_query",
    .target = "/events?",
    .expect = { .path = "/events", .query = "" },
  },
  {
    .name = "second_question_mark_stays_in_query",
    .target = "/a?b=c?d",
    .expect = { .path = "/a", .query = "b=c?d" },
  },
  {
    .name = "root",
    .target = "/",
    .expect = { .path = "/", .query = "" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_http_target_t target = sp_http_target_split(sp_cstr_as_str(c->target));
  sp_expect_str_eq_c(t, target.path, c->expect.path);
  sp_expect_str_eq_c(t, target.query, c->expect.query);
  return SP_OK;
}

sp_test_each_fn(http, target, test_t, tests, run);
