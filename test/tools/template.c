#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct module {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(module) {
  sp_test_file_manager_init(&ut.file_manager);
}

UTEST_F_TEARDOWN(module) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(module, hello) {
  EXPECT_NE(69, 420);
}
