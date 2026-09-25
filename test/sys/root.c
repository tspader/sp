#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_root)

UTEST_F(sys_root, out_of_range_returns_invalid) {
  SKIP_ON_WASM()
  EXPECT_EQ(sp_sys_get_root(1024), SP_SYS_INVALID_FD);
}

UTEST_F(sys_root, root_zero_is_named_dot) {
  SKIP_ON_WASM()
  c8 buf [8] = sp_zero;
  EXPECT_EQ(sp_sys_get_root_name(0, buf, sizeof(buf)), 1);
  EXPECT_STREQ(buf, ".");
}

UTEST_F(sys_root, out_of_range_has_no_name) {
  SKIP_ON_WASM()
  c8 buf [8] = sp_zero;
  EXPECT_EQ(sp_sys_get_root_name(1024, buf, sizeof(buf)), -1);
}
