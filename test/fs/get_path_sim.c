#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  PATH_CWD,
  PATH_EXE,
  PATH_STORAGE,
  PATH_CONFIG,
} path_fn_t;

typedef struct {
  const c8* name;
  path_fn_t fn;
  sp_err_t err;
} test_t;

static const test_t tests [] = {
  { .name = "cwd",     .fn = PATH_CWD,     .err = SP_ERR_SYS_NOT_FOUND },
  { .name = "exe",     .fn = PATH_EXE,     .err = SP_ERR_SYS_UNSUPPORTED },
  { .name = "storage", .fn = PATH_STORAGE, .err = SP_ERR_SYS_NOT_FOUND },
  { .name = "config",  .fn = PATH_CONFIG,  .err = SP_ERR_SYS_NAME_TOO_LONG },
};

static const test_t* active;

static sp_err_t get_path(c8* buf, u64 size, u64* len) {
  *len = 0;
  return active->err;
}

static sp_err_t get_fd_path(sp_sys_fd_t fd, c8* buf, u64 size, u64* len) {
  *len = 0;
  return active->err;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_exe_path = get_path;
  vt.get_storage_path = get_path;
  vt.get_config_path = get_path;
  vt.get_fd_path = get_fd_path;

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_zero;
  sp_err_t err = SP_OK;

  active = c;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  switch (c->fn) {
    case PATH_CWD:     err = sp_fs_get_cwd_path(mem, &path); break;
    case PATH_EXE:     err = sp_fs_get_exe_path(mem, &path); break;
    case PATH_STORAGE: err = sp_fs_get_storage_path(mem, &path); break;
    case PATH_CONFIG:  err = sp_fs_get_config_path(mem, &path); break;
  }
  sp_sys_set_vtable(saved);
  active = SP_NULLPTR;

  sp_expect_err_eq(t, err, c->err);
  return SP_OK;
}

sp_test_each_fn(fs, get_path_sim, test_t, tests, run, .serial = true);
