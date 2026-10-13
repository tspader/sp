#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  u32 depth;
} expect_t;

typedef struct {
  const c8* name;
  const c8* path;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "top_level", .path = "A",                  .expect = { .depth = 1 } },
  { .name = "nested",    .path = "A/B",                .expect = { .depth = 2 } },
  { .name = "deep",      .path = "A/B/C/D",            .expect = { .depth = 4 } },
  { .name = "multibyte", .path = "\xc3\xb1/\xc3\xbc",  .expect = { .depth = 2 } },
  { .name = "dotted",    .path = "A.B/C.D",            .expect = { .depth = 2 } },
};

sp_test_each(fs, it_depth, test_t, tests) {
  sp_fs_it_entry_t entry = { .path = sp_cstr_as_str(it->path) };
  sp_expect_eq(t, sp_fs_it_get_entry_depth(entry), it->expect.depth);
  return SP_OK;
}
