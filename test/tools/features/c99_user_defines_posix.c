// User defines _POSIX_C_SOURCE before any includes under -std=c99.
// Header order doesn't matter because visibility is set before any header runs.
// clang -std=c99 -I. -c -o /dev/null test/tools/features/c99_user_defines_posix.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "main.c"
