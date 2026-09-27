#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_root)

UTEST_F(sys_root, out_of_range_returns_invalid) {
  EXPECT_EQ(sp_sys_get_root(1024), SP_SYS_INVALID_FD);
}

UTEST_F(sys_root, out_of_range_has_no_label) {
  c8 buf [SP_PATH_MAX] = sp_zero;
  u64 len = 1;
  EXPECT_EQ(sp_sys_get_root_label(1024, buf, sizeof(buf), &len), SP_ERR_SYS_BAD_FD);
  EXPECT_EQ(len, 0);
}

UTEST_F(sys_root, every_root_has_a_label) {
  for (s32 it = 0; sp_sys_get_root(it) != SP_SYS_INVALID_FD; it++) {
    c8 buf [SP_PATH_MAX];
    sp_for(at, sizeof(buf)) buf[at] = (c8)0xAB;
    u64 len = 0;
    EXPECT_EQ(sp_sys_get_root_label(it, buf, sizeof(buf), &len), SP_OK);
    EXPECT_EQ(buf[len], 0);
  }
}

UTEST_F(sys_root, native_root_serves_everything) {
  SKIP_ON_WASM()

  c8 buf [SP_PATH_MAX] = sp_zero;
  u64 len = 1;
  EXPECT_EQ(sp_sys_get_root_label(0, buf, sizeof(buf), &len), SP_OK);
  EXPECT_EQ(len, 0);
}

UTEST_F(sys_root, label_needs_room_for_nul) {
  if (sp_sys_get_root(0) == SP_SYS_INVALID_FD) UTEST_SKIP("no roots");

  c8 buf [SP_PATH_MAX] = sp_zero;
  u64 len = 0;
  EXPECT_EQ(sp_sys_get_root_label(0, buf, sizeof(buf), &len), SP_OK);

  u64 n = 1;
  EXPECT_EQ(sp_sys_get_root_label(0, buf, len, &n), SP_ERR_SYS_NAME_TOO_LONG);
  EXPECT_EQ(n, 0);
}
