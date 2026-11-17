#include "fs.h"

typedef struct {
  const c8* name;
  sp_fs_write_mode_t mode;
} test_t;

static const test_t tests [] = {
  { .name = "truncate" },
  { .name = "append", .mode = SP_FS_WRITE_APPEND },
};

static sp_err_t unseekable_pwrite(sp_sys_fd_t fd, const void* buf, u64 count, u64 offset, u64* bytes_written) {
  if (bytes_written) *bytes_written = 0;
  return SP_ERR_SYS_UNSEEKABLE;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t path = sp_path_join(mem, sp_test_dir(t), sp_str_lit("A"));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.pwrite = unseekable_pwrite;

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_write_at(path, sp_str_lit("A"), c->mode, sp_sys_default_file_perms);
  sp_sys_set_vtable(saved);
  sp_expect_ok(t, err);

  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read_at(mem, path, &content));
  sp_expect_str_eq(t, content, sp_str_lit("A"));
  return SP_OK;
}

sp_test_each_fn(fs, write_stream, test_t, tests, run, .serial = true);
