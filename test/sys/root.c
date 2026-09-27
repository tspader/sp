#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_root)

UTEST_F(sys_root, out_of_range_returns_invalid) {
  EXPECT_EQ(sp_sys_get_root(1024), SP_SYS_INVALID_FD);
}

UTEST_F(sys_root, root_zero_has_a_path) {
  sp_sys_fd_t root = sp_sys_get_root(0);
  if (root == SP_SYS_INVALID_FD) UTEST_SKIP("no roots");

  c8 buf [SP_PATH_MAX] = sp_zero;
  u64 len = 0;
  EXPECT_EQ(sp_sys_get_fd_path(root, buf, sizeof(buf), &len), SP_OK);
  EXPECT_TRUE(len > 0);
  EXPECT_TRUE(buf[len] == 0);
}
