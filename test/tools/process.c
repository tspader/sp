#include "sp.h"

#include "process.h"

#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"

static const char* const usage[] = {
  "test_proc [options] --fn <function>",
  NULL,
};

int main(int argc, const char** argv) {
  const char* function_str = NULL;
  int exit_code = 0;
  int stdout_enabled = 0;
  int stderr_enabled = 0;

  struct argparse_option options[] = {
    OPT_HELP(),
    OPT_STRING('f', "fn", &function_str, "Function to execute (echo/print/print_env/exit_code)"),
    OPT_INTEGER('e', "exit-code", &exit_code, "Exit code for exit_code function"),
    OPT_BOOLEAN(0, "stdout", &stdout_enabled, "Enable stdout"),
    OPT_BOOLEAN(0, "stderr", &stderr_enabled, "Enable stderr"),
    OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, usage, 0);
  argparse_describe(&argparse, "\nTest process for sp_proc testing", "");
  argc = argparse_parse(&argparse, argc, argv);

  if (!function_str) {
    argparse_usage(&argparse);
    return 1;
  }

  test_proc_function_t function = test_proc_function_from_str(sp_str_view(function_str));

  if (function == TEST_PROC_FUNCTION_INVALID) {
    fprintf(stderr, "Unknown function: %s\n", function_str);
    return 1;
  }

  switch (function) {
    case TEST_PROC_FUNCTION_ECHO: {
      char buffer[1024];
      while (fgets(buffer, sizeof(buffer), stdin)) {
        if (stdout_enabled) fprintf(stdout, "%s", buffer);
        if (stderr_enabled) fprintf(stderr, "%s", buffer);
      }
      if (stdout_enabled) fflush(stdout);
      if (stderr_enabled) fflush(stderr);
      break;
    }
    case TEST_PROC_FUNCTION_PRINT: {
      if (stdout_enabled) {
        fprintf(stdout, "%.*s", sp_test_ps_canary.len, sp_test_ps_canary.data);
        fflush(stdout);
      }
      if (stderr_enabled) {
        fprintf(stderr, "%.*s", sp_test_ps_canary.len, sp_test_ps_canary.data);
        fflush(stderr);
      }
      break;
    }
    case TEST_PROC_FUNCTION_PRINT_ENV: {
      char line[256];
      while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
          line[len - 1] = '\0';
        }
        if (len == 0 || line[0] == '\0') break;
        
        const char* value = getenv(line);
        if (stdout_enabled) {
          if (value) {
            fprintf(stdout, "%s %s\n", line, value);
          } else {
            fprintf(stdout, "%s\n", line);
          }
          fflush(stdout);
        }
        if (stderr_enabled) {
          if (value) {
            fprintf(stderr, "%s %s\n", line, value);
          } else {
            fprintf(stderr, "%s\n", line);
          }
          fflush(stderr);
        }
      }
      break;
    }
    case TEST_PROC_FUNCTION_EXIT_CODE: {
      return exit_code;
    }
    default: {
      fprintf(stderr, "Unknown function: %s\n", function_str);
      return 1;
    }
  }

  return 0;
}
