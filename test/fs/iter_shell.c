#include "fs.h"

#define FS_SHELL_MAX_BATCHES 4
#define FS_SHELL_MAX_PARSES 4
#define FS_SHELL_MAX_ENTRIES 4

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
  sp_err_t err;
  bool skip;
} fs_shell_parse_t;

typedef struct {
  sp_err_t err;
  fs_shell_parse_t entries [FS_SHELL_MAX_PARSES];
} fs_shell_batch_t;

typedef struct {
  const c8* entries [FS_SHELL_MAX_ENTRIES];
  sp_err_t err;
  u32 reads;
  u32 parses;
} fs_shell_expect_t;

typedef struct {
  fs_shell_batch_t batches [FS_SHELL_MAX_BATCHES];
  fs_shell_expect_t expect;
} fs_shell_test_t;

UTEST_EMPTY_FIXTURE(fs_shell)

static fs_shell_test_t* fs_shell_script;
static u32 fs_shell_reads;
static u32 fs_shell_parses;

static u32 fs_shell_batch_len(fs_shell_batch_t* batch) {
  u32 n = 0;
  sp_carr_for(batch->entries, it) {
    fs_shell_parse_t* parse = &batch->entries[it];
    if (!parse->name && !parse->skip && !parse->err) break;
    n++;
  }
  return n;
}

static sp_err_t fs_shell_mock_dir_open(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_dir_t* out) {
  *out = sp_zero_s(sp_sys_dir_t);
  return SP_OK;
}

static sp_err_t fs_shell_mock_dir_read(sp_sys_dir_t* dir, sp_mem_buffer_t* buf) {
  buf->len = 0;
  u32 index = fs_shell_reads++;
  if (index >= FS_SHELL_MAX_BATCHES) return SP_OK;

  fs_shell_batch_t* batch = &fs_shell_script->batches[index];
  if (batch->err) return batch->err;
  buf->len = fs_shell_batch_len(batch);
  return SP_OK;
}

static sp_err_t fs_shell_mock_dir_parse(sp_sys_dir_t* dir, sp_mem_buffer_t* buf, u64* cursor, sp_sys_dir_entry_t* out) {
  *out = sp_zero_s(sp_sys_dir_entry_t);
  fs_shell_parses++;

  fs_shell_batch_t* batch = &fs_shell_script->batches[fs_shell_reads - 1];
  fs_shell_parse_t* parse = &batch->entries[*cursor];
  *cursor += 1;

  if (parse->err) return parse->err;
  if (parse->skip) return SP_OK;

  out->name = parse->name;
  out->len = (u32)sp_cstr_len(parse->name);
  out->kind = parse->kind;
  return SP_OK;
}

static sp_err_t fs_shell_mock_dir_close(sp_sys_dir_t* dir) {
  return SP_OK;
}

static void run_fs_shell_test(s32* utest_result, fs_shell_test_t t) {
  static sp_sys_vtable_t vt;
  vt = sp_sys_vtable_platform;
  vt.dir_open = fs_shell_mock_dir_open;
  vt.dir_read = fs_shell_mock_dir_read;
  vt.dir_parse = fs_shell_mock_dir_parse;
  vt.dir_close = fs_shell_mock_dir_close;

  fs_shell_script = &t;
  fs_shell_reads = 0;
  fs_shell_parses = 0;

  const sp_sys_vtable_t* old = sp_sys_set_vtable(&vt);

  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_t iter = sp_zero;
  EXPECT_EQ(sp_fs_dir_open(&iter, sp_sys_get_root(0), sp_str_lit("T"), sp_mem_slice(buf, sizeof(buf))), SP_OK);

  u32 produced = 0;
  sp_err_t walk = SP_OK;
  while (true) {
    sp_fs_dir_entry_t entry = sp_zero;
    walk = sp_fs_dir_next(&iter, &entry);
    if (walk != SP_OK) break;
    if (!entry.name.data) break;

    if (produced < FS_SHELL_MAX_ENTRIES && t.expect.entries[produced]) {
      if (!sp_str_equal_cstr(entry.name, t.expect.entries[produced])) {
        SP_TEST_REPORT("entry {} was {} but expected {}", sp_fmt_uint(produced), sp_fmt_str(entry.name), sp_fmt_cstr(t.expect.entries[produced]));
        SP_FAIL();
      }
    }
    else {
      SP_TEST_REPORT("iterator produced unexpected entry {}", sp_fmt_str(entry.name));
      SP_FAIL();
    }
    produced++;
  }

  EXPECT_EQ(walk, t.expect.err);

  u32 expected = 0;
  sp_carr_for(t.expect.entries, it) {
    if (!t.expect.entries[it]) break;
    expected++;
  }
  EXPECT_EQ(produced, expected);
  EXPECT_EQ(fs_shell_reads, t.expect.reads);
  EXPECT_EQ(fs_shell_parses, t.expect.parses);

  sp_fs_dir_close(&iter);
  sp_sys_set_vtable(old);
}

UTEST_F(fs_shell, refills_on_empty_parse) {
  run_fs_shell_test(&ur, (fs_shell_test_t) {
    .batches = {
      { .entries = { { .skip = true } } },
      { .entries = { { "A", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = { "A" },
      .reads = 3,
      .parses = 2,
    },
  });
}

UTEST_F(fs_shell, skips_dots) {
  run_fs_shell_test(&ur, (fs_shell_test_t) {
    .batches = {
      { .entries = { { "." }, { ".." }, { "A", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = { "A" },
      .reads = 2,
      .parses = 3,
    },
  });
}

UTEST_F(fs_shell, propagates_read_error) {
  run_fs_shell_test(&ur, (fs_shell_test_t) {
    .batches = {
      { .err = SP_ERR_SYS_BAD_FD },
    },
    .expect = {
      .err = SP_ERR_SYS_BAD_FD,
      .reads = 1,
    },
  });
}

UTEST_F(fs_shell, propagates_parse_error) {
  run_fs_shell_test(&ur, (fs_shell_test_t) {
    .batches = {
      { .entries = { { .err = SP_ERR_SYS_INVALID } } },
    },
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .reads = 1,
      .parses = 1,
    },
  });
}
