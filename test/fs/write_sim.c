#include "fs.h"

typedef struct {
  u32 perms;
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  sp_fs_write_mode_t mode;
  bool unseekable;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "truncate_unseekable",
    .unseekable = true,
  },
  {
    .name = "append_unseekable",
    .mode = SP_FS_WRITE_APPEND,
    .unseekable = true,
  },
  {
    .name = "replace_sets_perms_before_content",
    .setup = {
      { .path = "A", .content = "B" },
    },
    .mode = SP_FS_WRITE_REPLACE,
    .expect = {
      .perms = 1,
    },
  },
};

typedef struct {
  u32 perms;
  u64 exposed;
} probe_t;

static probe_t probe;

static sp_err_t unseekable_pwrite(sp_sys_fd_t fd, const void* buf, u64 count, u64 offset, u64* bytes_written) {
  if (bytes_written) *bytes_written = 0;
  return SP_ERR_SYS_UNSEEKABLE;
}

static sp_err_t perms_probe(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_perms_t perms) {
  sp_sys_file_meta_t meta = sp_zero;
  sp_sys_get_path_metadata_p(dir, path, len, &meta);
  probe.perms++;
  probe.exposed = (u64)meta.size;
  return sp_sys_set_file_perms_p(dir, path, len, perms);
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, c->setup));

  sp_path_t path = sp_path_join(mem, sandbox, sp_str_lit("A"));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.set_file_perms = perms_probe;
  if (c->unseekable) vt.pwrite = unseekable_pwrite;
  probe = sp_zero_s(probe_t);

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_write_at(path, sp_str_lit("A"), c->mode);
  sp_sys_set_vtable(saved);

  sp_expect_ok(t, err);
  sp_expect_eq(t, probe.perms, c->expect.perms);
  sp_expect_eq(t, probe.exposed, 0);

  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read_at(mem, path, &content));
  sp_expect_str_eq(t, content, sp_str_lit("A"));
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each_fn(fs, write_sim, test_t, tests, run, .serial = true);
