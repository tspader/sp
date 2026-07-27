#define SP_IMPLEMENTATION
#include "sp/sp_test.h"

// #include "sp_test/filter.c"
// #include "sp_test/resolve.c"
// #include "sys/open.c"
// #include "sys/iter.c"
//
static s32 entry(s32 argc, const c8** argv) {
  return sp_test_main(argc, argv, SP_NULLPTR);
}

SP_MAIN(entry)
