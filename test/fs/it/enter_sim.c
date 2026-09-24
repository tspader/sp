#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

#define MAX_YIELDS 6

typedef struct {
  const c8* path;
  sp_err_t err;
} enter_t;

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
  sp_fs_it_yield_t yield;
} yield_t;

typedef struct {
  yield_t yields [MAX_YIELDS];
  u32 opens;
} expect_t;

typedef struct {
  const c8* name;
  sim_dir_t dirs [SIM_MAX_DIRS];
  enter_t enter [MAX_YIELDS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "leaves_after_children_before_siblings",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "T/B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .enter = { { "T/B" } },
    .expect = {
      .yields = {
        { "T/B", SP_FS_KIND_DIR },
        { "T/B/D", SP_FS_KIND_FILE },
        { "T/B", SP_FS_KIND_DIR, SP_FS_IT_LEAVE },
        { "T/C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "pruned_dir_is_never_opened",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .yields = {
        { "T/B", SP_FS_KIND_DIR },
      },
      .opens = 1,
    },
  },
  {
    .name = "enter_ignores_kind_hint",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_FILE } } },
      { .path = "T/B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .enter = { { "T/B" } },
    .expect = {
      .yields = {
        { "T/B", SP_FS_KIND_FILE },
        { "T/B/C", SP_FS_KIND_FILE },
        { "T/B", SP_FS_KIND_DIR, SP_FS_IT_LEAVE },
      },
      .opens = 2,
    },
  },
  {
    .name = "enter_error_is_returned_and_walk_continues",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "T/B", .open = SP_ERR_SYS_IO },
    },
    .enter = { { "T/B", SP_ERR_SYS_IO } },
    .expect = {
      .yields = {
        { "T/B", SP_FS_KIND_DIR },
        { "T/C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
};

static const enter_t* find_enter(const test_t* test, sp_str_t path) {
  sp_carr_for(test->enter, e) {
    const enter_t* enter = &test->enter[e];
    if (!enter->path) break;
    if (sp_str_equal(sp_cstr_as_str(enter->path), path)) return enter;
  }
  return SP_NULLPTR;
}

sp_test_each(fs, it_enter_sim, test_t, tests, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);

  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  u32 leaves = 0;
  u32 produced = 0;
  sp_fs_it_t walk = sp_fs_it_new_at(mem, (sp_path_t) { .dir = sp_sys_get_root(0), .sub = sp_str_lit("T") }, 0);
  while (sp_fs_it_next(&walk)) {
    if (produced < MAX_YIELDS && it->expect.yields[produced].path) {
      const yield_t* want = &it->expect.yields[produced];
      sp_expect_str_eq_c(t, walk.entry.path, want->path);
      sp_expect_eq(t, (u32)walk.entry.kind, (u32)want->kind);
      sp_expect_eq(t, (u32)walk.yield, (u32)want->yield);
    }
    else {
      sp_test_fail(t, "walker produced unexpected yield {}", sp_fmt_str(walk.entry.path));
    }
    produced++;

    switch (walk.yield) {
      case SP_FS_IT_LEAVE: {
        leaves++;
        sp_expect_eq(t, s.count.closes, leaves);
        break;
      }
      case SP_FS_IT_ENTRY: {
        const enter_t* enter = find_enter(it, walk.entry.path);
        if (enter) sp_expect_err_eq(t, sp_fs_it_enter(&walk), enter->err);
        break;
      }
    }
  }
  sp_expect_ok(t, walk.err);
  sp_fs_it_deinit(&walk);

  sim_end(&s);

  u32 expected = 0;
  sp_carr_for(it->expect.yields, e) {
    if (!it->expect.yields[e].path) break;
    expected++;
  }
  sp_expect_eq(t, produced, expected);
  sp_expect_eq(t, s.count.opens, it->expect.opens);
  sp_expect_eq(t, s.count.nofollow, s.count.opens - 1);
  sp_expect_eq(t, s.count.closes, s.count.dirs);
  return SP_OK;
}
