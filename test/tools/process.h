#include "sp.h"

sp_str_t sp_test_ps_canary = sp_str_lit("foo");
s32 sp_test_ps_wait_exit_code = 69;

#define TEST_PROC_STREAM(X) \
  X(TEST_PROC_STREAM_STDOUT, "stdout") \
  X(TEST_PROC_STREAM_STDERR, "stderr")

typedef enum {
  TEST_PROC_STREAM(SP_X_NAMED_ENUM_DEFINE)
  TEST_PROC_STREAM_COUNT,
  TEST_PROC_STREAM_INVALID,
} test_proc_stream_t;

#define TEST_PROC_FUNCTION(X) \
  X(TEST_PROC_FUNCTION_ECHO,      "echo") \
  X(TEST_PROC_FUNCTION_ECHO_LINE, "echo_line") \
  X(TEST_PROC_FUNCTION_PRINT,     "print") \
  X(TEST_PROC_FUNCTION_PRINT_ENV, "print_env") \
  X(TEST_PROC_FUNCTION_WAIT,      "wait") \
  X(TEST_PROC_FUNCTION_EXIT_CODE, "exit_code")

typedef enum {
  TEST_PROC_FUNCTION(SP_X_NAMED_ENUM_DEFINE)
  TEST_PROC_FUNCTION_COUNT,
  TEST_PROC_FUNCTION_INVALID,
} test_proc_function_t;

const c8* test_proc_stream_to_cstr(test_proc_stream_t stream);
const c8* test_proc_function_to_cstr(test_proc_function_t fn);
test_proc_stream_t test_proc_stream_from_str(sp_str_t str);
test_proc_function_t test_proc_function_from_str(sp_str_t str);

const c8* test_proc_stream_to_cstr(test_proc_stream_t stream) {
  switch (stream) {
    TEST_PROC_STREAM(SP_X_NAMED_ENUM_CASE_TO_CSTR)
    default: SP_UNREACHABLE_RETURN("");
  }
}

const c8* test_proc_function_to_cstr(test_proc_function_t fn) {
  switch (fn) {
    TEST_PROC_FUNCTION(SP_X_NAMED_ENUM_CASE_TO_CSTR)
    default: SP_UNREACHABLE_RETURN("");
  }
}

test_proc_stream_t test_proc_stream_from_str(sp_str_t str) {
  TEST_PROC_STREAM(SP_X_NAMED_ENUM_STR_TO_ENUM)
  SP_UNREACHABLE_RETURN(TEST_PROC_STREAM_INVALID);
}

test_proc_function_t test_proc_function_from_str(sp_str_t str) {
  TEST_PROC_FUNCTION(SP_X_NAMED_ENUM_STR_TO_ENUM)
  SP_UNREACHABLE_RETURN(TEST_PROC_FUNCTION_INVALID);
}
