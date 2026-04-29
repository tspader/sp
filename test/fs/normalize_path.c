#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} normalize_path_case_t;

UTEST(fs_normalize_path, cases) {
  sp_mem_t a = sp_mem_os_new();
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
    sp_str_t result = sp_fs_normalize_path_a(a, sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

UTEST(fs_normalize_path, preserves_dotdot) {
  sp_mem_t a = sp_mem_os_new();
  SP_EXPECT_STR_EQ_CSTR(
    sp_fs_normalize_path_a(a, SP_LIT("a\\b\\..\\c")),
    "a/b/../c"
  );
}

UTEST(fs_normalize_path, preserves_dot) {
  sp_mem_t a = sp_mem_os_new();
  SP_EXPECT_STR_EQ_CSTR(
    sp_fs_normalize_path_a(a, SP_LIT("a\\.\\b")),
    "a/./b"
  );
}

UTEST(fs_normalize_path, nonexistent_path) {
  sp_mem_t a = sp_mem_os_new();
  SP_EXPECT_STR_EQ_CSTR(
    sp_fs_normalize_path_a(a, SP_LIT("C:\\no\\such\\path\\file.txt")),
    "C:/no/such/path/file.txt"
  );
}

SP_TEST_MAIN()
