#include "fs.h"

typedef struct {
  sp_err_t err;
  u32 calls;
  u64 exposed;
} probe_t;

static probe_t probe;

static sp_err_t perms_probe(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_perms_t perms) {
  sp_sys_file_meta_t meta = sp_zero;
  probe.err = sp_sys_get_path_metadata_p(dir, path, len, &meta);
  probe.calls++;
  probe.exposed = (u64)meta.size;
  return sp_sys_set_file_perms_p(dir, path, len, perms);
}

sp_test(fs, write_replace_sets_perms_before_content, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t path = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_must_ok(t, sp_fs_write_at(path, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.set_file_perms = perms_probe;
  probe = sp_zero_s(probe_t);

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_write_at(path, sp_str_lit("B"), SP_FS_WRITE_REPLACE);
  sp_sys_set_vtable(saved);

  sp_expect_ok(t, err);
  sp_expect_ok(t, probe.err);
  sp_expect_eq(t, probe.calls, 1);
  sp_expect_eq(t, probe.exposed, 0);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
