#include "fs.h"

typedef enum {
  OP_COPY_FILE,
  OP_COPY_TREE,
} op_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  op_t op;
  sp_fs_write_mode_t mode;
  const c8* src;
  const c8* dst;
  const c8* probed;
} test_t;

static const test_t tests [] = {
  {
    .name = "truncate_to_fresh_name",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "B",
    .probed = "B",
  },
  {
    .name = "link_to_fresh_name",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .op = OP_COPY_TREE,
    .mode = SP_FS_WRITE_REPLACE,
    .src = "A",
    .dst = "E",
    .probed = "L",
  },
};

typedef struct {
  sp_str_t name;
  u32 stats;
} probe_t;

static probe_t probe;

static sp_err_t probed_path_metadata(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_meta_t* meta) {
  if (sp_str_equal(sp_fs_get_name(sp_str(path, len)), probe.name)) probe.stats++;
  return sp_sys_get_path_metadata_p(dir, path, len, meta);
}

static sp_err_t probed_link_metadata(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_meta_t* meta) {
  if (sp_str_equal(sp_fs_get_name(sp_str(path, len)), probe.name)) probe.stats++;
  return sp_sys_get_link_metadata_p(dir, path, len, meta);
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  if (fs_setup_needs_symlinks(c->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, c->setup));
  sp_path_t from = sp_path_join(mem, sandbox, sp_cstr_as_str(c->src));
  sp_path_t to = sp_path_join(mem, sandbox, sp_cstr_as_str(c->dst));

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_path_metadata = probed_path_metadata;
  vt.get_link_metadata = probed_link_metadata;
  probe = (probe_t) { .name = sp_cstr_as_str(c->probed) };

  sp_err_t err = SP_OK;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  switch (c->op) {
    case OP_COPY_FILE: err = sp_fs_copy_file_at(from, to, c->mode); break;
    case OP_COPY_TREE: err = sp_fs_copy_tree_at(from, to, c->mode); break;
  }
  sp_sys_set_vtable(saved);

  sp_expect_ok(t, err);
  sp_expect_eq(t, probe.stats, 0);
  return SP_OK;
}

sp_test_each_fn(fs, copy_stat, test_t, tests, run, .serial = true);
