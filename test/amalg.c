#define SP_TEST_AMALGAMATION
#include "app.c"
// #include "asset.c"
#include "context.c"
#include "core.c"
#include "cv.c"
// #include "elf.c" // fails on windows because we use linux tools to roundtrip
// #include "fmon.c" // fails on windows because we use linux tools to roundtrip
#include "fs.c"
#include "glob.c"
#include "ht.c"
#include "io.c"
#include "leak.c"
#include "linkage.c"
#include "math.c"
#include "mem.c"
// #include "ps.c"
#include "rb.c"
#include "str.c"
#include "time.c"

#include "utest.h"

#ifndef SP_IMPLEMENTATION
  #define SP_IMPLEMENTATION
#endif
#include "sp.h"

#ifndef SP_TEST_IMPLEMENTATION
  #define SP_TEST_IMPLEMENTATION
#endif
#include "test.h"

UTEST_MAIN()
