#include "fs.h"

typedef struct {
  sp_err_t err;
  u32 len;
} expect_t;

typedef struct {
  const c8* name;
  u64 size;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "at_limit", .size = SP_LIMIT_U32_MAX, .expect = { .len = SP_LIMIT_U32_MAX } },
  { .name = "over_limit", .size = (u64)SP_LIMIT_U32_MAX + 1, .expect = { .err = SP_ERR_SYS_FILE_TOO_BIG } },
};

typedef struct {
  u64 size;
  u64 remaining;
} probe_t;

static probe_t probe;
static u8 block [16];

static sp_err_t sized_metadata(sp_sys_fd_t fd, sp_sys_file_meta_t* meta) {
  sp_try(sp_sys_get_file_metadata_p(fd, meta));
  meta->size = (s64)probe.size;
  return SP_OK;
}

static sp_err_t counted_read(sp_sys_fd_t fd, void* buf, u64 count, u64* bytes_read) {
  u64 n = sp_min(count, probe.remaining);
  probe.remaining -= n;
  if (bytes_read) *bytes_read = n;
  return SP_OK;
}

static sp_err_t unseekable_pread(sp_sys_fd_t fd, void* buf, u64 count, u64 offset, u64* bytes_read) {
  if (bytes_read) *bytes_read = 0;
  return SP_ERR_SYS_UNSEEKABLE;
}

static void* unbacked_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr, u64 old_size) {
  switch (mode) {
    case SP_ALLOCATOR_MODE_FREE: {
      return SP_NULLPTR;
    }
    case SP_ALLOCATOR_MODE_ALLOC:
    case SP_ALLOCATOR_MODE_RESIZE:
    case SP_ALLOCATOR_MODE_ALLOC_UNINITIALIZED:
    case SP_ALLOCATOR_MODE_RESIZE_UNINITIALIZED: {
      return block;
    }
  }
  return SP_NULLPTR;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_path_t path = sp_path_join(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_write_at(path, sp_str_lit(""), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_file_metadata = sized_metadata;
  vt.read = counted_read;
  vt.pread = unseekable_pread;
  probe = (probe_t) { .size = c->size, .remaining = c->size };

  sp_str_t content = sp_zero;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_read_at((sp_mem_t) { .on_alloc = unbacked_alloc }, path, &content);
  sp_sys_set_vtable(saved);

  sp_expect_err_eq(t, err, c->expect.err);
  sp_expect_eq(t, content.len, c->expect.len);
  return SP_OK;
}

sp_test_each_fn(fs, read_limit, test_t, tests, run, .serial = true);
