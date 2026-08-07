#define SP_IMPLEMENTATION
#include "sp.h"

#define PERF_FILE_SIZE_MB 256u
#define PERF_NAIVE_BUFFER 4096u

typedef enum {
  SRC_FILE,
  SRC_STREAM_FILE,
} src_t;

typedef enum {
  SINK_FILE,
  SINK_PIPE,
} sink_t;

typedef struct {
  u64 bytes;
  u64 ns;
} run_t;

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

#if defined(SP_LINUX)
static s32 drain_pipe(void* userdata) {
  sp_sys_fd_t fd = (sp_sys_fd_t)(uintptr_t)userdata;
  u8 buf [1u << 16];
  while (true) {
    u64 n = 0;
    if (sp_sys_read(fd, buf, sizeof(buf), &n) != SP_OK || !n) break;
  }
  sp_sys_close(fd);
  return 0;
}

static s32 open_blocking_pipe(sp_sys_fd_t* out_r, sp_sys_fd_t* out_w) {
  sp_sys_fd_t fds [2];
  s64 rc = sp_syscall(SP_SYSCALL_NUM_PIPE2, fds, SP_SYS_LINUX_O_CLOEXEC, 0, 0, 0);
  if (rc < 0) return -1;
  *out_r = fds[0];
  *out_w = fds[1];
  return 0;
}
#endif

static run_t copy_run(sp_str_t src_path, sp_str_t dst_path, src_t src, sink_t sink, bool fast, sp_mem_t mem) {
  sp_io_file_reader_t fr = sp_zero;
  sp_io_stream_reader_t sr = sp_zero;
  sp_io_reader_t* reader = SP_NULLPTR;

  switch (src) {
    case SRC_FILE: {
      sp_io_file_reader_from_path(&fr, src_path);
      reader = &fr.base;
      break;
    }
    case SRC_STREAM_FILE: {
      sp_sys_fd_t fd = SP_SYS_INVALID_FD;
      if (sp_sys_open_s(sp_sys_get_root(0), src_path, SP_SYS_OPEN_MODE_RO, 0, &fd) != SP_OK) {
        return sp_zero_s(run_t);
      }
      sp_io_stream_reader_from_file(&sr, fd, SP_IO_CLOSE_MODE_AUTO);
      reader = &sr.base;
      break;
    }
  }

  sp_io_file_writer_t fw = sp_zero;
  sp_io_stream_writer_t sw = sp_zero;
  sp_io_writer_t* writer = SP_NULLPTR;
#if defined(SP_LINUX)
  sp_sys_fd_t pipe_w = SP_SYS_INVALID_FD;
  sp_thread_t drainer = sp_zero;
#endif

  switch (sink) {
    case SINK_FILE: {
      sp_io_file_writer_from_path(&fw, dst_path);
      writer = &fw.base;
      break;
    }
    case SINK_PIPE: {
#if defined(SP_LINUX)
      sp_sys_fd_t pipe_r = SP_SYS_INVALID_FD;
      open_blocking_pipe(&pipe_r, &pipe_w);
      sp_thread_init(&drainer, drain_pipe, (void*)(uintptr_t)pipe_r);
      sp_io_stream_writer_from_fd(&sw, pipe_w, SP_IO_CLOSE_MODE_NONE);
      writer = &sw.base;
#endif
      break;
    }
  }

  u8* buf = fast ? SP_NULLPTR : sp_alloc_n(mem, u8, PERF_NAIVE_BUFFER);
  u64 copied = 0;

  sp_tm_timer_t t = sp_tm_start_timer();
  if (fast) {
    sp_io_copy(writer, reader, &copied);
  }
  else {
    sp_io_copy_b(writer, reader, buf, PERF_NAIVE_BUFFER, &copied);
  }
  u64 ns = sp_tm_read_timer(&t);

  switch (src) {
    case SRC_FILE:        sp_io_file_reader_close(&fr);   break;
    case SRC_STREAM_FILE: sp_io_stream_reader_close(&sr); break;
  }

  switch (sink) {
    case SINK_FILE: {
      sp_io_file_writer_close(&fw);
      break;
    }
    case SINK_PIPE: {
#if defined(SP_LINUX)
      sp_sys_close(pipe_w);
      sp_thread_join(&drainer);
#endif
      break;
    }
  }

  return (run_t){ .bytes = copied, .ns = ns };
}

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

static void run_pair(const c8* label, sp_str_t src, sp_str_t dst, src_t src_kind, sink_t sink_kind, sp_mem_t mem) {
  sp_log("{}", sp_fmt_cstr(label));
  run_t naive = copy_run(src, dst, src_kind, sink_kind, false, mem);
  report("naive", naive);
  run_t fast = copy_run(src, dst, src_kind, sink_kind, true, mem);
  report("fast ", fast);

  if (fast.ns) {
    u64 speedup_x100 = (naive.ns * 100u) / fast.ns;
    sp_log("  speedup: {}.{}x", sp_fmt_uint(speedup_x100 / 100u), sp_fmt_uint(speedup_x100 % 100u));
  }
}

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

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

  // One untimed pass warms the page cache so the first timed run isn't
  // penalized relative to the others.
  copy_run(src, dst, SRC_FILE, SINK_FILE, false, mem);

  run_pair("file -> file (copy_file_range)", src, dst, SRC_FILE, SINK_FILE, mem);
  run_pair("stream file -> file (copy_file_range)", src, dst, SRC_STREAM_FILE, SINK_FILE, mem);
#if defined(SP_LINUX)
  run_pair("file -> pipe (sendfile)", src, dst, SRC_FILE, SINK_PIPE, mem);
  run_pair("stream file -> pipe (sendfile)", src, dst, SRC_STREAM_FILE, SINK_PIPE, mem);
#endif

  sp_fs_remove_file(src);
  sp_fs_remove_file(dst);
  return 0;
}
SP_MAIN(run)
