#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

#define MAX_VISITS 6

typedef struct {
  const c8* path;
  sp_err_t err;
  sp_fs_order_t order;
} enter_t;

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
  sp_fs_order_t order;
} visit_t;

typedef struct {
  visit_t visits [MAX_VISITS];
  u32 opens;
} expect_t;

typedef struct {
  const c8* name;
  sim_dir_t dirs [SIM_MAX_DIRS];
  enter_t enter [MAX_VISITS];
  const c8* skip [MAX_VISITS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "leaves_after_children_before_siblings",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .visits = {
        { "B", SP_FS_KIND_DIR },
        { "B/D", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_DIR, SP_FS_POST_ORDER },
        { "C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "pruned_dir_is_never_opened",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .skip = { "B" },
    .expect = {
      .visits = {
        { "B", SP_FS_KIND_DIR },
      },
      .opens = 1,
    },
  },
  {
    .name = "enter_ignores_kind_hint",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_FILE } } },
      { .path = "B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .enter = { { "B" } },
    .expect = {
      .visits = {
        { "B", SP_FS_KIND_FILE },
        { "B/C", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_DIR, SP_FS_POST_ORDER },
      },
      .opens = 2,
    },
  },
  {
    .name = "enter_error_is_returned_and_walk_continues",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .open = SP_ERR_SYS_IO },
    },
    .enter = { { "B", SP_ERR_SYS_IO } },
    .expect = {
      .visits = {
        { "B", SP_FS_KIND_DIR },
        { "C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "enter_at_leave_relists",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .enter = { { "B" }, { "B", .order = SP_FS_POST_ORDER } },
    .expect = {
      .visits = {
        { "B", SP_FS_KIND_DIR },
        { "B/D", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_DIR, SP_FS_POST_ORDER },
        { "B/D", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_DIR, SP_FS_POST_ORDER },
      },
      .opens = 3,
    },
  },
};

static s32 find_enter(const test_t* test, sp_str_t path, sp_fs_order_t order, const bool* used) {
  sp_carr_for(test->enter, e) {
    const enter_t* enter = &test->enter[e];
    if (!enter->path) break;
    if (used[e] || enter->order != order) continue;
    if (sp_str_equal_cstr(path, enter->path)) return (s32)e;
  }
  return -1;
}

static bool is_skipped(const test_t* test, sp_str_t path) {
  sp_carr_for(test->skip, s) {
    if (!test->skip[s]) break;
    if (sp_str_equal_cstr(path, test->skip[s])) return true;
  }
  return false;
}

sp_test_each(fs, it_enter_sim, test_t, tests, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);

  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  sp_sys_dir_t root = sp_zero;

  sp_err_t err = sp_fs_open_dir_at(sim_root(), &root);
  sp_expect_ok(t, err);

  u32 leaves = 0;
  u32 produced = 0;
  bool used [MAX_VISITS] = sp_zero;
  if (!err) {
    sp_fs_it_t walk = sp_fs_it(mem, root);
    sp_fs_visit_t visit = sp_zero;
    while (sp_fs_it_visit(&walk, &visit)) {
      sp_fs_it_entry_t entry = visit.entry;
      if (produced < MAX_VISITS && it->expect.visits[produced].path) {
        const visit_t* want = &it->expect.visits[produced];
        sp_expect_str_eq_c(t, entry.path, want->path);
        sp_expect_eq(t, (u32)entry.kind, (u32)want->kind);
        sp_expect_eq(t, (u32)visit.order, (u32)want->order);
      }
      else {
        sp_test_fail(t, "walker produced unexpected visit {}", sp_fmt_str(entry.path));
      }
      sp_expect_str_eq(t, entry.name, sp_fs_get_name(entry.path));
      sp_expect_str_eq(t, entry.at.sub, entry.name);
      produced++;

      if (visit.order == SP_FS_POST_ORDER) {
        leaves++;
        sp_expect_eq(t, s.count.closes, leaves);
      }

      s32 enter = find_enter(it, entry.path, visit.order, used);
      if (enter >= 0) {
        used[enter] = true;
        sp_expect_err_eq(t, sp_fs_it_enter(&walk), it->enter[enter].err);
      }
      else if (visit.order == SP_FS_PRE_ORDER && is_skipped(it, entry.path)) {
        sp_fs_it_skip(&walk);
      }
    }
    sp_expect_ok(t, walk.err);
    sp_fs_it_deinit(&walk);
    sp_fs_close_dir(root);
  }

  sim_end(&s);

  u32 expected = 0;
  sp_carr_for(it->expect.visits, e) {
    if (!it->expect.visits[e].path) break;
    expected++;
  }
  sp_expect_eq(t, produced, expected);
  sp_expect_eq(t, s.count.opens, it->expect.opens);
  sp_expect_eq(t, s.count.nofollow, s.count.opens - 1);
  sp_expect_eq(t, s.count.closes, s.count.dirs);
  sp_expect_eq(t, s.count.fd_closes, s.count.dirs);
  return SP_OK;
}
