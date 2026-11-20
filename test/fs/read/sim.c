#include "fs.h"

typedef struct {
  sp_err_t err;
  const c8* content;
} expect_t;

typedef struct {
  const c8* name;
  const c8* content;
  s64 size;
  sp_err_t metadata;
  sp_err_t read;
  u32 failing_alloc;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "size_zero",
    .content = "AB",
    .expect = {
      .content = "AB",
    },
  },
  {
    .name = "size_exact",
    .content = "AB",
    .size = 2,
    .expect = {
      .content = "AB",
    },
  },
  {
    .name = "size_overstated",
    .content = "A",
    .size = 4,
    .expect = {
      .content = "A",
    },
  },
  {
    .name = "size_understated",
    .content = "AB",
    .size = 1,
    .expect = {
      .content = "AB",
    },
  },
  {
    .name = "size_unavailable",
    .content = "AB",
    .metadata = SP_ERR_SYS_IO,
    .expect = {
      .content = "AB",
    },
  },
  {
    .name = "read_failure",
    .content = "AB",
    .size = 2,
    .read = SP_ERR_SYS_IO,
    .expect = {
      .err = SP_ERR_SYS_IO,
    },
  },
  {
    .name = "first_allocation_fails",
    .content = "AB",
    .size = 2,
    .failing_alloc = 1,
    .expect = {
      .err = SP_ERR_SYS_NO_MEMORY,
    },
  },
  {
    .name = "growing_allocation_fails",
    .content = "AB",
    .size = 1,
    .failing_alloc = 2,
    .expect = {
      .err = SP_ERR_SYS_NO_MEMORY,
    },
  },
};

typedef struct {
  s64 size;
  sp_err_t metadata;
  sp_err_t read;
} probe_t;

typedef struct {
  sp_mem_t backing;
  u32 failing;
  u32 allocs;
} budget_t;

static probe_t probe;

static sp_err_t budgeted_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr, u64 old_size, void** out) {
  budget_t* budget = (budget_t*)user_data;
  switch (mode) {
    case SP_ALLOCATOR_MODE_FREE: {
      break;
    }
    case SP_ALLOCATOR_MODE_ALLOC:
    case SP_ALLOCATOR_MODE_RESIZE:
    case SP_ALLOCATOR_MODE_ALLOC_UNINITIALIZED:
    case SP_ALLOCATOR_MODE_RESIZE_UNINITIALIZED: {
      if (++budget->allocs == budget->failing) {
        *out = SP_NULLPTR;
        return SP_ERR_SYS_NO_MEMORY;
      }
      break;
    }
  }
  return budget->backing.on_alloc(budget->backing.user_data, mode, size, ptr, old_size, out);
}

static sp_err_t sized_metadata(sp_sys_fd_t fd, sp_sys_file_meta_t* meta) {
  if (probe.metadata) return probe.metadata;
  sp_try(sp_sys_get_file_metadata_p(fd, meta));
  meta->size = probe.size;
  return SP_OK;
}

static sp_err_t probed_read(sp_sys_fd_t fd, void* buf, u64 count, u64* bytes_read) {
  if (probe.read) {
    if (bytes_read) *bytes_read = 0;
    return probe.read;
  }
  return sp_sys_read_p(fd, buf, count, bytes_read);
}

static sp_err_t unseekable_pread(sp_sys_fd_t fd, void* buf, u64 count, u64 offset, u64* bytes_read) {
  if (bytes_read) *bytes_read = 0;
  return SP_ERR_SYS_UNSEEKABLE;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t path = sp_path_join(mem, sp_test_dir(t), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_write_cstr_at(path, c->content, SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_file_metadata = sized_metadata;
  vt.read = probed_read;
  vt.pread = unseekable_pread;
  probe = (probe_t) { .size = c->size, .metadata = c->metadata, .read = c->read };

  sp_test_tracking_t tracking = sp_zero;
  sp_test_tracking_init(&tracking, mem, mem);
  budget_t budget = { .backing = sp_test_tracking_as_allocator(&tracking), .failing = c->failing_alloc };

  sp_str_t content = sp_zero;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_read_at((sp_mem_t) { .on_alloc = budgeted_alloc, .user_data = &budget }, path, &content);
  sp_sys_set_vtable(saved);

  sp_expect_err_eq(t, err, c->expect.err);
  sp_expect_str_eq(t, content, c->expect.content ? sp_cstr_as_str(c->expect.content) : sp_str_lit(""));
  sp_expect_eq(t, tracking.live_count, err ? 0u : 1u);
  sp_test_tracking_deinit(&tracking);
  return SP_OK;
}

sp_test_each_fn(fs, read_sim, test_t, tests, run, .serial = true);
