#include "fs.h"

typedef struct {
  sp_err_t err;
  u32 renames;
  u32 links;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  sp_err_t rename;
  sp_err_t link;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "rename_publishes",
    .expect = {
      .renames = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "rename_refuses_existing",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .rename = SP_ERR_SYS_EXISTS,
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .renames = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "link_publishes_when_rename_is_unsupported",
    .rename = SP_ERR_SYS_UNSUPPORTED,
    .expect = {
      .renames = 1,
      .links = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "link_refuses_existing",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .rename = SP_ERR_SYS_UNSUPPORTED,
    .expect = {
      .err = SP_ERR_SYS_EXISTS,
      .renames = 1,
      .links = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "unsupported_when_neither_works",
    .rename = SP_ERR_SYS_UNSUPPORTED,
    .link = SP_ERR_SYS_UNSUPPORTED,
    .expect = {
      .err = SP_ERR_SYS_UNSUPPORTED,
      .renames = 1,
      .links = 1,
      .paths = {
        { .path = "A" },
      },
    },
  },
};

typedef struct {
  sp_err_t rename;
  sp_err_t link;
  u32 renames;
  u32 links;
} probe_t;

static probe_t probe;

static sp_err_t probed_rename(sp_sys_dir_t from_dir, const c8* from, u32 from_len, sp_sys_dir_t to_dir, const c8* to, u32 to_len, u32 flags) {
  if (!(flags & SP_SYS_RENAME_NO_REPLACE)) return sp_sys_rename_p(from_dir, from, from_len, to_dir, to, to_len, flags);

  probe.renames++;
  if (probe.rename) return probe.rename;
  return sp_sys_rename_p(from_dir, from, from_len, to_dir, to, to_len, 0);
}

static sp_err_t probed_link(sp_sys_dir_t from_dir, const c8* existing, u32 existing_len, sp_sys_dir_t to_dir, const c8* alias, u32 alias_len) {
  probe.links++;
  if (probe.link) return probe.link;
  return sp_sys_link_p(from_dir, existing, existing_len, to_dir, alias, alias_len);
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  if (c->rename == SP_ERR_SYS_UNSUPPORTED && !c->link) sp_test_skip_without_hard_links();

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, c->setup));
  sp_path_t path = sp_path_join(sp_test_arena(t), sandbox, sp_str_lit("A"));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.rename = probed_rename;
  vt.link = probed_link;
  probe = (probe_t) { .rename = c->rename, .link = c->link };

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_err_t err = sp_fs_write_at(path, sp_str_lit("B"), SP_FS_WRITE_EXCLUSIVE, sp_sys_default_file_perms);
  sp_sys_set_vtable(saved);

  sp_expect_err_eq(t, err, c->expect.err);
  sp_expect_eq(t, probe.renames, c->expect.renames);
  sp_expect_eq(t, probe.links, c->expect.links);
  fs_expect_paths(t, sandbox, c->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each_fn(fs, publish, test_t, tests, run, .serial = true);
