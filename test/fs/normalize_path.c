#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} normalize_path_case_t;

struct fs_normalize_path {
  sp_mem_heap_t* heap;
  sp_mem_t mem;
};

UTEST_F_SETUP(fs_normalize_path) {
  ut.heap = sp_mem_heap_new();
  ut.mem = sp_mem_heap_as_allocator(ut.heap);
}

UTEST_F_TEARDOWN(fs_normalize_path) {
  sp_mem_heap_destroy(ut.heap);
}

UTEST_F(fs_normalize_path, cases) {
  SKIP_ON_WASM()
  normalize_path_case_t cases[] = {
    { "",                              "" },
    { "foo",                           "foo" },
    { "foo/",                          "foo" },
    { "foo/bar",                       "foo/bar" },
    { "foo/bar.txt",                   "foo/bar.txt" },
    { ".profile",                      ".profile" },
    { "C:/foo/bar.txt",                "C:/foo/bar.txt" },
    { "C:\\foo\\bar.txt",              "C:/foo/bar.txt" },
    { "C:/Users\\Test/sub\\file.txt",  "C:/Users/Test/sub/file.txt" },
    { "C:\\Users\\Test\\",             "C:/Users/Test" },
    { "foo\\bar",                      "foo/bar" },
    { "\\",                            "" },
    { "/",                             "" },
    { "foo\\",                         "foo" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_normalize_path(ut.mem, sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

UTEST_F(fs_normalize_path, preserves_dotdot) {
  SKIP_ON_WASM()
  SP_EXPECT_STR_EQ_CSTR(
    sp_fs_normalize_path(ut.mem, sp_str_lit("a\\b\\..\\c")),
    "a/b/../c"
  );
}

UTEST_F(fs_normalize_path, preserves_dot) {
  SKIP_ON_WASM()
  SP_EXPECT_STR_EQ_CSTR(
    sp_fs_normalize_path(ut.mem, sp_str_lit("a\\.\\b")),
    "a/./b"
  );
}

UTEST_F(fs_normalize_path, nonexistent_path) {
  SKIP_ON_WASM()
  SP_EXPECT_STR_EQ_CSTR(
    sp_fs_normalize_path(ut.mem, sp_str_lit("C:\\no\\such\\path\\file.txt")),
    "C:/no/such/path/file.txt"
  );
}
