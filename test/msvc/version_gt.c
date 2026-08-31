#include "msvc.h"

typedef struct {
  bool forward;
  bool reverse;
} expect_t;

typedef struct {
  const c8* name;
  sp_msvc_version_t a;
  sp_msvc_version_t b;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "equal",
    .a = { .major = 1, .minor = 2, .build = 3, .revision = 4 },
    .b = { .major = 1, .minor = 2, .build = 3, .revision = 4 },
  },
  {
    .name = "major",
    .a = { .major = 2 },
    .b = { .major = 1, .minor = 9, .build = 9, .revision = 9 },
    .expect = { .forward = true },
  },
  {
    .name = "minor",
    .a = { .major = 1, .minor = 1 },
    .b = { .major = 1, .build = 9, .revision = 9 },
    .expect = { .forward = true },
  },
  {
    .name = "build",
    .a = { .major = 1, .build = 2 },
    .b = { .major = 1, .build = 1, .revision = 9 },
    .expect = { .forward = true },
  },
  {
    .name = "revision",
    .a = { .major = 1, .revision = 2 },
    .b = { .major = 1, .revision = 1 },
    .expect = { .forward = true },
  },
};

sp_test_each(msvc, version_gt, test_t, tests) {
  sp_expect_eq(t, sp_msvc_version_gt(it->a, it->b), it->expect.forward);
  sp_expect_eq(t, sp_msvc_version_gt(it->b, it->a), it->expect.reverse);
  return SP_OK;
}
