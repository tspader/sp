// SP_IMPLEMENTATION is defined in the Makefile to make clangd happy
#include "sp.h"

#include "process.h"

#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"

static const c8* const usage[] = {
  "test_proc [options] --fn <function>",
  NULL,
};

s32 main(s32 num_args, const c8** args) {
  const c8* function_str = NULL;
  s32 exit_code = 0;
  s32 stdout_enabled = 0;
  s32 stderr_enabled = 0;

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
  num_args = argparse_parse(&argparse, num_args, args);

  if (!function_str) {
    argparse_usage(&argparse);
    SP_EXIT_FAILURE();
  }

  test_proc_function_t function = test_proc_function_from_str(sp_str_view(function_str));

  if (function == TEST_PROC_FUNCTION_INVALID) {
    SP_LOG("unknown function: {:fg brightred}", SP_FMT_CSTR(function_str));
    SP_EXIT_FAILURE();
  }

  switch (function) {
    case TEST_PROC_FUNCTION_ECHO: {
      c8 buffer[1024];
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
      c8 line[256];
      while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
          line[len - 1] = '\0';
        }
        if (len == 0 || line[0] == '\0') break;

        const c8* value = getenv(line);
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
    case TEST_PROC_FUNCTION_WAIT: {
      sp_str_t arg = sp_str_view(args[0]);
      f64 ms = sp_parse_f64(arg);
      SP_LOG("{:fg brightyellow} is sleeping for {:fg cyan}ms", SP_FMT_STR(sp_os_get_executable_path()), SP_FMT_F64(ms));
      sp_os_sleep_ms(ms);
      SP_LOG("{:fg brightyellow} is done", SP_FMT_STR(sp_os_get_executable_path()));
      return 69;
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
