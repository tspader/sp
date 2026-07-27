#define SP_IMPLEMENTATION
#include "sp/sp_test.h"

static s32 entry(s32 argc, const c8** argv) {
  return sp_test_main(argc, argv, SP_NULLPTR);
}

SP_MAIN(entry)
