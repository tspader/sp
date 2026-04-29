#define SP_UNIMPLEMENTED() ((void)0)

#define SP_TEST_AMALGAMATION
#include "app.c"
#include "asset.c"
#include "array.c"
#include "cv.c"
#include "elf.c"
#include "env.c"
#include "fmon.c"
#include "format.c"
#include "fs.c"
#include "glob.c"
#include "ht.c"
#include "io.c"
#include "linkage.c"
#include "math.c"
#include "mem.c"
#include "prompt.c"
#include "ps.c"
#include "rb.c"
#include "str.c"
#include "thread.c"
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
