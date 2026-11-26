#define SP_UNIMPLEMENTED() ((void)0)

#define SP_IMPLEMENTATION
#define SP_TEST_IMPLEMENTATION
#define SP_TEST_AMALGAMATION
#include "app.c"
#include "asset.c"
#include "array.c"
#include "cli.c"
#include "etc.c"
#include "env.c"
#include "fmon.c"
#include "format.c"
#include "glob.c"
#include "ht.c"
#include "io.c"
#include "leak.c"
#include "math.c"
#include "mem.c"
#include "ps.c"
#include "qsort.c"
#include "rb.c"
#include "sys.c"
#include "term.c"
#include "thread.c"
#include "time.c"
#include "process.c"

static s32 entry(s32 num_args, const c8** args) {
  if (num_args > 1 && sp_cstr_equal(args[1], "--fn")) {
    return process_main(num_args, args);
  }
  return utest_main(num_args, args);
}

UTEST_STATE();
SP_MAIN(entry)
