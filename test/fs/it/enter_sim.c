#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

#define MAX_YIELDS 6

typedef struct {
  const c8* path;
  sp_err_t err;
  sp_fs_it_yield_t yield;
} enter_t;

typedef struct {
  const c8* path;
  const c8* name;
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
        { "T/B", "B", SP_FS_KIND_DIR },
        { "T/B/D", "D", SP_FS_KIND_FILE },
        { "T/B", "B", SP_FS_KIND_DIR, SP_FS_IT_LEAVE },
        { "T/C", "C", SP_FS_KIND_FILE },
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
        { "T/B", "B", SP_FS_KIND_DIR },
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
        { "T/B", "B", SP_FS_KIND_FILE },
        { "T/B/C", "C", SP_FS_KIND_FILE },
        { "T/B", "B", SP_FS_KIND_DIR, SP_FS_IT_LEAVE },
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
        { "T/B", "B", SP_FS_KIND_DIR },
        { "T/C", "C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "enter_at_leave_relists",
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .enter = { { "T/B" }, { "T/B", .yield = SP_FS_IT_LEAVE } },
    .expect = {
      .yields = {
        { "T/B", "B", SP_FS_KIND_DIR },
        { "T/B/D", "D", SP_FS_KIND_FILE },
        { "T/B", "B", SP_FS_KIND_DIR, SP_FS_IT_LEAVE },
        { "T/B/D", "D", SP_FS_KIND_FILE },
        { "T/B", "B", SP_FS_KIND_DIR, SP_FS_IT_LEAVE },
      },
      .opens = 3,
    },
  },
};

static s32 find_enter(const test_t* test, sp_str_t path, sp_fs_it_yield_t yield, const bool* used) {
  sp_carr_for(test->enter, e) {
    const enter_t* enter = &test->enter[e];
    if (!enter->path) break;
    if (used[e] || enter->yield != yield) continue;
    if (sp_str_equal(sp_cstr_as_str(enter->path), path)) return (s32)e;
  }
  return -1;
}

sp_test_each(fs, it_enter_sim, test_t, tests, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);

  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  u32 leaves = 0;
  u32 produced = 0;
  bool used [MAX_YIELDS] = sp_zero;
  sp_fs_it_t walk = sp_fs_it_new_at(mem, sp_path(sp_sys_get_root(0), sp_str_lit("T")), 0);
  while (sp_fs_it_next(&walk)) {
    if (produced < MAX_YIELDS && it->expect.yields[produced].path) {
      const yield_t* want = &it->expect.yields[produced];
      sp_expect_str_eq_c(t, walk.entry.path, want->path);
      sp_expect_str_eq_c(t, walk.entry.name, want->name);
      sp_expect_str_eq(t, walk.at.sub, walk.entry.name);
      sp_expect_str_eq(t, sp_fs_join_path(mem, sp_str_lit("T"), walk.entry.rel), walk.entry.path);
      sp_expect_eq(t, (u32)walk.entry.kind, (u32)want->kind);
      sp_expect_eq(t, (u32)walk.yield, (u32)want->yield);
    }
    else {
      sp_test_fail(t, "walker produced unexpected yield {}", sp_fmt_str(walk.entry.path));
    }
    produced++;

    if (walk.yield == SP_FS_IT_LEAVE) {
      leaves++;
      sp_expect_eq(t, s.count.closes, leaves);
    }

    s32 enter = find_enter(it, walk.entry.path, walk.yield, used);
    if (enter >= 0) {
      used[enter] = true;
      sp_expect_err_eq(t, sp_fs_it_enter(&walk), it->enter[enter].err);
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
