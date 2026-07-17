#define SP_CLI_IMPLEMENTATION
#include "cli/cli.h"
SP_TEST_MAIN()

#include "cli/lex.c"
#include "cli/parse.c"
#include "cli/complete.c"
#include "cli/dispatch.c"
#include "cli/usage.c"
#include "cli/assign.c"
