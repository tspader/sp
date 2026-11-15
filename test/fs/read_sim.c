#include "fs.h"

typedef struct {
  const c8* content;
} expect_t;

typedef struct {
  const c8* name;
  const c8* content;
  s64 size;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "size_zero_reads_to_eof",
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
      .content = "A",
    },
  },
};

static s64 reported;

static sp_err_t sized_metadata(sp_sys_fd_t fd, sp_sys_file_meta_t* meta) {
  sp_try(sp_sys_get_file_metadata_p(fd, meta));
  meta->size = reported;
  return SP_OK;
}

static sp_err_t unseekable_pread(sp_sys_fd_t fd, void* buf, u64 count, u64 offset, u64* bytes_read) {
  if (bytes_read) *bytes_read = 0;
  return SP_ERR_SYS_UNSEEKABLE;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t path = sp_path_join(mem, sp_test_dir(t), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_write_cstr_at(path, c->content, SP_FS_WRITE_TRUNCATE));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_file_metadata = sized_metadata;
  vt.pread = unseekable_pread;
  reported = c->size;

  sp_str_t content = sp_zero;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_read_at(mem, path, &content);
  sp_sys_set_vtable(saved);

  sp_expect_ok(t, err);
  sp_expect_str_eq_c(t, content, c->expect.content);
  return SP_OK;
}

sp_test_each_fn(fs, read_sim, test_t, tests, run, .serial = true);
