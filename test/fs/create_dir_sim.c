#include "sp.h"
#include "sp/sp_test.h"

#define MAX_MKDIRS 64

typedef struct {
  const c8* name;
  const c8* target;
} test_t;

static const test_t tests [] = {
  { .name = "bare",            .target = "A" },
  { .name = "nested",          .target = "A/B" },
  { .name = "under_root",      .target = "/A" },
  { .name = "dot",             .target = "." },
  { .name = "root",            .target = "/" },
  { .name = "empty",           .target = "" },
};

static u32 mkdirs;

static sp_err_t mkdir_missing(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_perms_t perms) {
  mkdirs++;
  return mkdirs > MAX_MKDIRS ? SP_ERR_SYS_LOOP : SP_ERR_SYS_NOT_FOUND;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.mkdir = mkdir_missing;

  mkdirs = 0;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_create_dir_at(sp_path_at_cwd(sp_cstr_as_str(c->target)));
  sp_sys_set_vtable(saved);

  sp_expect_err_eq(t, err, SP_ERR_SYS_NOT_FOUND);
  return SP_OK;
}

sp_test_each_fn(fs, create_dir_sim, test_t, tests, run, .serial = true);
