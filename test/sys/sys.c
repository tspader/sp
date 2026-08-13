#include "harness.h"

static sp_err_t sys_probe_symlinks(void* user) {
  sp_str_t dir = *(sp_str_t*)user;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_str_t target = sp_fs_join_path(scratch.mem, dir, sp_str_lit("probe_target"));
  sp_str_t link = sp_fs_join_path(scratch.mem, dir, sp_str_lit("probe_link"));

  sp_fs_create_file(target);
  sp_err_t err = sp_fs_create_sym_link(target, link);
  if (!err) sp_fs_remove_file(link);
  sp_fs_remove_file(target);

  sp_mem_end_scratch(scratch);
  return err;
}

static bool sys_case_wants_symlinks(sys_case_t* c) {
  sp_carr_for(c->setup, it) {
    if (!c->setup[it].path) break;
    if (c->setup[it].kind == SYS_SETUP_SYMLINK) return true;
  }
  return false;
}

static const c8* sys_step_name(sys_step_kind_t kind) {
  switch (kind) {
    case SYS_STEP_NONE:     return "none";
    case SYS_STEP_OPEN:     return "open";
    case SYS_STEP_OPEN_DIR: return "open_dir";
    case SYS_STEP_READ:     return "read";
    case SYS_STEP_WRITE:    return "write";
    case SYS_STEP_PWRITE:   return "pwrite";
  }
  return "";
}

static bool sys_check_err(sp_test_t* t, sp_err_t err, sp_err_t want) {
  if (err != want) {
    sp_test_fail(t, "returned {} but expected {}", sp_fmt_int(err), sp_fmt_int(want));
    return false;
  }
  return true;
}

sp_err_t sys_case_run(sp_test_t* t, sys_case_t* c) {
  sp_sys_fd_t fds [SYS_CASE_MAX_FDS];
  sp_carr_for(fds, it) fds[it] = SP_SYS_INVALID_FD;
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);

  if (sys_case_wants_symlinks(c) && sp_test_once(&sys_symlink_probe, sys_probe_symlinks, &sandbox)) {
    return sp_test_skip(t, "symlinks not available");
  }

  sp_try(sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd));

  sp_carr_for(c->setup, it) {
    sys_setup_t* ent = &c->setup[it];
    if (!ent->path) break;
    sp_str_t path = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(ent->path));

    switch (ent->kind) {
      case SYS_SETUP_FILE: {
        sp_fs_create_file_str(path, ent->content ? sp_cstr_as_str(ent->content) : sp_str_lit(""));
        break;
      }
      case SYS_SETUP_DIR: {
        sp_fs_create_dir(path);
        break;
      }
      case SYS_SETUP_SYMLINK: {
        sp_str_t target = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(ent->target));
        if (sp_fs_create_sym_link(target, path)) {
          sp_test_fail(t, "failed to create symlink {} -> {}", sp_fmt_str(path), sp_fmt_str(target));
          goto done;
        }
        break;
      }
    }
  }

  sp_carr_for(c->steps, it) {
    sys_step_t* step = &c->steps[it];
    if (step->kind == SYS_STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "[{}] {}", sp_fmt_uint(it), sp_fmt_cstr(sys_step_name(step->kind))));

    switch (step->kind) {
      case SYS_STEP_NONE: break;

      case SYS_STEP_OPEN: {
        sp_sys_fd_t fd = SP_SYS_INVALID_FD;
        sp_err_t err = sp_sys_open_s(sandbox_fd, sp_cstr_as_str(step->open.path), step->open.mode, step->open.flags, &fd);
        sys_check_err(t, err, step->open.err);
        if (!err) {
          if (step->open.err) sp_sys_close(fd);
          else                fds[step->open.slot] = fd;
        }
        break;
      }
      case SYS_STEP_OPEN_DIR: {
        sp_sys_fd_t fd = SP_SYS_INVALID_FD;
        sp_str_t path = sp_cstr_as_str(step->open_dir.path);
        sp_err_t err = sp_sys_open_dir_s(sandbox_fd, path, &fd);
        sys_check_err(t, err, step->open_dir.err);
        if (!err) {
          if (step->open_dir.err) sp_sys_close(fd);
          else                    fds[step->open_dir.slot] = fd;
        }
        break;
      }
      case SYS_STEP_READ: {
        c8 stack_buf [SYS_CASE_BUF_SIZE] = sp_zero;
        c8* buf = stack_buf;
        if (step->read.count > SYS_CASE_BUF_SIZE) {
          buf = sp_alloc_n(mem, c8, step->read.count);
        }
        u64 n = 0;
        sp_err_t err = sp_sys_read(fds[step->read.slot], buf, step->read.count, &n);
        if (sys_check_err(t, err, step->read.err) && !err) {
          u64 want = sp_cstr_len(step->read.expect);
          if (n != want || (n && !sp_mem_is_equal(buf, step->read.expect, want))) {
            sp_test_record(t, (sp_test_failure_t) {
              .message = sp_test_format(t, "read {} of {} bytes", sp_fmt_uint(n), sp_fmt_uint(want)),
              .expected = sp_test_format(t, "{.quote}", sp_fmt_cstr(step->read.expect)),
              .actual = sp_test_format(t, "{.quote}", sp_fmt_str(sp_str(buf, (u32)n))),
            });
          }
        }
        break;
      }
      case SYS_STEP_WRITE: {
        u64 len = sp_cstr_len(step->write.data);
        u64 n = 0;
        sp_err_t err = sp_sys_write(fds[step->write.slot], step->write.data, len, &n);
        if (sys_check_err(t, err, step->write.err) && !err && n != len) {
          sp_test_fail(t, "write returned {} but expected {}", sp_fmt_uint(n), sp_fmt_uint(len));
        }
        break;
      }
      case SYS_STEP_PWRITE: {
        u64 len = sp_cstr_len(step->pwrite.data);
        u64 n = 0;
        sp_err_t err = sp_sys_pwrite(fds[step->pwrite.slot], step->pwrite.data, len, step->pwrite.offset, &n);
        if (sys_check_err(t, err, step->pwrite.err) && !err && n != len) {
          sp_test_fail(t, "pwrite returned {} but expected {}", sp_fmt_uint(n), sp_fmt_uint(len));
        }
        break;
      }
    }
  }

  sp_test_kv_clear(t, "step");

  sp_carr_for(fds, it) {
    if (fds[it] != SP_SYS_INVALID_FD) {
      sp_sys_close(fds[it]);
      fds[it] = SP_SYS_INVALID_FD;
    }
  }

  sp_carr_for(c->expect, it) {
    sys_expect_t* ent = &c->expect[it];
    if (!ent->path) break;
    sp_str_t path = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(ent->path));

    bool exists = sp_fs_exists(path);
    if (exists != ent->exists) {
      sp_test_fail(t, "expected {} {} exist", sp_fmt_str(path), sp_fmt_cstr(ent->exists ? "to" : "not to"));
      continue;
    }

    if (ent->content) {
      sp_str_t actual = sp_zero;
      sp_io_read_file(mem, path, &actual);
      if (!sp_str_equal(actual, sp_cstr_as_str(ent->content))) {
        sp_test_record(t, (sp_test_failure_t) {
          .message = sp_test_format(t, "content of {}", sp_fmt_str(path)),
          .expected = sp_test_format(t, "{.quote}", sp_fmt_cstr(ent->content)),
          .actual = sp_test_format(t, "{.quote}", sp_fmt_str(actual)),
        });
      }
    }
  }

done:
  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
  return SP_OK;
}
