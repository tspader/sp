#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_root)

UTEST_F(sys_root, out_of_range_returns_invalid) {
  EXPECT_EQ(sp_sys_get_root(1024), SP_SYS_INVALID_FD);
}

UTEST_F(sys_root, out_of_range_has_no_name) {
  c8 buf [8] = sp_zero;
  u64 len = 0;
  EXPECT_EQ(sp_sys_get_root_name(1024, buf, sizeof(buf), &len), SP_ERR_SYS_BAD_FD);
}

UTEST_F(sys_root, root_zero_has_no_name) {
  SKIP_ON_WASM()
  c8 buf [8] = sp_zero;
  u64 len = 0;
  EXPECT_EQ(sp_sys_get_root_name(0, buf, sizeof(buf), &len), SP_ERR_SYS_UNSUPPORTED);
}

UTEST_F(sys_root, every_root_is_named_or_unsupported) {
  c8 buf [SP_PATH_MAX];
  sp_for(it, 1024) {
    if (sp_sys_get_root(it) == SP_SYS_INVALID_FD) break;
    u64 len = 0;
    sp_err_t err = sp_sys_get_root_name(it, buf, sizeof(buf), &len);
    EXPECT_TRUE(err == SP_OK || err == SP_ERR_SYS_UNSUPPORTED);
    if (err == SP_OK) EXPECT_TRUE(buf[len] == 0);
  }
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
