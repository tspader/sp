#include "sp.h"

extern void* memcpy(void* restrict dest, const void* restrict src, size_t n);
extern void* memmove(void* dest, const void* src, size_t n);
extern void* memset(void* dest, int c, size_t n);
extern int memcmp(const void* a, const void* b, size_t n);
extern size_t strlen(const char* s);

static void print(const char* s) {
  sp_sys_write(2, s, strlen(s));
}

static void print_ok(const char* name) {
  print("  [OK] ");
  print(name);
  print("\n");
}

static void print_fail(const char* name) {
  print("  [FAIL] ");
  print(name);
  print("\n");
}

#define TEST(name, cond) do { \
  if (cond) { print_ok(name); } \
  else { print_fail(name); failures++; } \
} while(0)

void _start(void) {
  int failures = 0;

  print("=== SP_BUILTIN test ===\n");

  print("\n--- memcpy ---\n");
  {
    char src[64] = "hello world";
    char dst[64] = {0};
    memcpy(dst, src, 12);
    TEST("memcpy basic", memcmp(dst, src, 12) == 0);

    memcpy(dst, src, 0);
    TEST("memcpy n=0", 1);

    char big_src[256];
    char big_dst[256];
    for (int i = 0; i < 256; i++) big_src[i] = (char)i;
    memcpy(big_dst, big_src, 256);
    TEST("memcpy 256 bytes", memcmp(big_dst, big_src, 256) == 0);
  }

  print("\n--- memset ---\n");
  {
    char buf[64];
    memset(buf, 'x', 16);
    int ok = 1;
    for (int i = 0; i < 16; i++) if (buf[i] != 'x') ok = 0;
    TEST("memset", ok);

    memset(buf, 0xAB, 64);
    ok = 1;
    for (int i = 0; i < 64; i++) if ((unsigned char)buf[i] != 0xAB) ok = 0;
    TEST("memset 64 bytes", ok);
  }

  print("\n--- memcmp ---\n");
  {
    char a[] = "abc";
    char b[] = "abc";
    char c[] = "abd";
    TEST("memcmp equal", memcmp(a, b, 3) == 0);
    TEST("memcmp less", memcmp(a, c, 3) < 0);
    TEST("memcmp greater", memcmp(c, a, 3) > 0);
    TEST("memcmp n=0", memcmp("abc", "xyz", 0) == 0);
  }

  print("\n--- memmove ---\n");
  {
    char buf[32] = "0123456789";
    memmove(buf + 2, buf, 8);
    TEST("memmove fwd overlap", memcmp(buf, "0101234567", 10) == 0);

    char buf2[32] = "0123456789";
    memmove(buf2, buf2 + 2, 8);
    TEST("memmove bwd overlap", memcmp(buf2, "23456789", 8) == 0);

    char buf3[32] = "hello";
    memmove(buf3, buf3, 5);
    TEST("memmove same ptr", memcmp(buf3, "hello", 5) == 0);
  }

  print("\n--- strlen ---\n");
  {
    TEST("strlen empty", strlen("") == 0);
    TEST("strlen hello", strlen("hello") == 5);
    TEST("strlen longer", strlen("hello world!") == 12);
  }

  print("\n=== done ===\n");
  if (failures == 0) {
    print("All tests passed!\n");
  } else {
    print("FAILURES detected\n");
  }

  sp_sys_exit(failures);
}
