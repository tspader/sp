#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_POSIX)
  #include <signal.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <unistd.h>
#endif

typedef enum {
  OP_UNLINK,
  OP_RMDIR,
  OP_MKDIR,
  OP_STAT,
  OP_LSTAT,
} op_t;

typedef enum {
  PATH_LITERAL,
  PATH_LONG,
} path_kind_t;

typedef struct {
  const c8* name;
  op_t op;
  path_kind_t path_kind;
  const c8* path;
  sp_err_t err;
  const c8* file;
  const c8* dir;
  const c8* link;
  const c8* target;
  const c8* exists;
  const c8* not_exists;
  sp_fs_kind_t kind;
} test_t;

static const test_t tests [] = {
  {
    .name = "unlink_refuses_long_path",
    .op = OP_UNLINK,
    .path_kind = PATH_LONG,
    .path = "victim1.bin",
    .err = SP_ERR_SYS_NAME_TOO_LONG,
    .file = "victim1.bin",
    .exists = "victim1.bin",
  },
  {
    .name = "mkdir_refuses_long_path",
    .op = OP_MKDIR,
    .path_kind = PATH_LONG,
    .path = "newdir1.dir",
    .err = SP_ERR_SYS_NAME_TOO_LONG,
    .not_exists = "newdir1.dir",
  },
  // disabled: win32 strips trailing separators where POSIX kernels give them
  // meaning (unlink("file/") deletes on win32, NOT_DIR on POSIX; lstat("lnk/")
  // stats the link on win32, follows on POSIX). the POSIX kernels also
  // disagree with each other in the directory/dir-symlink corners, so the
  // contract needs pinning on real kernels before the win32 translation is
  // written.
  //
  // {
  //   .name = "unlink_trailing_slash_refuses_file",
  //   .op = OP_UNLINK,
  //   .path = "file.bin/",
  //   .err = SP_ERR_SYS_NOT_DIR,
  //   .file = "file.bin",
  //   .exists = "file.bin",
  // },
  // {
  //   .name = "stat_trailing_slash_refuses_file",
  //   .op = OP_STAT,
  //   .path = "file.bin/",
  //   .err = SP_ERR_SYS_NOT_DIR,
  //   .file = "file.bin",
  // },
  // {
  //   .name = "lstat_trailing_slash_follows_dir_symlink",
  //   .op = OP_LSTAT,
  //   .path = "lnk/",
  //   .dir = "dir",
  //   .link = "lnk",
  //   .target = "dir",
  //   .kind = SP_FS_KIND_DIR,
  // },
  {
    .name = "rmdir_trailing_slash_removes_dir",
    .op = OP_RMDIR,
    .path = "dir/",
    .dir = "dir",
    .not_exists = "dir",
  },
  {
    .name = "unlink_empty_path_refuses",
    .op = OP_UNLINK,
    .path = "",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
};

// A path one component past SP_PATH_MAX whose first SP_PATH_MAX - 1 bytes
// resolve to name, so an implementation that truncates instead of refusing
// lands exactly on name and operates on it. Requires an odd-length name so
// the "./" padding comes out even.
static sp_str_t long_path(sp_mem_t mem, const c8* name) {
  u32 name_len = sp_cstr_len(name);
  u32 pad = (SP_PATH_MAX - 1) - name_len;
  c8* buf = sp_alloc_n(mem, c8, SP_PATH_MAX + 1);
  sp_for(it, pad / 2) {
    buf[it * 2] = '.';
    buf[it * 2 + 1] = '/';
  }
  sp_mem_copy(buf + pad, name, name_len);
  buf[SP_PATH_MAX - 1] = '/';
  buf[SP_PATH_MAX] = 'x';
  return sp_str(buf, SP_PATH_MAX + 1);
}

static bool check_err(sp_test_t* t, sp_err_t err, sp_err_t want) {
  if (err != want) {
    sp_test_fail(t, "returned {} but expected {}",
      sp_fmt_str(sp_test_err_str(t, err)), sp_fmt_str(sp_test_err_str(t, want)));
    return false;
  }
  return true;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  sp_try(sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd));

  if (c->file) sp_fs_create_file_str(sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->file)), sp_str_lit("A"));
  if (c->dir)  sp_fs_create_dir(sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->dir)));
  if (c->link) {
    sp_str_t target = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->target));
    sp_str_t link = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->link));
    if (sp_fs_create_sym_link(target, link)) {
      sp_sys_close(sandbox_fd);
      return sp_test_skip(t, "symlinks not available");
    }
  }

  sp_str_t path = c->path_kind == PATH_LONG ? long_path(mem, c->path) : sp_cstr_as_str(c->path);

  sp_sys_file_meta_t meta = sp_zero;
  sp_err_t err = SP_OK;
  switch (c->op) {
    case OP_UNLINK: err = sp_sys_unlink_s(sandbox_fd, path); break;
    case OP_RMDIR:  err = sp_sys_rmdir_s(sandbox_fd, path); break;
    case OP_MKDIR:  err = sp_sys_mkdir_s(sandbox_fd, path, 0755); break;
    case OP_STAT:   err = sp_sys_get_path_metadata_s(sandbox_fd, path, &meta); break;
    case OP_LSTAT:  err = sp_sys_get_link_metadata_s(sandbox_fd, path, &meta); break;
  }

  if (check_err(t, err, c->err) && !err && c->kind && meta.kind != c->kind) {
    sp_test_fail(t, "kind {} but expected {}", sp_fmt_int(meta.kind), sp_fmt_int(c->kind));
  }

  if (c->exists && !sp_fs_exists(sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->exists)))) {
    sp_test_fail(t, "expected {} to survive", sp_fmt_cstr(c->exists));
  }
  if (c->not_exists && sp_fs_exists(sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->not_exists)))) {
    sp_test_fail(t, "expected {} not to be created", sp_fmt_cstr(c->not_exists));
  }

  sp_sys_close(sandbox_fd);
  return SP_OK;
}

sp_test_each_fn(sys, path, test_t, tests, run);

sp_test(sys, canonicalize_refuses_overflow) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_fs_join_path(mem, sp_test_dir(t), sp_str_lit("file.bin"));
  sp_fs_create_file(path);

  c8 full [SP_PATH_MAX] = sp_zero;
  s64 len = sp_sys_canonicalize_path_s(path, full, sizeof(full));
  if (len <= 0) {
    sp_test_fail(t, "canonicalize failed with a full-size buffer");
    return SP_OK;
  }

  c8 buf [SP_PATH_MAX];
  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  s64 n = sp_sys_canonicalize_path_s(path, buf, (u64)len + 1);
  if (n != len) {
    sp_test_fail(t, "exact fit returned {} but expected {}", sp_fmt_int(n), sp_fmt_int(len));
  }
  else if (buf[len] != 0) {
    sp_test_fail(t, "exact fit did not NUL-terminate");
  }
  else if (!sp_mem_is_equal(buf, full, (u64)len)) {
    sp_test_fail(t, "exact fit returned different path");
  }

  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  n = sp_sys_canonicalize_path_s(path, buf, (u64)len);
  if (n != -1) {
    sp_test_fail(t, "no room for NUL: returned {} but expected -1", sp_fmt_int(n));
  }

  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;
  n = sp_sys_canonicalize_path_s(path, buf, (u64)len - 1);
  if (n != -1) {
    sp_test_fail(t, "undersized buffer: returned {} but expected -1", sp_fmt_int(n));
  }

  return SP_OK;
}

sp_test(sys, canonicalize_ignores_read_permission) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_fs_join_path(mem, sp_test_dir(t), sp_str_lit("file.bin"));
  sp_fs_create_file(path);

#if defined(SP_POSIX)
  if (geteuid() == 0) return sp_test_skip(t, "running as root");
#endif

  sp_sys_file_meta_t meta = sp_zero;
  sp_err_t err = sp_sys_chmod_s(sp_sys_get_root(0), path, &meta);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "chmod not supported");
  sp_try(err);

  c8 buf [SP_PATH_MAX] = sp_zero;
  s64 n = sp_sys_canonicalize_path_s(path, buf, sizeof(buf));
  if (n <= 0) sp_test_fail(t, "canonicalize failed on an unreadable file");
  return SP_OK;
}

sp_test(sys, canonicalize_does_not_block_on_fifo) {
#if defined(SP_POSIX)
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_fs_join_path(mem, sp_test_dir(t), sp_str_lit("fifo"));
  if (mkfifo(sp_str_to_cstr(mem, path), 0644)) return sp_test_skip(t, "fifos not available");

  pid_t pid = fork();
  if (pid < 0) return sp_test_skip(t, "fork not available");
  if (pid == 0) {
    c8 buf [SP_PATH_MAX] = sp_zero;
    _exit(sp_sys_canonicalize_path_s(path, buf, sizeof(buf)) > 0 ? 0 : 1);
  }

  s32 status = 0;
  sp_for(it, 100) {
    if (waitpid(pid, &status, WNOHANG) == pid) {
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        sp_test_fail(t, "canonicalize failed on a fifo");
      }
      return SP_OK;
    }
    sp_os_sleep_ms(5);
  }

  sp_test_fail(t, "canonicalize blocked on a fifo with no writer");
  kill(pid, SIGKILL);
  waitpid(pid, &status, 0);
  return SP_OK;
#else
  return sp_test_skip(t, "no fifos");
#endif
}
