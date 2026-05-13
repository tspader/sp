// Compare the userspace copy loop against the kernel-side fast path
// (copy_file_range on Linux) when copying a large file.
//
// sp_io_copy_b is the explicit-buffer form: it always loops read+write
// through the provided userspace buffer. sp_io_copy detects whether the
// (reader, writer) pair supports a kernel-to-kernel route and uses it if
// so, otherwise falling through to the generic loop.

#define SP_IMPLEMENTATION
#include "sp.h"

#define PERF_FILE_SIZE_MB 256u
#define PERF_NAIVE_BUFFER 4096u

static void fill_random(u8* p, u64 n, u64 seed) {
  // Cheap LCG. We just need bytes that vary enough that filesystems can't
  // collapse the file with sparse / dedup heuristics.
  u64 s = seed ? seed : 1;
  for (u64 i = 0; i < n; i++) {
    s = s * 6364136223846793005ull + 1442695040888963407ull;
    p[i] = (u8)(s >> 56);
  }
}

static sp_err_t make_source(sp_str_t path, u64 size_bytes, sp_mem_t mem) {
  u8* chunk = sp_alloc_n_a(mem, u8, 1u << 20);
  fill_random(chunk, 1u << 20, 0xdeadbeefcafebabeull);

  sp_io_file_writer_t w = sp_zero;
  sp_try(sp_io_file_writer_from_path(&w, path, SP_IO_WRITE_MODE_OVERWRITE));

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
  sp_io_file_writer_from_path(&w, dst, SP_IO_WRITE_MODE_OVERWRITE);

  u8* buf = sp_alloc_n_a(mem, u8, PERF_NAIVE_BUFFER);
  u64 copied = 0;

  sp_tm_timer_t t = sp_tm_start_timer();
  // sp_io_copy_b never tries the kernel-side fast path — it is the explicit
  // userspace-buffer form. Each PERF_NAIVE_BUFFER-sized chunk crosses the
  // user/kernel boundary twice (read + write).
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
  sp_io_file_writer_from_path(&w, dst, SP_IO_WRITE_MODE_OVERWRITE);

  u64 copied = 0;
  sp_tm_timer_t t = sp_tm_start_timer();
  // sp_io_copy checks for the fast path. With a file reader (exposes its fd
  // via .as_fd) and a file writer (consumes via .read_from -> copy_file_range
  // on Linux), bytes never enter userspace.
  sp_io_copy(&w.base, &r.base, &copied);
  u64 ns = sp_tm_read_timer(&t);

  sp_io_file_reader_close(&r);
  sp_io_file_writer_close(&w);
  return (run_t){ .bytes = copied, .ns = ns };
}

static void report(const c8* label, run_t run) {
  // MB/s = bytes / 1e6 / (ns / 1e9) = bytes * 1000 / ns.
  u64 mb_per_s = run.ns ? (run.bytes * 1000u) / run.ns : 0;
  sp_log_a(
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

  sp_str_t cwd = sp_fs_get_cwd_a(mem);
  sp_str_t src = sp_str_concat_a(mem, cwd, sp_str_lit("/io_copy_perf.src"));
  sp_str_t dst = sp_str_concat_a(mem, cwd, sp_str_lit("/io_copy_perf.dst"));

  u64 size_bytes = (u64)PERF_FILE_SIZE_MB * 1024u * 1024u;
  sp_log_a("preparing {} MiB source at {}", sp_fmt_uint(PERF_FILE_SIZE_MB), sp_fmt_str(src));
  sp_err_t err = make_source(src, size_bytes, mem);
  if (err) {
    sp_log_a("failed to prepare source: err={}", sp_fmt_uint((u32)err));
    return 1;
  }

  // Two trials of each so we can see cold-vs-warm cache effects. The first
  // naive run typically pays for cold reads of the source; the second runs
  // against a warm page cache. The fast-path run benefits from a warm cache
  // too, but the dominant cost it skips is the userspace copy itself.
  sp_log_a("running benchmark (two trials each)");
  run_t n1 = copy_naive(src, dst, mem); report("naive #1", n1);
  run_t f1 = copy_fast (src, dst);      report("fast  #1", f1);
  run_t n2 = copy_naive(src, dst, mem); report("naive #2", n2);
  run_t f2 = copy_fast (src, dst);      report("fast  #2", f2);

  if (f2.ns) {
    u64 speedup_x100 = (n2.ns * 100u) / f2.ns;
    sp_log_a(
      "warm-cache speedup: {}.{}x  (naive #2 / fast #2)",
      sp_fmt_uint(speedup_x100 / 100u),
      sp_fmt_uint(speedup_x100 % 100u)
    );
  }

  sp_fs_remove_file_a(src);
  sp_fs_remove_file_a(dst);
  return 0;
}
SP_MAIN(run)
