#include "sp.h"
#include "sp/sp_test.h"
#include "reparse.h"

typedef enum {
  SETUP_NONE,
  SETUP_FILE,
  SETUP_LINK,
  SETUP_DIR_LINK,
  SETUP_LINK_ABSOLUTE,
#if defined(SP_WIN32)
  SETUP_JUNCTION,
  SETUP_STUB,
#endif
} setup_t;

typedef struct {
  sp_err_t err;
} expect_t;

typedef struct {
  const c8* name;
  setup_t setup;
  bool taken;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "relative", .setup = SETUP_LINK },
  { .name = "absolute", .setup = SETUP_LINK_ABSOLUTE },
  { .name = "directory_link", .setup = SETUP_DIR_LINK },
  { .name = "taken_name", .setup = SETUP_LINK, .taken = true, .expect = { .err = SP_ERR_SYS_EXISTS } },
  { .name = "missing", .expect = { .err = SP_ERR_SYS_NOT_FOUND } },
  { .name = "not_a_link", .setup = SETUP_FILE, .expect = { .err = SP_ERR_SYS_INVALID } },
#if defined(SP_WIN32)
  { .name = "junction", .setup = SETUP_JUNCTION },
  { .name = "reparse_point_that_is_not_a_link", .setup = SETUP_STUB, .expect = { .err = SP_ERR_SYS_INVALID } },
#endif
};

sp_test_each(sys, copy_link, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("L"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("M"));

  switch (it->setup) {
    case SETUP_NONE: {
      break;
    }
    case SETUP_FILE: {
      sp_must_ok(t, sp_fs_write_at(from, sp_str_lit(""), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
      break;
    }
    case SETUP_LINK: {
      sp_test_skip_without_symlinks();
      sp_must_ok(t, sp_fs_create_sym_link_at(sp_str_lit("A"), from, SP_FS_KIND_FILE));
      break;
    }
    case SETUP_DIR_LINK: {
      sp_test_skip_without_symlinks();
      sp_must_ok(t, sp_fs_create_sym_link_at(sp_str_lit("A"), from, SP_FS_KIND_DIR));
      break;
    }
    case SETUP_LINK_ABSOLUTE: {
      sp_test_skip_without_absolute_names();
      sp_test_skip_without_symlinks();
      sp_str_t root = sp_zero;
      sp_must_ok(t, sp_fs_canonicalize_path_at(mem, sandbox, &root));
      sp_must_ok(t, sp_fs_create_sym_link_at(sp_fs_join_path(mem, root, sp_str_lit("A")), from, SP_FS_KIND_FILE));
      break;
    }
#if defined(SP_WIN32)
    case SETUP_JUNCTION: {
      sp_str_t root = sp_zero;
      sp_must_ok(t, sp_fs_canonicalize_path_at(mem, sandbox, &root));
      sp_must_ok(t, reparse_create_junction(mem, from, sp_fs_join_path(mem, root, sp_str_lit("A"))));
      break;
    }
    case SETUP_STUB: {
      sp_must_ok(t, reparse_create_stub(mem, from, SP_FS_KIND_FILE));
      break;
    }
#endif
  }
  if (it->taken) {
    sp_must_ok(t, sp_fs_write_at(to, sp_str_lit("M"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));
  }

  sp_err_t err = sp_sys_copy_link_s(from.dir, from.sub, to.dir, to.sub);
  sp_expect_err_eq(t, err, it->expect.err);

  if (it->taken) {
    sp_str_t content = sp_zero;
    sp_expect_ok(t, sp_fs_read_at(mem, to, &content));
    sp_expect_str_eq(t, content, sp_str_lit("M"));
  }
  if (it->expect.err) return SP_OK;

  sp_sys_file_meta_t source = sp_zero;
  sp_sys_file_meta_t copy = sp_zero;
  sp_must_ok(t, sp_sys_get_link_metadata_s(from.dir, from.sub, &source));
  sp_must_ok(t, sp_sys_get_link_metadata_s(to.dir, to.sub, &copy));
  sp_expect_eq(t, copy.kind, SP_FS_KIND_SYMLINK);
  sp_expect_eq(t, source.kind, SP_FS_KIND_SYMLINK);

  c8 source_buf [SP_PATH_MAX];
  c8 copy_buf [SP_PATH_MAX];
  sp_str_t source_target = sp_zero;
  sp_str_t copy_target = sp_zero;
  sp_must_ok(t, sp_sys_readlink_s(from.dir, from.sub, source_buf, sizeof(source_buf), &source_target));
  sp_must_ok(t, sp_sys_readlink_s(to.dir, to.sub, copy_buf, sizeof(copy_buf), &copy_target));
  sp_expect_str_eq(t, copy_target, source_target);

#if defined(SP_WIN32)
  u32 source_tag = 0;
  u32 copy_tag = 0;
  sp_must_ok(t, reparse_get_tag(mem, from, &source_tag));
  sp_must_ok(t, reparse_get_tag(mem, to, &copy_tag));
  sp_expect_eq(t, copy_tag, source_tag);
  sp_expect_eq(t, copy.perms.value & FILE_ATTRIBUTE_DIRECTORY, source.perms.value & FILE_ATTRIBUTE_DIRECTORY);
#endif
  return SP_OK;
}
