#include "sp.h"
#include "sp/sp_test.h"

#define MAX_BATCHES 4
#define MAX_PARSES 4
#define MAX_ENTRIES 8
#define FD ((sp_sys_fd_t)7777)

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
  sp_err_t err;
  bool skip;
} parse_t;

typedef struct {
  sp_err_t err;
  parse_t entries [MAX_PARSES];
} batch_t;

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  sp_err_t open;
  sp_err_t walk;
  entry_t entries [MAX_ENTRIES];
  u32 opens;
  u32 from_fds;
  u32 reads;
  u32 parses;
  u32 fd_closes;
} expect_t;

typedef struct {
  const c8* name;
  u64 cap;
  sp_err_t open_err;
  sp_err_t from_fd_err;
  batch_t batches [MAX_BATCHES];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "lists_entries_with_kinds",
    .batches = {
      { .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_DIR }, { "L", SP_FS_KIND_SYMLINK } } },
    },
    .expect = {
      .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_DIR }, { "L", SP_FS_KIND_SYMLINK } },
      .opens = 1,
      .from_fds = 1,
      .reads = 2,
      .parses = 3,
    },
  },
  {
    .name = "empty_read_ends_iteration",
    .expect = {
      .opens = 1,
      .from_fds = 1,
      .reads = 1,
    },
  },
  {
    .name = "skips_dots",
    .batches = {
      { .entries = { { "." }, { ".." }, { "A", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = { { "A", SP_FS_KIND_FILE } },
      .opens = 1,
      .from_fds = 1,
      .reads = 2,
      .parses = 3,
    },
  },
  {
    .name = "refills_on_empty_parse",
    .batches = {
      { .entries = { { .skip = true } } },
      { .entries = { { "A", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = { { "A", SP_FS_KIND_FILE } },
      .opens = 1,
      .from_fds = 1,
      .reads = 3,
      .parses = 2,
    },
  },
  {
    .name = "produces_entries_across_batches",
    .batches = {
      { .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_DIR } } },
      { .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = { { "A", SP_FS_KIND_FILE }, { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } },
      .opens = 1,
      .from_fds = 1,
      .reads = 3,
      .parses = 3,
    },
  },
  {
    .name = "propagates_read_error",
    .batches = {
      { .err = SP_ERR_SYS_BAD_FD },
    },
    .expect = {
      .walk = SP_ERR_SYS_BAD_FD,
      .opens = 1,
      .from_fds = 1,
      .reads = 1,
    },
  },
  {
    .name = "propagates_parse_error",
    .batches = {
      { .entries = { { .err = SP_ERR_SYS_INVALID } } },
    },
    .expect = {
      .walk = SP_ERR_SYS_INVALID,
      .opens = 1,
      .from_fds = 1,
      .reads = 1,
      .parses = 1,
    },
  },
  {
    .name = "propagates_open_error",
    .open_err = SP_ERR_SYS_NOT_FOUND,
    .expect = {
      .open = SP_ERR_SYS_NOT_FOUND,
      .opens = 1,
    },
  },
  {
    .name = "closes_fd_when_from_fd_fails",
    .from_fd_err = SP_ERR_SYS_IO,
    .expect = {
      .open = SP_ERR_SYS_IO,
      .opens = 1,
      .from_fds = 1,
      .fd_closes = 1,
    },
  },
  {
    .name = "refuses_undersized_buffer",
    .cap = SP_SYS_DIR_MIN_BUF - 1,
    .expect = {
      .open = SP_ERR_SYS_BUG,
    },
  },
};

typedef struct {
  u32 opens;
  u32 from_fds;
  u32 reads;
  u32 parses;
  u32 fd_closes;
  u32 dir_closes;
} count_t;

static const test_t* test;
static count_t count;

static u32 get_batch_len(const batch_t* batch) {
  u32 n = 0;
  sp_carr_for(batch->entries, it) {
    const parse_t* parse = &batch->entries[it];
    if (!parse->name && !parse->skip && !parse->err) break;
    n++;
  }
  return n;
}

static sp_err_t open_dir(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_fd_t* out) {
  count.opens++;
  if (test->open_err) return test->open_err;
  *out = FD;
  return SP_OK;
}

static sp_err_t close_fd(sp_sys_fd_t fd) {
  if (fd == FD) count.fd_closes++;
  return SP_OK;
}

static sp_err_t dir_from_fd(sp_sys_fd_t fd, sp_sys_dir_t* out) {
  count.from_fds++;
  if (test->from_fd_err) return test->from_fd_err;
  *out = sp_zero_s(sp_sys_dir_t);
  out->handle = (s64)fd;
  return SP_OK;
}

static sp_err_t dir_read(sp_sys_dir_t* dir, sp_mem_buffer_t* buf) {
  buf->len = 0;
  u32 index = count.reads++;
  if (index >= MAX_BATCHES) return SP_OK;

  const batch_t* batch = &test->batches[index];
  if (batch->err) return batch->err;
  buf->len = get_batch_len(batch);
  return SP_OK;
}

static sp_err_t dir_parse(sp_sys_dir_t* dir, sp_mem_buffer_t* buf, u64* cursor, sp_sys_dir_entry_t* out) {
  *out = sp_zero_s(sp_sys_dir_entry_t);
  count.parses++;

  const batch_t* batch = &test->batches[count.reads - 1];
  const parse_t* parse = &batch->entries[*cursor];
  *cursor += 1;

  if (parse->err) return parse->err;
  if (parse->skip) return SP_OK;

  out->name = parse->name;
  out->len = (u32)sp_cstr_len(parse->name);
  out->kind = parse->kind;
  return SP_OK;
}

static sp_err_t dir_close(sp_sys_dir_t* dir) {
  count.dir_closes++;
  return SP_OK;
}


sp_test_each(fs, iter_shell, test_t, tests, .serial = true) {
  static sp_sys_vtable_t vt;
  vt = sp_sys_vtable_platform;
  vt.open_dir = open_dir;
  vt.close = close_fd;
  vt.dir_from_fd = dir_from_fd;
  vt.dir_read = dir_read;
  vt.dir_parse = dir_parse;
  vt.dir_close = dir_close;

  test = it;
  count = sp_zero_s(count_t);

  const sp_sys_vtable_t* old = sp_sys_set_vtable(&vt);

  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  u64 cap = it->cap ? it->cap : sizeof(buf);

  sp_fs_dir_t iter = sp_zero;
  sp_err_t open_err = sp_fs_dir_open(&iter, sp_sys_get_root(0), sp_str_lit("T"), sp_mem_slice(buf, cap));
  sp_expect_err_eq(t, open_err, it->expect.open);

  u32 produced = 0;
  if (!open_err) {
    sp_err_t walk = SP_OK;
    while (true) {
      sp_fs_dir_entry_t entry = sp_zero;
      walk = sp_fs_dir_next(&iter, &entry);
      if (walk) break;
      if (!entry.name.data) break;

      if (produced < MAX_ENTRIES && it->expect.entries[produced].name) {
        sp_expect_str_eq_c(t, entry.name, it->expect.entries[produced].name);
        sp_expect_eq(t, (u32)entry.kind, (u32)it->expect.entries[produced].kind);
      }
      else {
        sp_test_fail(t, "iterator produced unexpected entry {}", sp_fmt_str(entry.name));
      }
      produced++;
    }
    sp_expect_err_eq(t, walk, it->expect.walk);
    sp_fs_dir_close(&iter);
  }

  sp_sys_set_vtable(old);

  u32 expected = 0;
  sp_carr_for(it->expect.entries, e) {
    if (!it->expect.entries[e].name) break;
    expected++;
  }
  sp_expect_eq(t, produced, expected);
  sp_expect_eq(t, count.opens, it->expect.opens);
  sp_expect_eq(t, count.from_fds, it->expect.from_fds);
  sp_expect_eq(t, count.reads, it->expect.reads);
  sp_expect_eq(t, count.parses, it->expect.parses);
  sp_expect_eq(t, count.fd_closes, it->expect.fd_closes);
  sp_expect_eq(t, count.dir_closes, open_err ? (u32)0 : (u32)1);
  return SP_OK;
}
