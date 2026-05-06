#ifndef SP_TEST_AMALGAMATION
  #define SP_TEST_AMALGAMATION
  #define MEM_TEST_OWNS_MAIN
#endif

#include "mem/builtin.c"
#include "mem/arena.c"
#include "mem/fixed.c"
#include "mem/slice.c"

#ifdef MEM_TEST_OWNS_MAIN
UTEST_MAIN()
#endif
