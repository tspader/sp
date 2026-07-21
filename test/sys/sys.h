#ifndef SYS_TEST_H
#define SYS_TEST_H

#include "sp.h"
#include "test.h"
#include "utest.h"

#define SYS_TEST_MAX_SETUP 8
#define SYS_TEST_MAX_STEPS 8
#define SYS_TEST_MAX_EXPECT 8
#define SYS_TEST_MAX_FDS 4
#define SYS_TEST_BUF_SIZE 64

static bool sys_symlinks_available = false;

static void sys_probe_symlinks(sp_test_file_manager_t* fm) {
  static bool probed = false;
  if (probed) return;
  probed = true;

  sp_str_t target = sp_test_file_path_c(fm, ".sys_symlink_probe_target");
  sp_str_t link = sp_test_file_path_c(fm, ".sys_symlink_probe_link");
  sp_fs_create_file(target);
  sys_symlinks_available = sp_fs_create_sym_link(target, link) == SP_OK;
  if (sys_symlinks_available) sp_fs_remove_file(link);
  sp_fs_remove_file(target);
}

typedef enum {
  SYS_SETUP_FILE,
  SYS_SETUP_DIR,
  SYS_SETUP_SYMLINK,
  SYS_SETUP_READONLY,
} sys_setup_kind_t;

typedef struct {
  const c8* path;
  sys_setup_kind_t kind;
  union {
    const c8* content;
    const c8* target;
  };
} sys_setup_t;

typedef enum {
  SYS_STEP_NONE,
  SYS_STEP_OPEN,
  SYS_STEP_OPEN_DIR,
  SYS_STEP_CLOSE,
  SYS_STEP_READ,
  SYS_STEP_PREAD,
  SYS_STEP_WRITE,
  SYS_STEP_PWRITE,
  SYS_STEP_SYMLINK,
  SYS_STEP_RENAME,
  SYS_STEP_LSTAT,
  SYS_STEP_STAT,
  SYS_STEP_FSTAT,
  SYS_STEP_MKDIR,
  SYS_STEP_RMDIR,
  SYS_STEP_UNLINK,
  SYS_STEP_LINK,
} sys_step_kind_t;

typedef enum {
  SYS_DIR_SANDBOX,
  SYS_DIR_SLOT_0,
  SYS_DIR_SLOT_1,
  SYS_DIR_SLOT_2,
  SYS_DIR_SLOT_3,
} sys_dir_t;

typedef struct {
  sys_step_kind_t kind;
  union {
    struct { u32 slot; const c8* path; sp_sys_open_mode_t mode; u32 flags; bool fail; sp_err_t err; } open;
    struct { u32 slot; const c8* path; bool fail; } open_dir;
    struct { u32 slot; } close;
    struct { u32 slot; u64 count; const c8* expect; bool fail; sp_err_t err; } read;
    struct { u32 slot; u64 count; u64 offset; const c8* expect; } pread;
    struct { u32 slot; const c8* data; bool fail; sp_err_t err; } write;
    struct { u32 slot; const c8* data; u64 offset; } pwrite;
    struct { const c8* target; const c8* alias; } symlink;
    struct { const c8* from; sys_dir_t from_dir; const c8* to; sys_dir_t to_dir; bool fail; } rename;
    struct { const c8* path; sp_fs_kind_t kind; bool fail; } lstat;
    struct { const c8* path; sp_fs_kind_t kind; s64 size; bool fail; } stat;
    struct { u32 slot; sp_fs_kind_t kind; s64 size; } fstat;
    struct { const c8* path; bool fail; } mkdir;
    struct { const c8* path; bool fail; } rmdir;
    struct { const c8* path; bool fail; } unlink;
    struct { const c8* existing; sys_dir_t existing_dir; const c8* alias; sys_dir_t alias_dir; bool fail; } link;
  };
} sys_step_t;

typedef struct {
  const c8* path;
  bool exists;
  const c8* content;
} sys_expect_t;

typedef struct {
  const c8* label;
  sys_setup_t setup [SYS_TEST_MAX_SETUP];
  sys_step_t steps [SYS_TEST_MAX_STEPS];
  sys_expect_t expect [SYS_TEST_MAX_EXPECT];
} sys_test_t;

static bool sys_test_wants_symlinks(sys_test_t* t) {
  sp_carr_for(t->setup, it) {
    if (!t->setup[it].path) break;
    if (t->setup[it].kind == SYS_SETUP_SYMLINK) return true;
  }
  sp_carr_for(t->steps, it) {
    if (t->steps[it].kind == SYS_STEP_NONE) break;
    if (t->steps[it].kind == SYS_STEP_SYMLINK) return true;
  }
  return false;
}

static void sys_expect_bytes(s32* utest_result, const c8* label, const c8* buf, s64 n, const c8* expect) {
  s64 len = (s64)sp_cstr_len(expect);
  if (n != len) {
    SP_TEST_REPORT("{} returned {} but expected {}", sp_fmt_cstr(label), sp_fmt_int(n), sp_fmt_int(len));
    SP_FAIL();
    return;
  }
  if (n > 0 && !sp_mem_is_equal(buf, expect, (u64)len)) {
    SP_TEST_REPORT("{} produced {} but expected {}", sp_fmt_cstr(label), sp_fmt_str(sp_str((c8*)buf, (u32)n)), sp_fmt_cstr(expect));
    SP_FAIL();
  }
}

static void sys_expect_err(s32* utest_result, const c8* label, sp_err_t err, sp_err_t expect, bool fail) {
  if (expect != SP_OK) {
    if (err != expect) {
      SP_TEST_REPORT("{} returned {} but expected {}", sp_fmt_cstr(label), sp_fmt_int((s64)err), sp_fmt_int((s64)expect));
      SP_FAIL();
    }
    return;
  }
  if (!fail && err != SP_OK) {
    SP_TEST_REPORT("{} returned {} but expected SP_OK", sp_fmt_cstr(label), sp_fmt_int((s64)err));
    SP_FAIL();
  }
  if (fail && err == SP_OK) {
    SP_TEST_REPORT("{} returned SP_OK but expected failure", sp_fmt_cstr(label));
    SP_FAIL();
  }
}

static void sys_expect_rc(s32* utest_result, const c8* label, s32 rc, bool fail) {
  if (!fail && rc != 0) {
    SP_TEST_REPORT("{} returned {} but expected 0", sp_fmt_cstr(label), sp_fmt_int(rc));
    SP_FAIL();
  }
  if (fail && rc == 0) {
    SP_TEST_REPORT("{} returned 0 but expected failure", sp_fmt_cstr(label));
    SP_FAIL();
  }
}

static void sys_expect_meta(s32* utest_result, const c8* label, s32 rc, bool fail, const sp_sys_file_meta_t* meta, sp_fs_kind_t kind, s64 size) {
  sys_expect_rc(utest_result, label, rc, fail);
  if (fail || rc != 0) return;
  if (kind == SP_FS_KIND_NONE) return;
  if (meta->kind != kind) {
    SP_TEST_REPORT("{} reported kind {} but expected {}", sp_fmt_cstr(label), sp_fmt_int((s64)meta->kind), sp_fmt_int((s64)kind));
    SP_FAIL();
  }
  if (kind == SP_FS_KIND_FILE && meta->size != size) {
    SP_TEST_REPORT("{} reported size {} but expected {}", sp_fmt_cstr(label), sp_fmt_int(meta->size), sp_fmt_int(size));
    SP_FAIL();
  }
}

static sp_sys_fd_t sys_dir_fd(sp_sys_fd_t sandbox, const sp_sys_fd_t* fds, sys_dir_t dir) {
  return dir == SYS_DIR_SANDBOX ? sandbox : fds[dir - SYS_DIR_SLOT_0];
}

static bool sys_apply_setup(s32* utest_result, sp_test_file_manager_t* fm, sp_str_t sandbox, sys_setup_t* setup) {
  sp_for(it, SYS_TEST_MAX_SETUP) {
    sys_setup_t* ent = &setup[it];
    if (!ent->path) break;
    sp_str_t path = sp_fs_join_path(fm->mem, sandbox, sp_cstr_as_str(ent->path));

    switch (ent->kind) {
      case SYS_SETUP_FILE: {
        sp_test_file_create_ex((sp_test_file_config_t) {
          .path = path,
          .content = ent->content ? sp_cstr_as_str(ent->content) : sp_str_lit(""),
        });
        break;
      }
      case SYS_SETUP_DIR: {
        sp_fs_create_dir(path);
        break;
      }
      case SYS_SETUP_SYMLINK: {
        sp_str_t target = sp_fs_join_path(fm->mem, sandbox, sp_cstr_as_str(ent->target));
        if (sp_fs_create_sym_link(target, path) != SP_OK) {
          SP_TEST_REPORT("failed to create symlink {} -> {}", sp_fmt_str(path), sp_fmt_str(target));
          SP_FAIL();
          return false;
        }
        break;
      }
      case SYS_SETUP_READONLY: {
        sp_sys_file_meta_t meta = sp_zero;
        if (sp_sys_get_path_metadata_s(sp_sys_get_root(0), path, &meta)) {
          SP_TEST_REPORT("failed to stat {}", sp_fmt_str(path));
          SP_FAIL();
          return false;
        }
#if defined(SP_WIN32)
        meta.raw_attrs |= FILE_ATTRIBUTE_READONLY;
#else
        meta.raw_attrs &= ~(u32)0222;
#endif
        if (sp_sys_chmod_s(sp_sys_get_root(0), path, &meta)) {
          SP_TEST_REPORT("failed to chmod {}", sp_fmt_str(path));
          SP_FAIL();
          return false;
        }
        break;
      }
    }
  }
  return true;
}

static void sys_expect_paths(s32* utest_result, sp_test_file_manager_t* fm, sp_str_t sandbox, sys_expect_t* expect) {
  sp_for(it, SYS_TEST_MAX_EXPECT) {
    sys_expect_t* ent = &expect[it];
    if (!ent->path) break;
    sp_str_t path = sp_fs_join_path(fm->mem, sandbox, sp_cstr_as_str(ent->path));

    bool exists = sp_fs_exists(path);
    if (exists != ent->exists) {
      SP_TEST_REPORT("expected {} {} exist", sp_fmt_str(path), sp_fmt_cstr(ent->exists ? "to" : "not to"));
      SP_FAIL();
      continue;
    }

    if (ent->content) {
      sp_str_t actual = sp_zero;
      sp_io_read_file(fm->mem, path, &actual);
      if (!sp_str_equal(actual, sp_cstr_as_str(ent->content))) {
        SP_TEST_REPORT("{} content was {} but expected {}", sp_fmt_str(path), sp_fmt_str(actual), sp_fmt_cstr(ent->content));
        SP_FAIL();
      }
    }
  }
}

static void run_sys_test(s32* utest_result, sys_test_t t) {
  sp_sys_fd_t fds [SYS_TEST_MAX_FDS];
  sp_carr_for(fds, it) fds[it] = SP_SYS_INVALID_FD;
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;

  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);

  if (sys_test_wants_symlinks(&t)) {
    sys_probe_symlinks(&fm);
    if (!sys_symlinks_available) {
      sp_test_file_manager_cleanup(&fm);
      UTEST_SKIP("symlinks not available");
    }
  }

  sp_str_t sandbox = sp_test_file_create_dir(&fm, t.label);
  sandbox_fd = sp_sys_open_dir_s(sp_sys_get_root(0), sandbox);
  if (sandbox_fd == SP_SYS_INVALID_FD) {
    SP_TEST_REPORT("failed to open sandbox {}", sp_fmt_str(sandbox));
    SP_FAIL();
    goto done;
  }

  if (!sys_apply_setup(utest_result, &fm, sandbox, t.setup)) goto done;

  sp_carr_for(t.steps, it) {
    sys_step_t* step = &t.steps[it];
    if (step->kind == SYS_STEP_NONE) break;

    switch (step->kind) {
      case SYS_STEP_NONE: break;

      case SYS_STEP_OPEN: {
        sp_sys_fd_t fd = SP_SYS_INVALID_FD;
        sp_err_t err = sp_sys_open_s(sandbox_fd, sp_cstr_as_str(step->open.path), step->open.mode, step->open.flags, &fd);
        sys_expect_err(utest_result, "open", err, step->open.err, step->open.fail);
        if (err == SP_OK) {
          if (step->open.err || step->open.fail) {
            sp_sys_close(fd);
          }
          else {
            fds[step->open.slot] = fd;
          }
        }
        break;
      }
      case SYS_STEP_OPEN_DIR: {
        sp_sys_fd_t fd = sp_sys_open_dir_s(sandbox_fd, sp_cstr_as_str(step->open_dir.path));
        if (step->open_dir.fail) {
          if (fd != SP_SYS_INVALID_FD) {
            SP_TEST_REPORT("open_dir of {} succeeded but expected failure", sp_fmt_cstr(step->open_dir.path));
            SP_FAIL();
            sp_sys_close(fd);
          }
        }
        else {
          if (fd == SP_SYS_INVALID_FD) {
            SP_TEST_REPORT("failed to open_dir {}", sp_fmt_cstr(step->open_dir.path));
            SP_FAIL();
          }
          fds[step->open_dir.slot] = fd;
        }
        break;
      }
      case SYS_STEP_CLOSE: {
        sp_sys_close(fds[step->close.slot]);
        fds[step->close.slot] = SP_SYS_INVALID_FD;
        break;
      }
      case SYS_STEP_READ: {
        c8 stack_buf [SYS_TEST_BUF_SIZE] = sp_zero;
        c8* buf = stack_buf;
        if (step->read.count > SYS_TEST_BUF_SIZE) {
          buf = (c8*)sp_sys_alloc(step->read.count);
          if (!buf) {
            SP_TEST_REPORT("failed to allocate read buffer of {} bytes", sp_fmt_int((s64)step->read.count));
            SP_FAIL();
            break;
          }
        }
        u64 n = 0;
        sp_err_t err = sp_sys_read(fds[step->read.slot], buf, step->read.count, &n);
        sys_expect_err(utest_result, "read", err, step->read.err, step->read.fail);
        if (err == SP_OK && !step->read.err && !step->read.fail) {
          sys_expect_bytes(utest_result, "read", buf, (s64)n, step->read.expect);
        }
        if (buf != stack_buf) sp_sys_free(buf, step->read.count);
        break;
      }
      case SYS_STEP_PREAD: {
        c8 buf [SYS_TEST_BUF_SIZE] = sp_zero;
        u64 n = 0;
        sp_err_t err = sp_sys_pread(fds[step->pread.slot], buf, step->pread.count, step->pread.offset, &n);
        sys_expect_err(utest_result, "pread", err, SP_OK, false);
        if (err == SP_OK) {
          sys_expect_bytes(utest_result, "pread", buf, (s64)n, step->pread.expect);
        }
        break;
      }
      case SYS_STEP_WRITE: {
        u64 len = sp_cstr_len(step->write.data);
        u64 n = 0;
        sp_err_t err = sp_sys_write(fds[step->write.slot], step->write.data, len, &n);
        sys_expect_err(utest_result, "write", err, step->write.err, step->write.fail);
        if (err == SP_OK && !step->write.err && !step->write.fail && n != len) {
          SP_TEST_REPORT("write returned {} but expected {}", sp_fmt_uint(n), sp_fmt_uint(len));
          SP_FAIL();
        }
        break;
      }
      case SYS_STEP_PWRITE: {
        u64 len = sp_cstr_len(step->pwrite.data);
        u64 n = 0;
        sp_err_t err = sp_sys_pwrite(fds[step->pwrite.slot], step->pwrite.data, len, step->pwrite.offset, &n);
        sys_expect_err(utest_result, "pwrite", err, SP_OK, false);
        if (err == SP_OK && n != len) {
          SP_TEST_REPORT("pwrite returned {} but expected {}", sp_fmt_uint(n), sp_fmt_uint(len));
          SP_FAIL();
        }
        break;
      }
      case SYS_STEP_SYMLINK: {
        s32 rc = sp_sys_symlink_s(sp_cstr_as_str(step->symlink.target), sandbox_fd, sp_cstr_as_str(step->symlink.alias));
        sys_expect_rc(utest_result, "symlink", rc, false);
        break;
      }
      case SYS_STEP_RENAME: {
        s32 rc = sp_sys_rename_s(
          sys_dir_fd(sandbox_fd, fds, step->rename.from_dir), sp_cstr_as_str(step->rename.from),
          sys_dir_fd(sandbox_fd, fds, step->rename.to_dir), sp_cstr_as_str(step->rename.to));
        sys_expect_rc(utest_result, "rename", rc, step->rename.fail);
        break;
      }
      case SYS_STEP_LSTAT: {
        sp_sys_file_meta_t meta = sp_zero;
        s32 rc = sp_sys_get_link_metadata_s(sandbox_fd, sp_cstr_as_str(step->lstat.path), &meta);
        sys_expect_meta(utest_result, "lstat", rc, step->lstat.fail, &meta, step->lstat.kind, 0);
        break;
      }
      case SYS_STEP_STAT: {
        sp_sys_file_meta_t meta = sp_zero;
        s32 rc = sp_sys_get_path_metadata_s(sandbox_fd, sp_cstr_as_str(step->stat.path), &meta);
        sys_expect_meta(utest_result, "stat", rc, step->stat.fail, &meta, step->stat.kind, step->stat.size);
        break;
      }
      case SYS_STEP_FSTAT: {
        sp_sys_file_meta_t meta = sp_zero;
        sp_err_t rc = sp_sys_get_file_metadata(fds[step->fstat.slot], &meta);
        sys_expect_meta(utest_result, "fstat", rc == SP_OK ? 0 : -1, false, &meta, step->fstat.kind, step->fstat.size);
        break;
      }
      case SYS_STEP_MKDIR: {
        s32 rc = sp_sys_mkdir_s(sandbox_fd, sp_cstr_as_str(step->mkdir.path), 0755);
        sys_expect_rc(utest_result, "mkdir", rc, step->mkdir.fail);
        break;
      }
      case SYS_STEP_RMDIR: {
        s32 rc = sp_sys_rmdir_s(sandbox_fd, sp_cstr_as_str(step->rmdir.path));
        sys_expect_rc(utest_result, "rmdir", rc, step->rmdir.fail);
        break;
      }
      case SYS_STEP_UNLINK: {
        s32 rc = sp_sys_unlink_s(sandbox_fd, sp_cstr_as_str(step->unlink.path));
        sys_expect_rc(utest_result, "unlink", rc, step->unlink.fail);
        break;
      }
      case SYS_STEP_LINK: {
        s32 rc = sp_sys_link_s(
          sys_dir_fd(sandbox_fd, fds, step->link.existing_dir), sp_cstr_as_str(step->link.existing),
          sys_dir_fd(sandbox_fd, fds, step->link.alias_dir), sp_cstr_as_str(step->link.alias));
        sys_expect_rc(utest_result, "link", rc, step->link.fail);
        break;
      }
    }
  }

  sp_carr_for(fds, it) {
    if (fds[it] != SP_SYS_INVALID_FD) {
      sp_sys_close(fds[it]);
      fds[it] = SP_SYS_INVALID_FD;
    }
  }

  sys_expect_paths(utest_result, &fm, sandbox, t.expect);

done:
  sp_carr_for(fds, it) {
    if (fds[it] != SP_SYS_INVALID_FD) sp_sys_close(fds[it]);
  }
  sp_carr_for(t.steps, it) {
    if (t.steps[it].kind == SYS_STEP_SYMLINK) sp_fs_remove_file(sp_cstr_as_str(t.steps[it].symlink.alias));
  }
  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
  sp_test_file_manager_cleanup(&fm);
}

#endif
