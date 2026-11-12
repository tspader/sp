#include "fs.h"

typedef struct {
  const c8* path;
  sp_err_t err;
  bool vanish;
} enter_t;

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  entry_t entries [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  enter_t enter [FS_MAX_PATHS];
  const c8* skip [FS_MAX_PATHS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "descends_without_enter",
    .setup = {
      { "A/B" },
      { "C" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_FILE },
        { "C", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "skip_prunes_dir",
    .setup = {
      { "A/B" },
      { "C" },
    },
    .skip = { "A" },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "C", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "nested_enters",
    .setup = {
      { "A/B/C" },
    },
    .enter = { { "A" }, { "A/B" } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_DIR },
        { "A/B/C", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "enter_file_is_not_dir",
    .setup = {
      { "A" },
      { "B" },
    },
    .enter = { { "A", SP_ERR_SYS_NOT_DIR } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "enter_symlink_is_loop",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .enter = { { "L", SP_ERR_SYS_LOOP } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "L", SP_FS_KIND_SYMLINK },
      },
    },
  },
  {
    .name = "enter_vanished_is_not_found",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "C" },
    },
    .enter = { { "A", SP_ERR_SYS_NOT_FOUND, .vanish = true } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "C", SP_FS_KIND_FILE },
      },
    },
  },
};

static const enter_t* find_enter(const test_t* test, sp_str_t path) {
  sp_carr_for(test->enter, e) {
    const enter_t* enter = &test->enter[e];
    if (!enter->path) break;
    if (sp_str_equal_cstr(path, enter->path)) return enter;
  }
  return SP_NULLPTR;
}

static bool is_skipped(const test_t* test, sp_str_t path) {
  sp_carr_for(test->skip, s) {
    if (!test->skip[s]) break;
    if (sp_str_equal_cstr(path, test->skip[s])) return true;
  }
  return false;
}

sp_test_each(fs, it_enter, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  fs_match_t entries [FS_MAX_PATHS] = sp_zero;
  u32 num_entries = 0;
  sp_carr_for(it->expect.entries, e) {
    const entry_t* want = &it->expect.entries[e];
    if (!want->path) break;
    entries[num_entries++] = (fs_match_t) { .key = sp_cstr_as_str(want->path), .kind = want->kind };
  }

  fs_match_t leaves [FS_MAX_PATHS] = sp_zero;
  u32 num_leaves = 0;
  sp_str_t entered [FS_MAX_PATHS] = sp_zero;
  u32 depth = 0;

  sp_sys_dir_t root = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &root));

  sp_fs_it_t walk = sp_fs_it(mem, root);
  sp_fs_visit_t visit = sp_zero;
  while (sp_fs_it_visit(&walk, &visit)) {
    sp_fs_it_entry_t entry = visit.entry;
    switch (visit.order) {
      case SP_FS_POST_ORDER: {
        fs_match(t, leaves, num_leaves, entry.path, entry.kind);
        sp_expect_eq(t, (u32)sp_fs_get_kind_at(entry.at), (u32)SP_FS_KIND_DIR);
        sp_must(t, depth > 0);
        sp_expect_str_eq(t, entry.path, entered[--depth]);
        break;
      }
      case SP_FS_PRE_ORDER: {
        fs_match(t, entries, num_entries, entry.path, entry.kind);
        bool descend = entry.kind == SP_FS_KIND_DIR;
        if (is_skipped(it, entry.path)) {
          sp_fs_it_skip(&walk);
          descend = false;
        }
        const enter_t* enter = find_enter(it, entry.path);
        if (enter) {
          if (enter->vanish) sp_must_ok(t, sp_sys_rmdir_s(entry.at.dir, entry.at.sub));
          sp_expect_err_eq(t, sp_fs_it_enter(&walk), enter->err);
          descend = !enter->err;
        }
        if (descend) {
          sp_str_t path = sp_str_copy(mem, entry.path);
          entered[depth++] = path;
          leaves[num_leaves++] = (fs_match_t) { .key = path, .kind = SP_FS_KIND_DIR };
        }
        break;
      }
    }
  }
  sp_expect_ok(t, walk.err);
  sp_expect_eq(t, depth, (u32)0);
  sp_fs_it_deinit(&walk);
  sp_fs_close_dir(root);

  fs_match_finish(t, entries, num_entries);
  fs_match_finish(t, leaves, num_leaves);
  return SP_OK;
}
