#ifndef FORMAT_TEST_H
#define FORMAT_TEST_H

#include "sp.h"
#include "test.h"
#include "utest.h"

typedef struct {
  const c8* fmt;
  sp_fmt_argv_t args [4];
  const c8* expect;
  sp_err_t err;
  u32 expect_len;
} format_fmt_test_t;

static void run_format_fmt(s32* utest_result, format_fmt_test_t t) {
  sp_str_r result = sp_fmt(sp_mem_get_scratch(), t.fmt, t.args[0], t.args[1], t.args[2], t.args[3]);
  EXPECT_EQ(result.err, t.err);
  if (t.err != SP_OK) return;
  if (t.expect_len) {
    EXPECT_EQ(result.value.len, t.expect_len);
  }
  else {
    SP_EXPECT_STR_EQ_CSTR(result.value, t.expect);
  }
}

#endif // FORMAT_TEST_H
