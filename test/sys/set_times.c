#include "sp.h"
#include "sp/sp_test.h"

#define ATIME_S 1000000000
#define MTIME_S 1100000000
#define ATIME_NS 100
#define MTIME_NS 200

typedef enum {
  SETUP_FILE,
  SETUP_DIR,
  SETUP_LINK,
} setup_t;

typedef struct {
  sp_err_t err;
  const c8* kept;
} expect_t;

typedef struct {
  const c8* name;
  setup_t setup;
  const c8* path;
  s64 nsec;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "file", .path = "A" },
  { .name = "dir", .setup = SETUP_DIR, .path = "A" },
  { .name = "follows_link", .setup = SETUP_LINK, .path = "L", .expect = { .kept = "L" } },
  { .name = "nanoseconds_past_a_second", .path = "A", .nsec = 1000000000, .expect = { .err = SP_ERR_SYS_INVALID } },
  { .name = "nanoseconds_negative", .path = "A", .nsec = -1, .expect = { .err = SP_ERR_SYS_INVALID } },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t a = sp_path_join(mem, sandbox, sp_str_lit("A"));

  switch (c->setup) {
    case SETUP_FILE: {
      sp_must_ok(t, sp_fs_create_file_at(a));
      break;
    }
    case SETUP_DIR: {
      sp_must_ok(t, sp_fs_create_dir_at(a));
      break;
    }
    case SETUP_LINK: {
      sp_test_skip_without_symlinks();
      sp_must_ok(t, sp_fs_create_file_at(a));
      sp_must_ok(t, sp_fs_create_sym_link_at(sp_str_lit("A"), sp_path_join(mem, sandbox, sp_str_lit("L")), SP_FS_KIND_FILE));
      break;
    }
  }

  sp_sys_timespec_t atime = { .tv_sec = ATIME_S, .tv_nsec = c->nsec ? c->nsec : ATIME_NS };
  sp_sys_timespec_t mtime = { .tv_sec = MTIME_S, .tv_nsec = c->nsec ? c->nsec : MTIME_NS };
  sp_path_t path = sp_path_join(mem, sandbox, sp_cstr_as_str(c->path));
  sp_expect_err_eq(t, sp_sys_set_times_s(path.dir, path.sub, atime, mtime), c->expect.err);
  if (c->expect.err) return SP_OK;

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(a.dir, a.sub, &meta));
  sp_expect_eq(t, meta.atime.tv_sec, atime.tv_sec);
  sp_expect_eq(t, meta.atime.tv_nsec, atime.tv_nsec);
  sp_expect_eq(t, meta.mtime.tv_sec, mtime.tv_sec);
  sp_expect_eq(t, meta.mtime.tv_nsec, mtime.tv_nsec);

  if (c->expect.kept) {
    sp_path_t kept = sp_path_join(mem, sandbox, sp_cstr_as_str(c->expect.kept));
    sp_sys_file_meta_t link = sp_zero;
    sp_must_ok(t, sp_sys_get_link_metadata_s(kept.dir, kept.sub, &link));
    sp_expect_ne(t, link.mtime.tv_sec, mtime.tv_sec);
  }
  return SP_OK;
}

sp_test_each_fn(sys, set_times, test_t, tests, run);
