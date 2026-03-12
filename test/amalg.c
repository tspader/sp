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
#include "rb.c"
#include "time.c"

#include "utest.h"

#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

UTEST_MAIN()
