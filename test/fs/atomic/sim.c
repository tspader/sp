#include "fs.h"

#define COLLISIONS 2

static u32 creates;

static sp_err_t colliding_open(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_open_mode_t mode, u32 flags, sp_sys_file_perms_t perms, sp_sys_fd_t* fd) {
  if ((flags & SP_SYS_OPEN_EXCLUSIVE) && creates++ < COLLISIONS) return SP_ERR_SYS_EXISTS;
  return sp_sys_open_p(dir, path, len, mode, flags, perms, fd);
}

sp_test(fs, atomic_retries_temp_name_collision, .serial = true) {
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t path = sp_path_join(sp_test_arena(t), sandbox, sp_str_lit("A"));
  const fs_expected_path_t paths [FS_MAX_PATHS] = {
    { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
  };

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.open = colliding_open;
  creates = 0;

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_fs_atomic_t af = sp_zero;
  sp_err_t err = sp_fs_atomic_open_at(&af, path, sp_sys_default_file_perms);
  if (!err) err = sp_io_write_str(sp_fs_atomic_writer(&af), sp_str_lit("A"), SP_NULLPTR);
  if (!err) err = sp_fs_atomic_commit(&af, SP_FS_ATOMIC_REPLACE);
  sp_sys_set_vtable(saved);

  sp_expect_ok(t, err);
  sp_expect_eq(t, creates, COLLISIONS + 1);
  fs_expect_paths(t, sandbox, paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}
