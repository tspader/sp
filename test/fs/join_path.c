#include "fs.h"

typedef struct {
  const c8* a;
  const c8* b;
  const c8* expected;
} join_path_case_t;

struct fs_join_path {
  sp_mem_heap_t* heap;
  sp_mem_t mem;
};

UTEST_F_SETUP(fs_join_path) {
  ut.heap = sp_mem_heap_new();
  ut.mem = sp_mem_heap_as_allocator(ut.heap);
}

UTEST_F_TEARDOWN(fs_join_path) {
  sp_mem_heap_destroy(ut.heap);
}

UTEST_F(fs_join_path, cases) {
  SKIP_ON_WASM()
  join_path_case_t cases[] = {
    { "foo",  "bar",     "foo/bar" },
    { "foo/", "bar",     "foo/bar" },
    { "foo/", "bar/",    "foo/bar" },
    { "foo",  "bar/baz", "foo/bar/baz" },
    { "/",    "bar",     "/bar" },
    { "/",    "bar/baz", "/bar/baz" },
    { "C:/",  "bar",     "C:/bar" },
    { "C:",   "bar",     "C:/bar" },
    { "",     "bar",     "bar" },
    { "foo",  "",        "foo" },
    { "",     "",        "" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_join_path(ut.mem, sp_str_view(cases[i].a), sp_str_view(cases[i].b));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}
