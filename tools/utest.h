/*
   The latest version of this library is available on GitHub;
   https://github.com/sheredom/utest.h

   This version has been modified to depend only on sp.h.
*/

/*
   This is free and unencumbered software released into the public domain.

   Anyone is free to copy, modify, publish, use, compile, sell, or
   distribute this software, either in source code form or as a compiled
   binary, for any purpose, commercial or non-commercial, and by any
   means.

   In jurisdictions that recognize copyright laws, the author or authors
   of this software dedicate any and all copyright interest in the
   software to the public domain. We make this dedication for the benefit
   of the public at large and to the detriment of our heirs and
   successors. We intend this dedication to be an overt act of
   relinquishment in perpetuity of all present and future rights to this
   software under copyright law.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   For more information, please refer to <http://unlicense.org/>
*/

#ifndef SHEREDOM_UTEST_H_INCLUDED
#define SHEREDOM_UTEST_H_INCLUDED

#include "sp.h"

#define UTEST_TEST_PASSED (0)
#define UTEST_TEST_FAILURE (1)
#define UTEST_TEST_SKIPPED (2)

#if defined(__TINYC__)
#define UTEST_ATTRIBUTE(a) __attribute((a))
#elif defined(_MSC_VER)
#define UTEST_ATTRIBUTE(a)
#else
#define UTEST_ATTRIBUTE(a) __attribute__((a))
#endif

#if defined(_MSC_VER)
#define UTEST_WEAK __forceinline
#define UTEST_UNUSED
#define UTEST_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__) || defined(__TINYC__)
#define UTEST_WEAK UTEST_ATTRIBUTE(weak)
#define UTEST_UNUSED UTEST_ATTRIBUTE(unused)
#define UTEST_INLINE inline
#else
#define UTEST_WEAK
#define UTEST_UNUSED
#define UTEST_INLINE inline
#endif

#define UTEST_CAST(type, x) ((type)(x))
#define UTEST_PTR_CAST(type, x) ((type)(x))
#define UTEST_EXTERN extern
#define UTEST_NULL SP_NULLPTR

#if defined(_MSC_VER)

#if defined(_WIN64)
#define UTEST_SYMBOL_PREFIX
#else
#define UTEST_SYMBOL_PREFIX "_"
#endif

#pragma section(".CRT$XCU", read)
#define UTEST_INITIALIZER(f)                                                   \
  static void __cdecl f(void);                                                 \
  __pragma(comment(linker, "/include:" UTEST_SYMBOL_PREFIX #f "_"))            \
      __declspec(allocate(".CRT$XCU")) void(__cdecl * f##_)(void) = f;         \
  static void __cdecl f(void)

#else

#define UTEST_INITIALIZER(f)                                                   \
  static void f(void) UTEST_ATTRIBUTE(constructor);                            \
  static void f(void)

#endif

static UTEST_INLINE void *utest_realloc(void *const pointer, u64 new_size) {
  void *const new_pointer = sp_mem_os_realloc(pointer, new_size);
  if (UTEST_NULL == new_pointer) {
    sp_mem_os_free(pointer);
  }
  return new_pointer;
}

static UTEST_INLINE s64 utest_ns(void) {
  return UTEST_CAST(s64, sp_tm_now_point());
}

typedef void (*utest_testcase_t)(int *, u64);

struct utest_test_state_s {
  utest_testcase_t func;
  u64 index;
  c8* name;
  c8* set;
  c8* test;
};

struct utest_state_s {
  struct utest_test_state_s *tests;
  u64 tests_length;
  sp_io_writer_t output;
  int has_output;
};

/* extern to the global state utest needs to execute */
UTEST_EXTERN struct utest_state_s utest_state;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvariadic-macros"
#endif
#define UTEST_PRINTF(...)                                                      \
  do {                                                                         \
    sp_str_t _utest_fmtd = sp_fmt_a(sp_context_get_allocator(), __VA_ARGS__).value;                               \
    sp_os_print(_utest_fmtd);                                                 \
    if (utest_state.has_output) {                                              \
      sp_io_write_str(&utest_state.output, _utest_fmtd, SP_NULLPTR);         \
    }                                                                          \
  } while (0)
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if defined(__clang__)
#define UTEST_OVERLOADABLE UTEST_ATTRIBUTE(overloadable)
#endif

#if defined(UTEST_OVERLOADABLE)

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(signed char c);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(signed char c) {
  UTEST_PRINTF("{}", sp_fmt_int(UTEST_CAST(s32, c)));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned char c);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned char c) {
  UTEST_PRINTF("{.hex}", sp_fmt_uint(c));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(float f);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(float f) {
  UTEST_PRINTF("{:.3}", sp_fmt_float(f));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(double d);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(double d) {
  UTEST_PRINTF("{:.3}", sp_fmt_float(d));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long double d);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long double d) {
  UTEST_PRINTF("{:.3}", sp_fmt_float(UTEST_CAST(f64, d)));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(int i) {
  UTEST_PRINTF("{}", sp_fmt_int(UTEST_CAST(s32, i)));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned int i) {
  UTEST_PRINTF("{}", sp_fmt_uint(UTEST_CAST(u32, i)));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long int i) {
  UTEST_PRINTF("{}", sp_fmt_int(UTEST_CAST(s64, i)));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long unsigned int i) {
  UTEST_PRINTF("{}", sp_fmt_uint(UTEST_CAST(u64, i)));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(const void *p);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(const void *p) {
  UTEST_PRINTF("{}", sp_fmt_ptr((void *)p));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long int i) {
  UTEST_PRINTF("{}", sp_fmt_int(UTEST_CAST(s64, i)));
}

UTEST_WEAK UTEST_OVERLOADABLE void
utest_type_printer(long long unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void
utest_type_printer(long long unsigned int i) {
  UTEST_PRINTF("{}", sp_fmt_uint(UTEST_CAST(u64, i)));
}

#else
#define utest_type_printer(...) UTEST_PRINTF("undef")
#endif

#define UTEST_SURPRESS_WARNING_BEGIN
#define UTEST_SURPRESS_WARNING_END

#if defined(__clang__)
/* clang-format off */
#define UTEST_AUTO(x)                                                          \
  _Pragma("clang diagnostic push")                                             \
      _Pragma("clang diagnostic ignored \"-Wgnu-auto-type\"") __auto_type      \
          _Pragma("clang diagnostic pop")
/* clang-format on */
#else
#define UTEST_AUTO(x) __typeof__(x + 0)
#endif

static UTEST_INLINE int utest_strncmp(const c8 *a, const c8 *b, u32 n) {
  for (u32 i = 0; i < n; i++) {
    if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
    if (a[i] == '\0') return 0;
  }
  return 0;
}

#define UTEST_STRNCMP(x, y, size) utest_strncmp(x, y, size)

#define UTEST_SKIP(msg)                                                        \
  do {                                                                         \
    *utest_result = UTEST_TEST_SKIPPED;                                        \
    return;                                                                    \
  } while (0)

#if defined(__clang__)
#define UTEST_COND(x, y, cond, msg, is_assert)                                 \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    _Pragma("clang diagnostic push")                                           \
        _Pragma("clang diagnostic ignored \"-Wlanguage-extension-token\"")     \
            _Pragma("clang diagnostic ignored \"-Wfloat-equal\"")              \
                UTEST_AUTO(x) xEval = (x);                                     \
    UTEST_AUTO(y) yEval = (y);                                                 \
    if (!((xEval)cond(yEval))) {                                               \
      const c8 *const xAsString = #x;                                         \
      const c8 *const yAsString = #y;                                         \
      _Pragma("clang diagnostic pop")                                          \
          UTEST_PRINTF("{}:{}: Failure\n",                                     \
                       sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));           \
      UTEST_PRINTF("  expected -> {.gray} {} {.gray}\n",                              \
                   sp_fmt_cstr(xAsString), sp_fmt_cstr(#cond),                \
                   sp_fmt_cstr(yAsString));                                    \
      UTEST_PRINTF("    actual -> ");                                           \
      utest_type_printer(xEval);                                               \
      UTEST_PRINTF(" vs ");                                                    \
      utest_type_printer(yEval);                                               \
      UTEST_PRINTF("\n");                                                      \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END
#elif defined(__GNUC__)
#define UTEST_COND(x, y, cond, msg, is_assert)                                 \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    UTEST_AUTO(x) xEval = (x);                                                 \
    UTEST_AUTO(y) yEval = (y);                                                 \
    if (!((xEval)cond(yEval))) {                                               \
      const c8 *const xAsString = #x;                                         \
      const c8 *const yAsString = #y;                                         \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : ({}) {} ({})\n",                              \
                   sp_fmt_cstr(xAsString), sp_fmt_cstr(#cond),                \
                   sp_fmt_cstr(yAsString));                                    \
      UTEST_PRINTF("    Actual : ");                                           \
      utest_type_printer(xEval);                                               \
      UTEST_PRINTF(" vs ");                                                    \
      utest_type_printer(yEval);                                               \
      UTEST_PRINTF("\n");                                                      \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END
#else
#define UTEST_COND(x, y, cond, msg, is_assert)                                 \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    if (!((x)cond(y))) {                                                       \
      UTEST_PRINTF("{}:{}: Failure (Expected {} Actual)\n",                    \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__),                \
                   sp_fmt_cstr(#cond));                                        \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END
#endif

#define EXPECT_EQ(x, y) UTEST_COND(x, y, ==, "", 0)
#define EXPECT_EQ_MSG(x, y, msg) UTEST_COND(x, y, ==, msg, 0)
#define ASSERT_EQ(x, y) UTEST_COND(x, y, ==, "", 1)
#define ASSERT_EQ_MSG(x, y, msg) UTEST_COND(x, y, ==, msg, 1)

#define EXPECT_NE(x, y) UTEST_COND(x, y, !=, "", 0)
#define EXPECT_NE_MSG(x, y, msg) UTEST_COND(x, y, !=, msg, 0)
#define ASSERT_NE(x, y) UTEST_COND(x, y, !=, "", 1)
#define ASSERT_NE_MSG(x, y, msg) UTEST_COND(x, y, !=, msg, 1)

#define EXPECT_LT(x, y) UTEST_COND(x, y, <, "", 0)
#define EXPECT_LT_MSG(x, y, msg) UTEST_COND(x, y, <, msg, 0)
#define ASSERT_LT(x, y) UTEST_COND(x, y, <, "", 1)
#define ASSERT_LT_MSG(x, y, msg) UTEST_COND(x, y, <, msg, 1)

#define EXPECT_LE(x, y) UTEST_COND(x, y, <=, "", 0)
#define EXPECT_LE_MSG(x, y, msg) UTEST_COND(x, y, <=, msg, 0)
#define ASSERT_LE(x, y) UTEST_COND(x, y, <=, "", 1)
#define ASSERT_LE_MSG(x, y, msg) UTEST_COND(x, y, <=, msg, 1)

#define EXPECT_GT(x, y) UTEST_COND(x, y, >, "", 0)
#define EXPECT_GT_MSG(x, y, msg) UTEST_COND(x, y, >, msg, 0)
#define ASSERT_GT(x, y) UTEST_COND(x, y, >, "", 1)
#define ASSERT_GT_MSG(x, y, msg) UTEST_COND(x, y, >, msg, 1)

#define EXPECT_GE(x, y) UTEST_COND(x, y, >=, "", 0)
#define EXPECT_GE_MSG(x, y, msg) UTEST_COND(x, y, >=, msg, 0)
#define ASSERT_GE(x, y) UTEST_COND(x, y, >=, "", 1)
#define ASSERT_GE_MSG(x, y, msg) UTEST_COND(x, y, >=, msg, 1)

#define UTEST_TRUE(x, msg, is_assert)                                          \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const int xEval = !!(x);                                                   \
    if (!(xEval)) {                                                            \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : true\n");                                     \
      UTEST_PRINTF("    Actual : {}\n",                                        \
                   sp_fmt_cstr((xEval) ? "true" : "false"));                  \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_TRUE(x) UTEST_TRUE(x, "", 0)
#define EXPECT_TRUE_MSG(x, msg) UTEST_TRUE(x, msg, 0)
#define ASSERT_TRUE(x) UTEST_TRUE(x, "", 1)
#define ASSERT_TRUE_MSG(x, msg) UTEST_TRUE(x, msg, 1)

#define UTEST_FALSE(x, msg, is_assert)                                         \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const int xEval = !!(x);                                                   \
    if (xEval) {                                                               \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : false\n");                                    \
      UTEST_PRINTF("    Actual : {}\n",                                        \
                   sp_fmt_cstr((xEval) ? "true" : "false"));                  \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_FALSE(x) UTEST_FALSE(x, "", 0)
#define EXPECT_FALSE_MSG(x, msg) UTEST_FALSE(x, msg, 0)
#define ASSERT_FALSE(x) UTEST_FALSE(x, "", 1)
#define ASSERT_FALSE_MSG(x, msg) UTEST_FALSE(x, msg, 1)

#define UTEST_STREQ(x, y, msg, is_assert)                                      \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const c8 *xEval = (x);                                                    \
    const c8 *yEval = (y);                                                    \
    if (UTEST_NULL == xEval || UTEST_NULL == yEval ||                          \
        !sp_cstr_equal(xEval, yEval)) {                                       \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : \"{}\"\n",                                    \
                   sp_fmt_cstr(xEval ? xEval : "(null)"));                    \
      UTEST_PRINTF("    Actual : \"{}\"\n",                                    \
                   sp_fmt_cstr(yEval ? yEval : "(null)"));                    \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_STREQ(x, y) UTEST_STREQ(x, y, "", 0)
#define EXPECT_STREQ_MSG(x, y, msg) UTEST_STREQ(x, y, msg, 0)
#define ASSERT_STREQ(x, y) UTEST_STREQ(x, y, "", 1)
#define ASSERT_STREQ_MSG(x, y, msg) UTEST_STREQ(x, y, msg, 1)

#define UTEST_STRNE(x, y, msg, is_assert)                                      \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const c8 *xEval = (x);                                                    \
    const c8 *yEval = (y);                                                    \
    if (UTEST_NULL == xEval || UTEST_NULL == yEval ||                          \
        sp_cstr_equal(xEval, yEval)) {                                        \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : \"{}\"\n",                                    \
                   sp_fmt_cstr(xEval ? xEval : "(null)"));                    \
      UTEST_PRINTF("    Actual : \"{}\"\n",                                    \
                   sp_fmt_cstr(yEval ? yEval : "(null)"));                    \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_STRNE(x, y) UTEST_STRNE(x, y, "", 0)
#define EXPECT_STRNE_MSG(x, y, msg) UTEST_STRNE(x, y, msg, 0)
#define ASSERT_STRNE(x, y) UTEST_STRNE(x, y, "", 1)
#define ASSERT_STRNE_MSG(x, y, msg) UTEST_STRNE(x, y, msg, 1)

#define UTEST_STRNEQ(x, y, n, msg, is_assert)                                  \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const c8 *xEval = (x);                                                    \
    const c8 *yEval = (y);                                                    \
    const u32 nEval = UTEST_CAST(u32, n);                                     \
    if (UTEST_NULL == xEval || UTEST_NULL == yEval ||                          \
        0 != UTEST_STRNCMP(xEval, yEval, nEval)) {                             \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : \"{}\"\n",                                    \
                   sp_fmt_str(sp_str(xEval, nEval)));                          \
      UTEST_PRINTF("    Actual : \"{}\"\n",                                    \
                   sp_fmt_str(sp_str(yEval, nEval)));                          \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_STRNEQ(x, y, n) UTEST_STRNEQ(x, y, n, "", 0)
#define EXPECT_STRNEQ_MSG(x, y, n, msg) UTEST_STRNEQ(x, y, n, msg, 0)
#define ASSERT_STRNEQ(x, y, n) UTEST_STRNEQ(x, y, n, "", 1)
#define ASSERT_STRNEQ_MSG(x, y, n, msg) UTEST_STRNEQ(x, y, n, msg, 1)

#define UTEST_STRNNE(x, y, n, msg, is_assert)                                  \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const c8 *xEval = (x);                                                    \
    const c8 *yEval = (y);                                                    \
    const u32 nEval = UTEST_CAST(u32, n);                                     \
    if (UTEST_NULL == xEval || UTEST_NULL == yEval ||                          \
        0 == UTEST_STRNCMP(xEval, yEval, nEval)) {                             \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : \"{}\"\n",                                    \
                   sp_fmt_str(sp_str(xEval, nEval)));                          \
      UTEST_PRINTF("    Actual : \"{}\"\n",                                    \
                   sp_fmt_str(sp_str(yEval, nEval)));                          \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_STRNNE(x, y, n) UTEST_STRNNE(x, y, n, "", 0)
#define EXPECT_STRNNE_MSG(x, y, n, msg) UTEST_STRNNE(x, y, n, msg, 0)
#define ASSERT_STRNNE(x, y, n) UTEST_STRNNE(x, y, n, "", 1)
#define ASSERT_STRNNE_MSG(x, y, n, msg) UTEST_STRNNE(x, y, n, msg, 1)

#define UTEST_NEAR(x, y, epsilon, msg, is_assert)                              \
  UTEST_SURPRESS_WARNING_BEGIN do {                                            \
    const f64 diff =                                                           \
        utest_fabs(UTEST_CAST(f64, x) - UTEST_CAST(f64, y));                  \
    if (diff > UTEST_CAST(f64, epsilon) || utest_isnan(diff)) {               \
      UTEST_PRINTF("{}:{}: Failure\n",                                         \
                   sp_fmt_cstr(__FILE__), sp_fmt_int(__LINE__));               \
      UTEST_PRINTF("  Expected : {}\n", sp_fmt_float(UTEST_CAST(f64, x)));     \
      UTEST_PRINTF("    Actual : {}\n", sp_fmt_float(UTEST_CAST(f64, y)));     \
      if ((msg)[0] != '\0') {                                                  \
        UTEST_PRINTF("   Message : {}\n", sp_fmt_cstr(msg));                  \
      }                                                                        \
      *utest_result = UTEST_TEST_FAILURE;                                      \
      if (is_assert) {                                                         \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  while (0)                                                                    \
  UTEST_SURPRESS_WARNING_END

#define EXPECT_NEAR(x, y, epsilon) UTEST_NEAR(x, y, epsilon, "", 0)
#define EXPECT_NEAR_MSG(x, y, epsilon, msg) UTEST_NEAR(x, y, epsilon, msg, 0)
#define ASSERT_NEAR(x, y, epsilon) UTEST_NEAR(x, y, epsilon, "", 1)
#define ASSERT_NEAR_MSG(x, y, epsilon, msg) UTEST_NEAR(x, y, epsilon, msg, 1)

#if defined(__clang__)
#if __has_warning("-Wunsafe-buffer-usage")
#define UTEST_SURPRESS_WARNINGS_BEGIN                                          \
  _Pragma("clang diagnostic push")                                             \
      _Pragma("clang diagnostic ignored \"-Wunsafe-buffer-usage\"")
#define UTEST_SURPRESS_WARNINGS_END _Pragma("clang diagnostic pop")
#else
#define UTEST_SURPRESS_WARNINGS_BEGIN
#define UTEST_SURPRESS_WARNINGS_END
#endif
#else
#define UTEST_SURPRESS_WARNINGS_BEGIN
#define UTEST_SURPRESS_WARNINGS_END
#endif

#define UTEST(SET, NAME)                                                       \
  UTEST_SURPRESS_WARNINGS_BEGIN                                                \
  UTEST_EXTERN struct utest_state_s utest_state;                               \
  static void utest_run_##SET##_##NAME(int *utest_result);                     \
  static void utest_##SET##_##NAME(int *utest_result, u64 utest_index) {      \
    (void)utest_index;                                                         \
    utest_run_##SET##_##NAME(utest_result);                                    \
  }                                                                            \
  UTEST_INITIALIZER(utest_register_##SET##_##NAME) {                           \
    const u64 index = utest_state.tests_length++;                              \
    const c8 name_part[] = #SET "." #NAME;                                     \
    const u64 name_size = sizeof(name_part);                                   \
    const c8 set_part[] = #SET;                                     \
    const u64 set_size = sizeof(set_part);                                   \
    const c8 test_part[] = #NAME;                                     \
    const u64 test_size = sizeof(test_part);                                   \
    c8 *name = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(name_size));              \
    c8 *set = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(set_size));              \
    c8 *test = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(test_size));              \
    utest_state.tests = UTEST_PTR_CAST(                                        \
        struct utest_test_state_s *,                                           \
        utest_realloc(UTEST_PTR_CAST(void *, utest_state.tests),               \
                      sizeof(struct utest_test_state_s) *                      \
                          utest_state.tests_length));                          \
    if (utest_state.tests && name) {                                           \
      utest_state.tests[index].func = &utest_##SET##_##NAME;                   \
      utest_state.tests[index].name = name;                                    \
      utest_state.tests[index].set = set;                                    \
      utest_state.tests[index].test = test;                                    \
      utest_state.tests[index].index = 0;                                      \
      sp_mem_copy(name_part, name, name_size);                                 \
      sp_mem_copy(set_part, set, set_size);                                 \
      sp_mem_copy(test_part, test, test_size);                                 \
    } else {                                                                   \
      if (utest_state.tests) {                                                 \
        sp_mem_os_free(utest_state.tests);                                     \
        utest_state.tests = UTEST_NULL;                                        \
      }                                                                        \
      if (name) {                                                              \
        sp_mem_os_free(name);                                                   \
        sp_mem_os_free(set);                                                   \
        sp_mem_os_free(test);                                                   \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  UTEST_SURPRESS_WARNINGS_END                                                  \
  void utest_run_##SET##_##NAME(int *utest_result)

#define UTEST_F_SETUP(FIXTURE)                                                 \
  static void utest_f_setup_##FIXTURE(int *utest_result,                       \
                                      struct FIXTURE *utest_fixture)

#define UTEST_F_TEARDOWN(FIXTURE)                                              \
  static void utest_f_teardown_##FIXTURE(int *utest_result,                    \
                                         struct FIXTURE *utest_fixture)

#define UTEST_F(FIXTURE, NAME)                                                 \
  UTEST_SURPRESS_WARNINGS_BEGIN                                                \
  UTEST_EXTERN struct utest_state_s utest_state;                               \
  static void utest_f_setup_##FIXTURE(int *, struct FIXTURE *);                \
  static void utest_f_teardown_##FIXTURE(int *, struct FIXTURE *);             \
  static void utest_run_##FIXTURE##_##NAME(int *, struct FIXTURE *);           \
  static void utest_f_##FIXTURE##_##NAME(int *utest_result,                    \
                                         u64 utest_index) {                    \
    struct FIXTURE fixture;                                                    \
    (void)utest_index;                                                         \
    sp_mem_zero(&fixture, sizeof(fixture));                                    \
    utest_f_setup_##FIXTURE(utest_result, &fixture);                           \
    if (UTEST_TEST_PASSED != *utest_result) {                                  \
      return;                                                                  \
    }                                                                          \
    utest_run_##FIXTURE##_##NAME(utest_result, &fixture);                      \
    utest_f_teardown_##FIXTURE(utest_result, &fixture);                        \
  }                                                                            \
  UTEST_INITIALIZER(utest_register_##FIXTURE##_##NAME) {                       \
    const u64 index = utest_state.tests_length++;                              \
    const c8 name_part[] = #FIXTURE "." #NAME;                                 \
    const u64 name_size = sizeof(name_part);                                   \
    const c8 set_part[] = #FIXTURE;                                     \
    const u64 set_size = sizeof(set_part);                                   \
    const c8 test_part[] = #NAME;                                     \
    const u64 test_size = sizeof(test_part);                                   \
    c8 *name = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(name_size));              \
    c8 *set = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(set_size));              \
    c8 *test = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(test_size));              \
                                                                               \
    utest_state.tests = UTEST_PTR_CAST(                                        \
        struct utest_test_state_s *,                                           \
        utest_realloc(UTEST_PTR_CAST(void *, utest_state.tests),               \
                      sizeof(struct utest_test_state_s) *                      \
                          utest_state.tests_length));                          \
    if (utest_state.tests && name) {                                           \
      utest_state.tests[index].func = &utest_f_##FIXTURE##_##NAME;             \
      utest_state.tests[index].name = name;                                    \
      utest_state.tests[index].set = set;                                    \
      utest_state.tests[index].test = test;                                    \
      utest_state.tests[index].index = 0;                                      \
      sp_mem_copy(name_part, name, name_size);                                 \
      sp_mem_copy(set_part, set, set_size);                                    \
      sp_mem_copy(test_part, test, test_size);                                 \
    } else {                                                                   \
      if (utest_state.tests) {                                                 \
        sp_mem_os_free(utest_state.tests);                                     \
        utest_state.tests = UTEST_NULL;                                        \
      }                                                                        \
      if (name) {                                                              \
        sp_mem_os_free(name);                                                   \
        sp_mem_os_free(set);                                                   \
        sp_mem_os_free(test);                                                   \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  UTEST_SURPRESS_WARNINGS_END                                                  \
  void utest_run_##FIXTURE##_##NAME(int *utest_result,                         \
                                    struct FIXTURE *utest_fixture)

#define UTEST_I_SETUP(FIXTURE)                                                 \
  static void utest_i_setup_##FIXTURE(                                         \
      int *utest_result, struct FIXTURE *utest_fixture, u64 utest_index)

#define UTEST_I_TEARDOWN(FIXTURE)                                              \
  static void utest_i_teardown_##FIXTURE(                                      \
      int *utest_result, struct FIXTURE *utest_fixture, u64 utest_index)

#define UTEST_I(FIXTURE, NAME, INDEX)                                          \
  UTEST_SURPRESS_WARNINGS_BEGIN                                                \
  UTEST_EXTERN struct utest_state_s utest_state;                               \
  static void utest_run_##FIXTURE##_##NAME##_##INDEX(int *, struct FIXTURE *); \
  static void utest_i_##FIXTURE##_##NAME##_##INDEX(int *utest_result,          \
                                                   u64 index) {                \
    struct FIXTURE fixture;                                                    \
    sp_mem_zero(&fixture, sizeof(fixture));                                    \
    utest_i_setup_##FIXTURE(utest_result, &fixture, index);                    \
    if (UTEST_TEST_PASSED != *utest_result) {                                  \
      return;                                                                  \
    }                                                                          \
    utest_run_##FIXTURE##_##NAME##_##INDEX(utest_result, &fixture);            \
    utest_i_teardown_##FIXTURE(utest_result, &fixture, index);                 \
  }                                                                            \
  UTEST_INITIALIZER(utest_register_##FIXTURE##_##NAME##_##INDEX) {             \
    u64 i;                                                                     \
    for (i = 0; i < (INDEX); i++) {                                            \
      const u64 index = utest_state.tests_length++;                            \
      const c8 name_part[] = #FIXTURE "." #NAME;                               \
      sp_str_t fmtd = sp_fmt_a(sp_context_get_allocator(), "{}/{}", sp_fmt_cstr(name_part),                \
                                       sp_fmt_uint(i)).value;                         \
      u64 name_size = fmtd.len + 1;                                           \
      c8 *name = UTEST_PTR_CAST(c8 *, sp_mem_os_alloc(name_size));            \
      utest_state.tests = UTEST_PTR_CAST(                                      \
          struct utest_test_state_s *,                                         \
          utest_realloc(UTEST_PTR_CAST(void *, utest_state.tests),             \
                        sizeof(struct utest_test_state_s) *                    \
                            utest_state.tests_length));                        \
      if (utest_state.tests && name) {                                         \
        utest_state.tests[index].func = &utest_i_##FIXTURE##_##NAME##_##INDEX; \
        utest_state.tests[index].index = i;                                    \
        utest_state.tests[index].name = name;                                  \
        sp_mem_copy(fmtd.data, name, fmtd.len);                               \
        name[fmtd.len] = '\0';                                                \
      } else {                                                                 \
        if (utest_state.tests) {                                               \
          sp_mem_os_free(utest_state.tests);                                    \
          utest_state.tests = UTEST_NULL;                                      \
        }                                                                      \
        if (name) {                                                            \
          sp_mem_os_free(name);                                                 \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  UTEST_SURPRESS_WARNINGS_END                                                  \
  void utest_run_##FIXTURE##_##NAME##_##INDEX(int *utest_result,               \
                                              struct FIXTURE *utest_fixture)

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

UTEST_WEAK
f64 utest_fabs(f64 d);
UTEST_WEAK
f64 utest_fabs(f64 d) {
  union {
    f64 d;
    u64 u;
  } both;
  both.d = d;
  both.u &= 0x7fffffffffffffffu;
  return both.d;
}

UTEST_WEAK
int utest_isnan(f64 d);
UTEST_WEAK
int utest_isnan(f64 d) {
  union {
    f64 d;
    u64 u;
  } both;
  both.d = d;
  both.u &= 0x7fffffffffffffffu;
  return both.u > 0x7ff0000000000000u;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if defined(__clang__)
#if __has_warning("-Wunsafe-buffer-usage")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#endif

UTEST_WEAK
int utest_should_filter_test(const c8 *filter, const c8 *testcase);
UTEST_WEAK int utest_should_filter_test(const c8 *filter,
                                        const c8 *testcase) {
  if (filter) {
    const c8 *filter_cur = filter;
    const c8 *testcase_cur = testcase;
    const c8 *filter_wildcard = UTEST_NULL;

    while (('\0' != *filter_cur) && ('\0' != *testcase_cur)) {
      if ('*' == *filter_cur) {
        /* store the position of the wildcard */
        filter_wildcard = filter_cur;

        /* skip the wildcard character */
        filter_cur++;

        while (('\0' != *filter_cur) && ('\0' != *testcase_cur)) {
          if ('*' == *filter_cur) {
            /*
               we found another wildcard (filter is something like *foo*) so we
               exit the current loop, and return to the parent loop to handle
               the wildcard case
            */
            break;
          } else if (*filter_cur != *testcase_cur) {
            /* otherwise our filter didn't match, so reset it */
            filter_cur = filter_wildcard;
          }

          /* move testcase along */
          testcase_cur++;

          /* move filter along */
          filter_cur++;
        }

        if (('\0' == *filter_cur) && ('\0' == *testcase_cur)) {
          return 0;
        }

        /* if the testcase has been exhausted, we don't have a match! */
        if ('\0' == *testcase_cur) {
          return 1;
        }
      } else {
        if (*testcase_cur != *filter_cur) {
          /* test case doesn't match filter */
          return 1;
        } else {
          /* move our filter and testcase forward */
          testcase_cur++;
          filter_cur++;
        }
      }
    }

    if (('\0' != *filter_cur) ||
        (('\0' != *testcase_cur) &&
         ((filter == filter_cur) || ('*' != filter_cur[-1])))) {
      /* we have a mismatch! */
      return 1;
    }
  }

  return 0;
}

static UTEST_INLINE s32 utest_main(s32 argc, const c8 **argv);
s32 utest_main(s32 argc, const c8 **argv) {
#if defined(SP_FREESTANDING)
  {
    typedef void (*_utest_init_fn)(void);
    extern _utest_init_fn __init_array_start[];
    extern _utest_init_fn __init_array_end[];
    for (_utest_init_fn *fn = __init_array_start; fn < __init_array_end; fn++) {
      (*fn)();
    }
  }
#endif
  u64 failed = 0;
  u64 skipped = 0;
  u64 index = 0;
  u64 *failed_testcases = UTEST_NULL;
  u64 failed_testcases_length = 0;
  u64 *skipped_testcases = UTEST_NULL;
  u64 skipped_testcases_length = 0;
  const c8 *filter = UTEST_NULL;
  u64 ran_tests = 0;
  int enable_mixed_units = 1;
  int random_order = 0;
  u32 seed = 0;

  /* loop through all arguments looking for our options */
  for (index = 1; index < UTEST_CAST(u64, argc); index++) {
    /* Informational switches */
    const c8 help_str[] = "--help";
    const c8 list_str[] = "--list-tests";
    /* Test config switches */
    const c8 filter_str[] = "--filter=";
    const c8 output_str[] = "--output=";
    const c8 enable_mixed_units_str[] = "--enable-mixed-units";
    const c8 random_order_str[] = "--random-order";
    const c8 random_order_with_seed_str[] = "--random-order=";

    if (0 == UTEST_STRNCMP(argv[index], help_str, sizeof(help_str) - 1)) {
      sp_os_print(SP_LIT(
             "utest.h - the single file unit testing solution for C/C++!\n"
             "Command line Options:\n"
             "  --help                  Show this message and exit.\n"
             "  --filter=<filter>       Filter the test cases to run (EG. "
             "MyTest*.a would run MyTestCase.a but not MyTestCase.b).\n"
             "  --list-tests            List testnames, one per line. Output "
             "names can be passed to --filter.\n"
             "  --output=<output>       Output an xunit XML file to the file "
             "specified in <output>.\n"
             "  --disable-mixed-units    Disable the per-test output to contain "
             "mixed units (s/ms/us/ns).\n"
             "  --random-order[=<seed>] Randomize the order that the tests are "
             "ran in. If the optional <seed> argument is not provided, then a "
             "random starting seed is used.\n"));
      goto cleanup;
    } else if (0 ==
               UTEST_STRNCMP(argv[index], filter_str, sizeof(filter_str) - 1)) {
      /* user wants to filter what test cases run! */
      filter = argv[index] + sizeof(filter_str) - 1;
    } else if (0 ==
               UTEST_STRNCMP(argv[index], output_str, sizeof(output_str) - 1)) {
      sp_str_t path = sp_str_view(argv[index] + sizeof(output_str) - 1);
      sp_io_writer_from_file(&utest_state.output, path,
                             SP_IO_WRITE_MODE_OVERWRITE);
      utest_state.has_output = 1;
    } else if (0 == UTEST_STRNCMP(argv[index], list_str,
                                  sizeof(list_str) - 1)) {
      for (index = 0; index < utest_state.tests_length; index++) {
        sp_log_a("{}", sp_fmt_cstr(utest_state.tests[index].name));
      }
      /* when printing the test list, don't actually run the tests */
      return 0;
    } else if (0 == UTEST_STRNCMP(argv[index], enable_mixed_units_str,
                                  sizeof(enable_mixed_units_str) - 1)) {
      enable_mixed_units = 0;
    } else if (0 == UTEST_STRNCMP(argv[index], random_order_with_seed_str,
                                  sizeof(random_order_with_seed_str) - 1)) {
      seed = sp_parse_u32(sp_str_view(
          argv[index] + sizeof(random_order_with_seed_str) - 1));
      random_order = 1;
    } else if (0 == UTEST_STRNCMP(argv[index], random_order_str,
                                  sizeof(random_order_str) - 1)) {
      const s64 ns = utest_ns();

      // Some really poor pseudo-random using the current time.
      seed = UTEST_CAST(u32, ns >> 32) * 31 +
             UTEST_CAST(u32, ns & 0xffffffff);
      random_order = 1;
    }
  }

  if (random_order) {
    // Use Fisher-Yates with the Durstenfield's version to randomly re-order the
    // tests.
    for (index = utest_state.tests_length; index > 1; index--) {
      // For the random order we'll use PCG.
      const u32 state = seed;
      const u32 word =
          ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
      const u32 next =
          ((word >> 22u) ^ word) % UTEST_CAST(u32, index);

      // Swap the randomly chosen element into the last location.
      const struct utest_test_state_s copy = utest_state.tests[index - 1];
      utest_state.tests[index - 1] = utest_state.tests[next];
      utest_state.tests[next] = copy;

      // Move the seed onwards.
      seed = seed * 747796405u + 2891336453u;
    }
  }

  for (index = 0; index < utest_state.tests_length; index++) {
    if (utest_should_filter_test(filter, utest_state.tests[index].name)) {
      continue;
    }

    ran_tests++;
  }

  sp_str_t arch = sp_str_lit("unknown");
  #if defined(SP_AMD64)
  arch = sp_str_lit("x86_64");
  #elif defined(SP_ARM64)
  arch = sp_str_lit("aarch64");
  #endif

  sp_str_t os = sp_os_get_name();

  sp_str_t abi = sp_str_lit("unknown");


  #if defined(SP_LIBC_MSVC)
    abi = sp_str_lit("msvc");
  #elif defined(SP_LIBC_GNU)
    abi = sp_str_lit("gnu");
  #elif defined(SP_LIBC_NONE)
    abi = sp_str_lit("none");
  #elif defined(SP_MACOS)
    abi = sp_str_lit("apple");
  #elif defined(SP_LINUX)
    abi = sp_str_lit("musl");
  #endif

  sp_log_a(
    "> running {.fg brightblack} test cases on {}-{}-{}",
    sp_fmt_uint(ran_tests),
    sp_fmt_str(arch), sp_fmt_str(os), sp_fmt_str(abi)
  );

  for (index = 0; index < utest_state.tests_length; index++) {
    int result = UTEST_TEST_PASSED;
    s64 ns = 0;

    if (utest_should_filter_test(filter, utest_state.tests[index].name)) {
      continue;
    }

    sp_print_a("{}.{}...", sp_fmt_cstr(utest_state.tests[index].set), sp_fmt_cstr(utest_state.tests[index].test));

    ns = utest_ns();
    utest_state.tests[index].func(&result, utest_state.tests[index].index);
    ns = utest_ns() - ns;

    // Record the failing test.
    if (UTEST_TEST_FAILURE == result) {
      const u64 failed_testcase_index = failed_testcases_length++;
      failed_testcases = (u64*)utest_realloc(failed_testcases, sizeof(u64) * failed_testcases_length);
      if (UTEST_NULL != failed_testcases) {
        failed_testcases[failed_testcase_index] = index;
      }
      failed++;
    } else if (UTEST_TEST_SKIPPED == result) {
      const u64 skipped_testcase_index = skipped_testcases_length++;
      skipped_testcases = UTEST_PTR_CAST(
          u64 *, utest_realloc(UTEST_PTR_CAST(void *, skipped_testcases),
                                  sizeof(u64) * skipped_testcases_length));
      if (UTEST_NULL != skipped_testcases) {
        skipped_testcases[skipped_testcase_index] = index;
      }
      skipped++;
    }

    {
      const c8 *const units[] = {"ns", "us", "ms", "s", UTEST_NULL};
      unsigned int unit_index = 0;
      s64 time = ns;

      if (enable_mixed_units) {
        for (unit_index = 0; UTEST_NULL != units[unit_index]; unit_index++) {
          if (10000 > time) {
            break;
          }

          time /= 1000;
        }
      }

      if (UTEST_TEST_FAILURE == result) {
        sp_print_a("{.fg red} ", sp_fmt_cstr("failed"));
      } else if (UTEST_TEST_SKIPPED == result) {
        sp_print_a("{.fg yellow} ", sp_fmt_cstr("skipped"));
      } else {
        sp_print_a("{.fg green} ", sp_fmt_cstr("ok"));
      }
      sp_log_a("{.fg brightblack}{.fg brightblack}", sp_fmt_int(time), sp_fmt_cstr(units[unit_index]));
    }
  }

  sp_log_a("{.fg green} {} test cases ran.",
         sp_fmt_cstr("[==========]"),
         sp_fmt_uint(ran_tests));
  sp_log_a("{}[  PASSED  ]{} {} tests.",
         sp_fmt_cstr(SP_ANSI_FG_GREEN), sp_fmt_cstr(SP_ANSI_RESET),
         sp_fmt_uint(ran_tests - failed - skipped));

  if (0 != skipped) {
    sp_log_a("{}[  SKIPPED ]{} {} tests, listed below:",
           sp_fmt_cstr(SP_ANSI_FG_YELLOW), sp_fmt_cstr(SP_ANSI_RESET),
           sp_fmt_uint(skipped));
    for (index = 0; index < skipped_testcases_length; index++) {
      sp_log_a("{}[  SKIPPED ]{} {}",
             sp_fmt_cstr(SP_ANSI_FG_YELLOW), sp_fmt_cstr(SP_ANSI_RESET),
             sp_fmt_cstr(utest_state.tests[skipped_testcases[index]].name));
    }
  }

  if (0 != failed) {
    sp_log_a("{}[  FAILED  ]{} {} tests, listed below:",
           sp_fmt_cstr(SP_ANSI_FG_RED), sp_fmt_cstr(SP_ANSI_RESET),
           sp_fmt_uint(failed));
    for (index = 0; index < failed_testcases_length; index++) {
      sp_log_a("{}[  FAILED  ]{} {}",
             sp_fmt_cstr(SP_ANSI_FG_RED), sp_fmt_cstr(SP_ANSI_RESET),
             sp_fmt_cstr(utest_state.tests[failed_testcases[index]].name));
    }
  }

  if (utest_state.has_output) {
    sp_io_write_cstr(&utest_state.output, "</testsuite>\n</testsuites>\n",
                     SP_NULLPTR);
  }

cleanup:
  for (index = 0; index < utest_state.tests_length; index++) {
    sp_mem_os_free(UTEST_PTR_CAST(void *, utest_state.tests[index].name));
  }

  sp_mem_os_free(UTEST_PTR_CAST(void *, skipped_testcases));
  sp_mem_os_free(UTEST_PTR_CAST(void *, failed_testcases));
  sp_mem_os_free(UTEST_PTR_CAST(void *, utest_state.tests));

  if (utest_state.has_output) {
    sp_io_writer_close(&utest_state.output);
  }

  return UTEST_CAST(int, failed);
}

#if defined(__clang__)
#if __has_warning("-Wunsafe-buffer-usage")
#pragma clang diagnostic pop
#endif
#endif

/*
   we need, in exactly one source file, define the global struct that will hold
   the data we need to run utest. This macro allows the user to declare the
   data without having to use the UTEST_MAIN macro, thus allowing them to write
   their own main() function.
*/
#define UTEST_STATE() struct utest_state_s utest_state = {0, 0, {0}, 0}

/*
   define a main() function to call into utest.h and start executing tests! A
   user can optionally not use this macro, and instead define their own main()
   function and manually call utest_main. The user must, in exactly one source
   file, use the UTEST_STATE macro to declare a global struct variable that
   utest requires.
*/
#define UTEST_MAIN()                                                           \
  UTEST_STATE();                                                               \
  s32 _utest_entry(s32 argc, const c8 **argv) {                               \
    return utest_main(argc, argv);                                             \
  }                                                                            \
  SP_MAIN(_utest_entry)

#endif /* SHEREDOM_UTEST_H_INCLUDED */
