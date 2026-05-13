#include "sp.h"

#if defined(SP_FREESTANDING) || defined(SP_WASM_FREESTANDING)
int run(int n, const char** a) { return 0; }
SP_MAIN(run);
#else

#ifdef _WIN32
  #define sp_getpid() ((s32)GetCurrentProcessId())
#elif defined(SP_WASM)
  #define sp_getpid() 0
#else
  #define sp_getpid() getpid()
#endif

#include "process.h"

#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"

static const c8* const usage[] = {
  "test_proc [options] --fn <function>",
  NULL,
};

static void write_str(bool stdout_enabled, bool stderr_enabled, sp_str_t s) {
  if (stdout_enabled) sp_sys_write(sp_sys_stdout, s.data, s.len);
  if (stderr_enabled) sp_sys_write(sp_sys_stderr, s.data, s.len);
}

static void write_bytes(bool stdout_enabled, bool stderr_enabled, const void* p, u64 n) {
  if (stdout_enabled) sp_sys_write(sp_sys_stdout, p, n);
  if (stderr_enabled) sp_sys_write(sp_sys_stderr, p, n);
}

// Reads one byte at a time from stdin until '\n' or EOF. Returns the number of bytes
// read (excluding any terminating null), with the trailing '\n' stripped. Returns 0
// when stdin is at EOF before any byte is read.
static u32 read_line(c8* buf, u32 cap) {
  u32 len = 0;
  while (len + 1 < cap) {
    c8 c;
    s64 n = sp_sys_read(sp_sys_stdin, &c, 1);
    if (n <= 0) break;
    if (c == '\n') break;
    buf[len++] = c;
  }
  buf[len] = '\0';
  return len;
}

s32 main(s32 num_args, const c8** args) {
  const c8* function_str = NULL;
  s32 exit_code = 0;
  s32 stdout_enabled = 0;
  s32 stderr_enabled = 0;
  const c8* pattern_char = "A";
  s32 pattern_size = 100;
  s32 pattern_count = 10;

  sp_mem_t mem = sp_mem_os_new();

  struct argparse_option options[] = {
    OPT_HELP(),
    OPT_STRING('f', "fn", &function_str, "Function to execute (echo/print/print_env/exit_code)"),
    OPT_INTEGER('e', "exit-code", &exit_code, "Exit code for exit_code function"),
    OPT_BOOLEAN(0, "stdout", &stdout_enabled, "Enable stdout"),
    OPT_BOOLEAN(0, "stderr", &stderr_enabled, "Enable stderr"),
    OPT_STRING('c', "char", &pattern_char, "Character for pattern function"),
    OPT_INTEGER('s', "size", &pattern_size, "Size of each write for pattern function"),
    OPT_INTEGER('n', "count", &pattern_count, "Number of writes for pattern function"),
    OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, usage, 0);
  argparse_describe(&argparse, "\nTest process for sp_ps testing", "");
  num_args = argparse_parse(&argparse, num_args, args);

  if (!function_str) {
    argparse_usage(&argparse);
    SP_EXIT_FAILURE();
  }

  test_proc_function_t function = test_proc_function_from_str(sp_str_view(function_str));

  if (function == TEST_PROC_FUNCTION_INVALID) {
    sp_log("unknown function: {.fg brightred}", sp_fmt_cstr(function_str));
    SP_EXIT_FAILURE();
  }

  bool out_on = stdout_enabled != 0;
  bool err_on = stderr_enabled != 0;

  switch (function) {
    case TEST_PROC_FUNCTION_ECHO: {
      u8 buffer[1024];
      while (true) {
        s64 n = sp_sys_read(sp_sys_stdin, buffer, sizeof(buffer));
        if (n <= 0) break;
        write_bytes(out_on, err_on, buffer, (u64)n);
      }
      break;
    }
    case TEST_PROC_FUNCTION_ECHO_LINE: {
      c8 line[1024];
      while (true) {
        u32 len = read_line(line, sizeof(line));
        if (len == 0) break;
        sp_str_t prefixed = sp_fmt(mem, "echo: {}\n", sp_fmt_str(sp_str(line, len))).value;
        write_str(out_on, err_on, prefixed);
      }
      break;
    }
    case TEST_PROC_FUNCTION_PRINT: {
      write_str(out_on, err_on, sp_test_ps_canary);
      break;
    }
    case TEST_PROC_FUNCTION_PRINT_ENV: {
      c8 line[256];
      while (true) {
        u32 len = read_line(line, sizeof(line));
        if (len == 0) break;

        sp_str_t key = sp_str(line, len);
        sp_str_t value = sp_os_env_get(key);
        sp_str_t out = !sp_str_empty(value)
          ? sp_fmt(mem, "{} {}\n", sp_fmt_str(key), sp_fmt_str(value)).value
          : sp_fmt(mem, "{}\n", sp_fmt_str(key)).value;
        write_str(out_on, err_on, out);
      }
      break;
    }
    case TEST_PROC_FUNCTION_SLOW_WRITE: {
      const u32 chunk_size = 10;
      const u32 total_size = 100;
      const u32 delay_ms = 20;

      u8 buffer[10];
      for (u32 i = 0; i < chunk_size; i++) {
        buffer[i] = (u8)('A' + (i % 26));
      }

      u32 written = 0;
      while (written < total_size) {
        write_bytes(out_on, err_on, buffer, chunk_size);
        written += chunk_size;
        if (written < total_size) sp_os_sleep_ms(delay_ms);
      }
      break;
    }
    case TEST_PROC_FUNCTION_CONSUME: {
      u8 buffer[4096];
      u64 total_read = 0;
      while (true) {
        s64 n = sp_sys_read(sp_sys_stdin, buffer, sizeof(buffer));
        if (n <= 0) break;
        total_read += (u64)n;
      }
      sp_str_t line = sp_fmt(mem, "{}\n", sp_fmt_uint(total_read)).value;
      write_str(out_on, err_on, line);
      break;
    }
    case TEST_PROC_FUNCTION_WAIT: {
      sp_str_t arg = sp_str_view(args[0]);
      f64 ms = sp_parse_f64(arg);
      sp_log("process.c ({.fg brightyellow}) is sleeping for {.fg cyan}ms", sp_fmt_int(sp_getpid()), sp_fmt_float(ms));
      sp_os_sleep_ms(ms);
      sp_log("process.c ({.fg brightyellow}) is done", sp_fmt_int(sp_getpid()));
      return sp_test_ps_wait_exit_code;
    }
    case TEST_PROC_FUNCTION_EXIT_CODE: {
      return exit_code;
    }
    case TEST_PROC_FUNCTION_FLOOD: {
      const u32 flood_size = 512 * 1024;
      u8* buffer = sp_alloc_n(mem, u8, flood_size);
      for (u32 i = 0; i < flood_size; i++) {
        buffer[i] = (u8)('A' + (i % 26));
      }
      write_bytes(out_on, err_on, buffer, flood_size);
      break;
    }
    case TEST_PROC_FUNCTION_PATTERN: {
      u8* buffer = sp_alloc_n(mem, u8, pattern_size);
      for (s32 i = 0; i < pattern_size; i++) {
        buffer[i] = (u8)pattern_char[0];
      }
      for (s32 i = 0; i < pattern_count; i++) {
        write_bytes(out_on, err_on, buffer, pattern_size);
      }
      break;
    }
    default: {
      sp_log_err("Unknown function: {}", sp_fmt_cstr(function_str));
      return 1;
    }
  }

  return 0;
}
#endif
