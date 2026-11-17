#include "fs.h"

typedef enum {
  OP_WRITE,
  OP_COPY,
} op_t;

typedef struct {
  const c8* name;
  op_t op;
} test_t;

static const test_t tests [] = {
  { .name = "write" },
  { .name = "copy", .op = OP_COPY },
};

typedef struct {
  u32 temps;
  u32 read_only;
  u32 sets;
} probe_t;

static probe_t probe;

static sp_err_t probed_open(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_open_mode_t mode, u32 flags, sp_sys_file_perms_t perms, sp_sys_fd_t* fd) {
  if (flags & SP_SYS_OPEN_EXCLUSIVE) {
    probe.temps++;
    if (sp_sys_is_read_only(perms)) probe.read_only++;
  }
  return sp_sys_open_p(dir, path, len, mode, flags, perms, fd);
}

static sp_err_t probed_set_file_perms(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_perms_t perms) {
  probe.sets++;
  return sp_sys_set_file_perms_p(dir, path, len, perms);
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_test_skip_on_wasm();
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));

  sp_sys_file_perms_t read_only = sp_sys_default_file_perms;
  sp_sys_set_read_only(&read_only, true);
  sp_must_ok(t, sp_fs_write_at(from, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, read_only));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.open = probed_open;
  vt.set_file_perms = probed_set_file_perms;
  probe = sp_zero_s(probe_t);

  sp_err_t err = SP_OK;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  switch (c->op) {
    case OP_WRITE: err = sp_fs_write_at(to, sp_str_lit("A"), SP_FS_WRITE_REPLACE, read_only); break;
    case OP_COPY:  err = sp_fs_copy_file_at(from, to, SP_FS_WRITE_REPLACE); break;
  }
  sp_sys_set_vtable(saved);

  sp_expect_ok(t, err);
  sp_expect_eq(t, probe.temps, 1);
  sp_expect_eq(t, probe.read_only, 1);
  sp_expect_eq(t, probe.sets, 0);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each_fn(fs, temp_perms, test_t, tests, run, .serial = true);
