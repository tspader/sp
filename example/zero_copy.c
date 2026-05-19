#define SP_IMPLEMENTATION
#include "sp.h"

#define PERF_FILE_SIZE_MB 256u
#define PERF_NAIVE_BUFFER 4096u

static void fill_random(u8* p, u64 n, u64 seed) {
  u64 s = seed ? seed : 1;
  for (u64 i = 0; i < n; i++) {
    s = s * 6364136223846793005ull + 1442695040888963407ull;
    p[i] = (u8)(s >> 56);
  }
}

static sp_err_t make_source(sp_str_t path, u64 size_bytes, sp_mem_t mem) {
  u8* chunk = sp_alloc_n(mem, u8, 1u << 20);
  fill_random(chunk, 1u << 20, 0xdeadbeef69420694ull);

  sp_io_file_writer_t w = sp_zero;
  sp_try(sp_io_file_writer_from_path(&w, path));

  u64 remaining = size_bytes;
  while (remaining) {
    u64 want = remaining < (1u << 20) ? remaining : (1u << 20);
    sp_try(sp_io_write(&w.base, chunk, want, SP_NULLPTR));
    remaining -= want;
  }
  return sp_io_file_writer_close(&w);
}

typedef struct {
  u64 bytes;
  u64 ns;
} run_t;

static run_t copy_naive(sp_str_t src, sp_str_t dst, sp_mem_t mem) {
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_reader_from_path(&r, src);
  sp_io_file_writer_from_path(&w, dst);

  u8* buf = sp_alloc_n(mem, u8, PERF_NAIVE_BUFFER);
  u64 copied = 0;

  sp_tm_timer_t t = sp_tm_start_timer();
  sp_io_copy_b(&w.base, &r.base, buf, PERF_NAIVE_BUFFER, &copied);
  u64 ns = sp_tm_read_timer(&t);

  sp_io_file_reader_close(&r);
  sp_io_file_writer_close(&w);
  return (run_t){ .bytes = copied, .ns = ns };
}

static run_t copy_fast(sp_str_t src, sp_str_t dst) {
  sp_io_file_reader_t r = sp_zero;
  sp_io_file_writer_t w = sp_zero;
  sp_io_file_reader_from_path(&r, src);
  sp_io_file_writer_from_path(&w, dst);

  u64 copied = 0;
  sp_tm_timer_t t = sp_tm_start_timer();
  sp_io_copy(&w.base, &r.base, &copied);
  u64 ns = sp_tm_read_timer(&t);

  sp_io_file_reader_close(&r);
  sp_io_file_writer_close(&w);
  return (run_t){ .bytes = copied, .ns = ns };
}

#if defined(SP_LINUX)
static s32 drain_pipe(void* userdata) {
  sp_sys_fd_t fd = (sp_sys_fd_t)(uintptr_t)userdata;
  u8 buf [1u << 16];
  while (true) {
    s64 rc = sp_sys_read(fd, buf, sizeof(buf));
    if (rc <= 0) break;
  }
  sp_sys_close(fd);
  return 0;
}

static s32 open_blocking_pipe(sp_sys_fd_t* out_r, sp_sys_fd_t* out_w) {
  sp_sys_fd_t fds [2];
  s64 rc = sp_syscall(SP_SYSCALL_NUM_PIPE2, fds, SP_O_CLOEXEC, 0, 0, 0);
  if (rc < 0) return -1;
  *out_r = fds[0];
  *out_w = fds[1];
  return 0;
}

static run_t copy_naive_pipe(sp_str_t src, sp_mem_t mem) {
  sp_sys_fd_t pipe_r = SP_SYS_INVALID_FD;
  sp_sys_fd_t pipe_w = SP_SYS_INVALID_FD;
  open_blocking_pipe(&pipe_r, &pipe_w);

  sp_thread_t drainer = sp_zero;
  sp_thread_init(&drainer, drain_pipe, (void*)(uintptr_t)pipe_r);

  sp_io_file_reader_t r = sp_zero;
  sp_io_stream_writer_t w = sp_zero;
  sp_io_file_reader_from_path(&r, src);
  sp_io_stream_writer_from_fd(&w, pipe_w, SP_IO_CLOSE_MODE_NONE);

  u8* buf = sp_alloc_n(mem, u8, PERF_NAIVE_BUFFER);
  u64 copied = 0;

  sp_tm_timer_t t = sp_tm_start_timer();
  sp_io_copy_b(&w.base, &r.base, buf, PERF_NAIVE_BUFFER, &copied);
  u64 ns = sp_tm_read_timer(&t);

  sp_io_file_reader_close(&r);
  sp_sys_close(pipe_w);
  sp_thread_join(&drainer);
  return (run_t){ .bytes = copied, .ns = ns };
}

static run_t copy_fast_pipe(sp_str_t src) {
  sp_sys_fd_t pipe_r = SP_SYS_INVALID_FD;
  sp_sys_fd_t pipe_w = SP_SYS_INVALID_FD;
  open_blocking_pipe(&pipe_r, &pipe_w);

  sp_thread_t drainer = sp_zero;
  sp_thread_init(&drainer, drain_pipe, (void*)(uintptr_t)pipe_r);

  sp_io_file_reader_t r = sp_zero;
  sp_io_stream_writer_t w = sp_zero;
  sp_io_file_reader_from_path(&r, src);
  sp_io_stream_writer_from_fd(&w, pipe_w, SP_IO_CLOSE_MODE_NONE);

  u64 copied = 0;
  sp_tm_timer_t t = sp_tm_start_timer();
  sp_io_copy(&w.base, &r.base, &copied);
  u64 ns = sp_tm_read_timer(&t);

  sp_io_file_reader_close(&r);
  sp_sys_close(pipe_w);
  sp_thread_join(&drainer);
  return (run_t){ .bytes = copied, .ns = ns };
}
#endif

static void report(const c8* label, run_t run) {
  // MB/s = bytes / 1e6 / (ns / 1e9) = bytes * 1000 / ns.
  u64 mb_per_s = run.ns ? (run.bytes * 1000u) / run.ns : 0;
  sp_log(
    "  {}: {} bytes in {} us ({} MB/s)",
    sp_fmt_cstr(label),
    sp_fmt_uint(run.bytes),
    sp_fmt_uint(run.ns / 1000u),
    sp_fmt_uint(mb_per_s)
  );
}

s32 run(s32 num_args, const c8** args) {
  (void)num_args; (void)args;
  sp_mem_t mem = sp_mem_arena_as_allocator(sp_mem_arena_new(sp_mem_os_new()));

  sp_str_t cwd = sp_fs_get_cwd(mem);
  sp_str_t src = sp_str_concat(mem, cwd, sp_str_lit("/io.src"));
  sp_str_t dst = sp_str_concat(mem, cwd, sp_str_lit("/io.dst"));

  u64 size_bytes = (u64)PERF_FILE_SIZE_MB * 1024u * 1024u;
  sp_log("preparing {} MiB source at {}", sp_fmt_uint(PERF_FILE_SIZE_MB), sp_fmt_str(src));
  sp_err_t err = make_source(src, size_bytes, mem);
  if (err) {
    sp_log("failed to prepare source: {}", sp_fmt_uint((u32)err));
    return 1;
  }

  // Run it twice to at least pretend to warm up the IO, but the amount that
  // going kernel -> kernel saves us overshadows any caching
  sp_log("file -> file (copy_file_range)");
  run_t n1 = copy_naive(src, dst, mem); report("naive #1", n1);
  run_t f1 = copy_fast (src, dst);      report("fast  #1", f1);
  run_t n2 = copy_naive(src, dst, mem); report("naive #2", n2);
  run_t f2 = copy_fast (src, dst);      report("fast  #2", f2);

  if (f2.ns) {
    u64 speedup_x100 = (n2.ns * 100u) / f2.ns;
    sp_log(
      "warm-cache speedup: {}.{}x  (naive #2 / fast #2)",
      sp_fmt_uint(speedup_x100 / 100u),
      sp_fmt_uint(speedup_x100 % 100u)
    );
  }

#if defined(SP_LINUX)
  sp_log("file -> pipe (sendfile)");
  run_t pn1 = copy_naive_pipe(src, mem); report("naive #1", pn1);
  run_t pf1 = copy_fast_pipe(src); report("fast  #1", pf1);
  run_t pn2 = copy_naive_pipe(src, mem); report("naive #2", pn2);
  run_t pf2 = copy_fast_pipe(src); report("fast  #2", pf2);

  if (pf2.ns) {
    u64 speedup_x100 = (pn2.ns * 100u) / pf2.ns;
    sp_log(
      "warm-cache speedup: {}.{}x  (naive #2 / fast #2)",
      sp_fmt_uint(speedup_x100 / 100u),
      sp_fmt_uint(speedup_x100 % 100u)
    );
  }
#endif

  sp_fs_remove_file(src);
  sp_fs_remove_file(dst);
  return 0;
}
SP_MAIN(run)
