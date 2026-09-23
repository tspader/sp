#include "fs.h"

typedef struct {
  const c8* path;
  sp_err_t err;
  bool remove;
} enter_t;

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  entry_t entries [FS_MAX_PATHS];
  const c8* leaves [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  enter_t enter [FS_MAX_PATHS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "enters_only_listed",
    .setup = {
      { "R/A/B" },
      { "R/C/D" },
      { "R/E" },
    },
    .enter = { { "A" } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_FILE },
        { "C", SP_FS_KIND_DIR },
        { "E", SP_FS_KIND_FILE },
      },
      .leaves = { "A" },
    },
  },
  {
    .name = "leaves_in_stack_order",
    .setup = {
      { "R/A/B/C" },
    },
    .enter = { { "A" }, { "A/B" } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_DIR },
        { "A/B/C", SP_FS_KIND_FILE },
      },
      .leaves = { "A/B", "A" },
    },
  },
  {
    .name = "no_enter_no_leave",
    .setup = {
      { "R/A/B" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "enter_file_is_not_dir",
    .setup = {
      { "R/A" },
      { "R/B" },
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
      { "R/A", FS_SETUP_DIR },
      { "R/A/B" },
      { .path = "R/L", .kind = FS_SETUP_SYMLINK, .target = "R/A" },
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
      { "R/A/B" },
      { "R/C" },
    },
    .enter = { { "A", SP_ERR_SYS_NOT_FOUND, .remove = true } },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "C", SP_FS_KIND_FILE },
      },
    },
  },
};

static const enter_t* find_enter(sp_mem_t mem, const test_t* test, sp_str_t base, sp_str_t path) {
  sp_carr_for(test->enter, e) {
    const enter_t* enter = &test->enter[e];
    if (!enter->path) break;
    if (sp_str_equal(sp_fs_join_path(mem, base, sp_cstr_as_str(enter->path)), path)) return enter;
  }
  return SP_NULLPTR;
}

sp_test_each(fs, it_enter, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, 0, &dir));

  sp_str_t base = sp_str_lit("R");
  fs_match_t entries [FS_MAX_PATHS] = sp_zero;
  u32 num_entries = 0;
  sp_carr_for(it->expect.entries, e) {
    const entry_t* want = &it->expect.entries[e];
    if (!want->path) break;
    entries[num_entries++] = (fs_match_t) { .key = sp_fs_join_path(mem, base, sp_cstr_as_str(want->path)), .kind = want->kind };
  }

  fs_match_t leaves [FS_MAX_PATHS] = sp_zero;
  u32 num_leaves = 0;
  sp_carr_for(it->expect.leaves, e) {
    if (!it->expect.leaves[e]) break;
    leaves[num_leaves++] = (fs_match_t) { .key = sp_fs_join_path(mem, base, sp_cstr_as_str(it->expect.leaves[e])), .kind = SP_FS_KIND_DIR };
  }

  sp_str_t entered [FS_MAX_PATHS] = sp_zero;
  u32 depth = 0;

  sp_fs_it_t walk = sp_fs_it_new_at(mem, (sp_path_t) { .dir = dir, .sub = base }, 0);
  while (sp_fs_it_next(&walk)) {
    sp_expect_str_eq(t, walk.at.sub, walk.entry.name);
    switch (walk.yield) {
      case SP_FS_IT_LEAVE: {
        fs_match(t, leaves, num_leaves, walk.entry.path, SP_FS_KIND_DIR);
        sp_expect_eq(t, (u32)sp_fs_get_kind_at(walk.at), (u32)SP_FS_KIND_DIR);
        sp_must(t, depth > 0);
        sp_expect_str_eq(t, walk.entry.path, entered[--depth]);
        break;
      }
      case SP_FS_IT_ENTRY: {
        fs_match(t, entries, num_entries, walk.entry.path, walk.entry.kind);
        const enter_t* enter = find_enter(mem, it, base, walk.entry.path);
        if (!enter) break;
        if (enter->remove) sp_must_ok(t, sp_fs_remove_dir_at(walk.at));
        sp_expect_err_eq(t, sp_fs_it_enter(&walk), enter->err);
        if (!enter->err) entered[depth++] = sp_str_copy(mem, walk.entry.path);
        break;
      }
    }
  }
  sp_expect_ok(t, walk.err);
  sp_expect_eq(t, depth, (u32)0);
  sp_fs_it_deinit(&walk);
  sp_sys_close(dir);

  fs_match_finish(t, entries, num_entries);
  fs_match_finish(t, leaves, num_leaves);
  return SP_OK;
}
