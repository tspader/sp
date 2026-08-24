#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_root)

UTEST_F(sys_root, out_of_range_returns_invalid) {
  SKIP_ON_WASM()
  EXPECT_EQ(sp_sys_get_root(1024), SP_SYS_INVALID_FD);
}
