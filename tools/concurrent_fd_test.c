#define SP_IMPLEMENTATION
#include "sp.h"
#include <unistd.h>

sp_str_t get_process_path(void) {
  return sp_fs_join_path(sp_fs_get_exe_path(), SP_LIT("process"));
}

typedef struct {
  u32 transitions;
  u32 a_bytes;
  u32 b_bytes;
  bool interleaved;
} analysis_t;

analysis_t analyze(u8* data, u32 len, u32 write_size) {
  analysis_t result = SP_ZERO_INITIALIZE();
  if (len == 0) return result;

  c8 prev = (c8)data[0];
  u32 run_start = 0;

  sp_for(i, len) {
    c8 c = (c8)data[i];
    if (c == 'A') result.a_bytes++;
    else if (c == 'B') result.b_bytes++;

    if (c != prev) {
      u32 run_len = i - run_start;
      if (run_len % write_size != 0) {
        result.interleaved = true;
      }
      result.transitions++;
      run_start = i;
      prev = c;
    }
  }

  u32 final_run_len = len - run_start;
  if (final_run_len % write_size != 0) {
    result.interleaved = true;
  }

  return result;
}

void run_test(const c8* label, u32 write_size, u32 write_count) {
  s32 pipes[2];
  SP_ASSERT(pipe(pipes) == 0);

  sp_str_t size_str = sp_format("{}", SP_FMT_U32(write_size));
  sp_str_t count_str = sp_format("{}", SP_FMT_U32(write_count));

  sp_ps_t ps_a = sp_ps_create((sp_ps_config_t) {
    .command = get_process_path(),
    .args = {
      SP_LIT("--fn"), SP_LIT("pattern"),
      SP_LIT("--stdout"),
      SP_LIT("-c"), SP_LIT("A"),
      SP_LIT("-s"), size_str,
      SP_LIT("-n"), count_str,
    },
    .io = {
      .in  = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_EXISTING, .fd = pipes[1] },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  sp_ps_t ps_b = sp_ps_create((sp_ps_config_t) {
    .command = get_process_path(),
    .args = {
      SP_LIT("--fn"), SP_LIT("pattern"),
      SP_LIT("--stdout"),
      SP_LIT("-c"), SP_LIT("B"),
      SP_LIT("-s"), size_str,
      SP_LIT("-n"), count_str,
    },
    .io = {
      .in  = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_EXISTING, .fd = pipes[1] },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  SP_ASSERT(ps_a.pid != 0);
  SP_ASSERT(ps_b.pid != 0);

  close(pipes[1]);

  u32 expected = write_size * write_count * 2;
  u8* buffer = (u8*)sp_alloc(expected);
  u32 total = 0;

  while (total < expected) {
    ssize_t n = read(pipes[0], buffer + total, expected - total);
    if (n <= 0) break;
    total += n;
  }

  sp_ps_wait(&ps_a);
  sp_ps_wait(&ps_b);
  close(pipes[0]);

  analysis_t a = analyze(buffer, total, write_size);

  const c8* status = a.interleaved ? "INTERLEAVED" : "atomic";
  SP_LOG("{}: {}B x {} => {} trans, {}", 
    SP_FMT_CSTR(label),
    SP_FMT_U32(write_size),
    SP_FMT_U32(write_count),
    SP_FMT_U32(a.transitions),
    SP_FMT_CSTR(status));

  if (a.interleaved) {
    sp_for(i, total - 1) {
      if (buffer[i] != buffer[i+1]) {
        u32 boundary = (i + 1) % 4096;
        SP_LOG("  first transition at byte {}, offset from 4096 boundary: {}", 
          SP_FMT_U32(i), SP_FMT_U32(boundary));
        break;
      }
    }
  }
}

s32 main(void) {
  SP_LOG("Two sp_ps subprocesses sharing stdout fd via SP_PS_IO_MODE_EXISTING");
  SP_LOG("Each writes pattern of 'A' or 'B' repeatedly");
  SP_LOG("Checking if writes interleave mid-syscall");
  SP_LOG("");

  sp_for(i, 5) {
    SP_LOG("--- run {} ---", SP_FMT_U32(i));
    run_test("< PIPE_BUF", 100, 100);
    run_test("= PIPE_BUF", 4096, 50);
    run_test("> PIPE_BUF", 8192, 50);
    run_test(">> PIPE_BUF", 65536, 20);
    SP_LOG("");
  }

  return 0;
}
