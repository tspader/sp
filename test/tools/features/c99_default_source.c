// User defines _DEFAULT_SOURCE before any includes under -std=c99.
// clang -std=c99 -I. -c -o /dev/null test/tools/features/c99_default_source.c
#define _DEFAULT_SOURCE
#include <stdio.h>
#include "main.c"
