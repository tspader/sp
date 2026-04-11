// User defines _GNU_SOURCE before any includes under -std=c99.
// Overrides __STRICT_ANSI__, everything visible.
// clang -std=c99 -I. -c -o /dev/null test/tools/features/c99_gnu_source.c
#define _GNU_SOURCE
#include <stdio.h>
#include "main.c"
