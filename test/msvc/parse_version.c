#include "msvc.h"

typedef struct {
  u32 major;
  u32 minor;
  u32 build;
  u32 revision;
} expect_t;

typedef struct {
  const c8* name;
  const c8* str;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "full",
    .str = "10.0.22621.3235",
    .expect = { .major = 10, .build = 22621, .revision = 3235 },
  },
  {
    .name = "one_part",
    .str = "17",
    .expect = { .major = 17 },
  },
  {
    .name = "empty",
    .str = "",
  },
  {
    .name = "extra_parts",
    .str = "1.2.3.4.5",
    .expect = { .major = 1, .minor = 2, .build = 3, .revision = 4 },
  },
  {
    .name = "garbage_part",
    .str = "10.x.3",
    .expect = { .major = 10, .build = 3 },
  },
};

sp_test_each(msvc, parse_version, test_t, tests) {
  sp_msvc_version_t version = sp_msvc_parse_version(sp_test_arena(t), sp_cstr_as_str(it->str));
  sp_expect_str_eq_c(t, version.str, it->str);
  sp_expect_eq(t, version.major, it->expect.major);
  sp_expect_eq(t, version.minor, it->expect.minor);
  sp_expect_eq(t, version.build, it->expect.build);
  sp_expect_eq(t, version.revision, it->expect.revision);
  return SP_OK;
}
