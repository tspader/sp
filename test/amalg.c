#define SP_TEST_AMALGAMATION
#include "app.c"
#include "context.c"
#include "core.c"
#include "cv.c"
#include "fs.c"
#include "glob.c"
#include "ht.c"
#include "io.c"
#include "leak.c"
#include "linkage.c"
#include "math.c"
#include "ps.c"
#include "rb.c"
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
