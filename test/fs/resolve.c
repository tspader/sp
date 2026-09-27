#include "sp.h"
#include "sp/sp_test.h"

#define MAX_ROOTS 4
#define FD_BASE 9000

typedef struct {
  const c8* label;
  sp_err_t err;
} root_t;

typedef struct {
  s32 root;
  bool invalid;
  const c8* sub;
} expect_t;

typedef struct {
  const c8* name;
  root_t roots [MAX_ROOTS];
  const c8* path;
  expect_t expect;
} test_t;

static const test_t* active;

static s32 num_roots() {
  s32 n = 0;
  sp_carr_for(active->roots, it) {
    if (!active->roots[it].label && !active->roots[it].err) break;
    n++;
  }
  return n;
}

static sp_sys_fd_t get_root(s32 it) {
  if (it < 0 || it >= num_roots()) return SP_SYS_INVALID_FD;
  return (sp_sys_fd_t)(FD_BASE + it);
}

static sp_err_t get_root_label(s32 it, c8* buf, u64 size, u64* len) {
  *len = 0;
  if (it < 0 || it >= num_roots()) return SP_ERR_SYS_BAD_FD;

  const root_t* root = &active->roots[it];
  if (root->err) return root->err;

  u64 n = sp_cstr_len(root->label);
  if (n >= size) return SP_ERR_SYS_NAME_TOO_LONG;
  sp_mem_copy(buf, root->label, n);
  buf[n] = 0;
  *len = n;
  return SP_OK;
}

static const test_t tests [] = {
  {
    .name = "relative_goes_to_cwd",
    .roots = { { .label = "/A" } },
    .path = "A/B",
    .expect = { .sub = "A/B" },
  },
  {
    .name = "label_is_stripped",
    .roots = { { .label = "/B" }, { .label = "/A" } },
    .path = "/A/B",
    .expect = { .root = 1, .sub = "B" },
  },
  {
    .name = "separator_run_is_stripped",
    .roots = { { .label = "/B" }, { .label = "/A" } },
    .path = "/A//B",
    .expect = { .root = 1, .sub = "B" },
  },
  {
    .name = "exact_label_is_dot",
    .roots = { { .label = "/B" }, { .label = "/A" } },
    .path = "/A",
    .expect = { .root = 1, .sub = "." },
  },
  {
    .name = "trailing_separator_is_dot",
    .roots = { { .label = "/B" }, { .label = "/A" } },
    .path = "/A/",
    .expect = { .root = 1, .sub = "." },
  },
  {
    .name = "label_trailing_separator_is_ignored",
    .roots = { { .label = "/B" }, { .label = "/A/" } },
    .path = "/A/B",
    .expect = { .root = 1, .sub = "B" },
  },
  {
    .name = "label_matches_whole_components",
    .roots = { { .label = "/B" }, { .label = "/A" } },
    .path = "/AB/C",
    .expect = { .sub = "/AB/C" },
  },
  {
    .name = "deepest_label_wins",
    .roots = { { .label = "/A" }, { .label = "/A/B" } },
    .path = "/A/B/C",
    .expect = { .root = 1, .sub = "C" },
  },
  {
    .name = "deepest_label_wins_in_any_order",
    .roots = { { .label = "/A/B" }, { .label = "/A" } },
    .path = "/A/B/C",
    .expect = { .sub = "C" },
  },
  {
    .name = "slash_label_takes_everything",
    .roots = { { .label = "/B" }, { .label = "/" } },
    .path = "/A/B",
    .expect = { .root = 1, .sub = "A/B" },
  },
  {
    .name = "empty_label_takes_nothing",
    .roots = { { .label = "" } },
    .path = "/A/B",
    .expect = { .sub = "/A/B" },
  },
  {
    .name = "relative_label_never_matches",
    .roots = { { .label = "A" }, { .label = "/A" } },
    .path = "/A/B",
    .expect = { .root = 1, .sub = "B" },
  },
  {
    .name = "unmatched_absolute_goes_to_cwd",
    .roots = { { .label = "/B" }, { .label = "/A" } },
    .path = "/C",
    .expect = { .sub = "/C" },
  },
  {
    .name = "unreadable_label_is_skipped",
    .roots = { { .err = SP_ERR_SYS_NAME_TOO_LONG }, { .label = "/A" } },
    .path = "/A/B",
    .expect = { .root = 1, .sub = "B" },
  },
  {
    .name = "no_roots",
    .path = "A",
    .expect = { .invalid = true, .sub = "A" },
  },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_root = get_root;
  vt.get_root_label = get_root_label;

  active = c;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_path_t path = sp_path_resolve(sp_cstr_as_str(c->path));
  sp_sys_set_vtable(saved);
  active = SP_NULLPTR;

  sp_sys_fd_t dir = c->expect.invalid ? SP_SYS_INVALID_FD : (sp_sys_fd_t)(FD_BASE + c->expect.root);
  sp_expect_eq(t, path.dir, dir);
  sp_expect_str_eq(t, path.sub, sp_cstr_as_str(c->expect.sub));
  return SP_OK;
}

sp_test_each_fn(fs, resolve, test_t, tests, run, .serial = true);
