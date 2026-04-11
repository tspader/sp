// System header before sp.h under -std=c99.
// glibc locks in strict visibility, sp.h feature macros arrive too late.
// clang -std=c99 -I. -c -o /dev/null test/tools/features/c99_header_before.c
#include <stdio.h>
#include "main.c"
