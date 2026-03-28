#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} get_stem_case_t;

UTEST(fs_get_stem, cases) {
  get_stem_case_t cases[] = {
    { "",                "" },
    { "foo",             "foo" },
    { "foo/",            "foo/" },
    { "foo/bar",         "bar" },
    { "foo/bar.txt",     "bar" },
    { "foo..txt",        "foo." },
    { ".profile",        "" },
    { ".profile.txt",    ".profile" },
    { "C:/foo/bar.txt",  "bar" },
    // Backslash: get_name returns the whole thing, then extension is stripped
    { "C:\\foo\\bar.txt","C:\\foo\\bar" },
    { "foo.bar",         "foo" },
    { "foo.",            "foo" },
    { "foo.bar.baz",     "foo.bar" },
    { "foo.bar.",        "foo.bar" },
    { ".foo",            "" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_get_stem(sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
