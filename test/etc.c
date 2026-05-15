#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"


SP_TEST_MAIN()


/////////////////////
// PARSER TESTS    //
/////////////////////



#define SP_TEST_ENUM(X) \
  X(SP_ENUM_FOO) \
  X(SP_ENUM_BAR) \
  X(SP_ENUM_BAZ) \
  X(SP_ENUM_QUX)

typedef enum {
  SP_TEST_ENUM(SP_X_ENUM_DEFINE)
} sp_test_enum_t;

sp_str_t sp_test_enum_to_str(sp_test_enum_t e) {
  switch (e) {
    SP_TEST_ENUM(SP_X_ENUM_CASE_TO_STRING)
    default: SP_UNREACHABLE_RETURN(sp_str_lit(""));
  }
}

const c8* sp_test_enum_to_cstr(sp_test_enum_t e) {
  switch (e) {
    SP_TEST_ENUM(SP_X_ENUM_CASE_TO_CSTR)
    default: SP_UNREACHABLE_RETURN("");
  }
}

UTEST(sp_enum_macros, name_generation) {
  ASSERT_STREQ(sp_test_enum_to_cstr(SP_ENUM_BAZ), "SP_ENUM_BAZ");
  SP_EXPECT_STR_EQ_CSTR(sp_test_enum_to_str(SP_ENUM_QUX), "SP_ENUM_QUX");
}

UTEST(sp_minmax, basic) {
  ASSERT_EQ(sp_max(1, 2), 2);
  ASSERT_EQ(sp_max(2, 1), 2);
  ASSERT_EQ(sp_max(3, 3), 3);
  ASSERT_EQ(sp_min(1, 2), 1);
  ASSERT_EQ(sp_min(2, 1), 1);
  ASSERT_EQ(sp_min(3, 3), 3);
  ASSERT_EQ(sp_max(-5, -2), -2);
  ASSERT_EQ(sp_min(-5, -2), -5);
}

UTEST(sp_minmax, precedence) {
  ASSERT_EQ(sp_max(1, 2) * 3, 6);
  ASSERT_EQ(1 + sp_min(2, 3), 3);
  ASSERT_EQ(-sp_max(1, 2), -2);
  ASSERT_EQ(sp_max(1, 2) + sp_min(3, 4), 5);
}

UTEST(sp_clamp, basic) {
  ASSERT_EQ(sp_clamp(5, 0, 10), 5);
  ASSERT_EQ(sp_clamp(-1, 0, 10), 0);
  ASSERT_EQ(sp_clamp(11, 0, 10), 10);
  ASSERT_EQ(sp_clamp(0, 0, 10), 0);
  ASSERT_EQ(sp_clamp(10, 0, 10), 10);
  ASSERT_EQ(sp_clamp(-5, -10, -1), -5);
  ASSERT_EQ(sp_clamp(-20, -10, -1), -10);
}

UTEST(sp_clamp, precedence) {
  ASSERT_EQ(sp_clamp(5, 0, 10) * 2, 10);
  ASSERT_EQ(1 + sp_clamp(11, 0, 10), 11);
}
