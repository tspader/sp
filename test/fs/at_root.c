#include "sp.h"
#include "sp/sp_test.h"

#define AT_ROOT_MAX_ROOTS 4
#define AT_ROOT_FD_BASE 9000

typedef struct {
  const c8* name;
  sp_err_t err;
} at_root_root_t;

typedef struct {
  s32 root;
  bool invalid;
  const c8* sub;
} at_root_expect_t;

typedef struct {
  const c8* name;
  at_root_root_t roots [AT_ROOT_MAX_ROOTS];
  const c8* sub;
  at_root_expect_t expect;
} at_root_test_t;

static const at_root_test_t* active;

static s32 num_roots(void) {
  s32 n = 0;
  sp_carr_for(active->roots, it) {
    if (!active->roots[it].name && !active->roots[it].err) break;
    n++;
  }
  return n;
}

static sp_sys_fd_t fake_get_root(s32 it) {
  if (it < 0 || it >= num_roots()) return SP_SYS_INVALID_FD;
  return (sp_sys_fd_t)(AT_ROOT_FD_BASE + it);
}

static sp_err_t fake_get_root_name(s32 it, c8* buf, u64 size, u64* len) {
  *len = 0;
  if (it < 0 || it >= num_roots()) return SP_ERR_SYS_BAD_FD;

  const at_root_root_t* root = &active->roots[it];
  if (root->err) return root->err;

  u64 n = sp_cstr_len(root->name);
  if (n >= size) return SP_ERR_SYS_NAME_TOO_LONG;
  sp_mem_copy(buf, root->name, n);
  buf[n] = 0;
  *len = n;
  return SP_OK;
}

static const at_root_test_t tests [] = {
  {
    .name = "unlabeled_root_takes_relative",
    .roots = { { .err = SP_ERR_SYS_UNSUPPORTED } },
    .sub = "x/y",
    .expect = { .sub = "x/y" },
  },
  {
    .name = "unlabeled_root_takes_absolute",
    .roots = { { .err = SP_ERR_SYS_UNSUPPORTED } },
    .sub = "/etc/x",
    .expect = { .sub = "/etc/x" },
  },
  {
    .name = "relative_ignores_labels",
    .roots = { { .name = "." }, { .name = "/cfg" } },
    .sub = "cfg/a",
    .expect = { .sub = "cfg/a" },
  },
  {
    .name = "label_prefix_is_stripped",
    .roots = { { .name = "/bar" } },
    .sub = "/bar/x",
    .expect = { .sub = "x" },
  },
  {
    .name = "exact_label_is_dot",
    .roots = { { .name = "/bar" } },
    .sub = "/bar",
    .expect = { .sub = "." },
  },
  {
    .name = "trailing_separator_is_dot",
    .roots = { { .name = "/bar" } },
    .sub = "/bar/",
    .expect = { .sub = "." },
  },
  {
    .name = "label_matches_whole_components",
    .roots = { { .name = "/bar" } },
    .sub = "/barn/x",
    .expect = { .sub = "/barn/x" },
  },
  {
    .name = "longest_label_wins",
    .roots = { { .name = "/bar" }, { .name = "/bar/x" } },
    .sub = "/bar/x/y",
    .expect = { .root = 1, .sub = "y" },
  },
  {
    .name = "longest_label_wins_regardless_of_order",
    .roots = { { .name = "/bar/x" }, { .name = "/bar" } },
    .sub = "/bar/x/y",
    .expect = { .sub = "y" },
  },
  {
    .name = "slash_label_takes_everything",
    .roots = { { .name = "/" } },
    .sub = "/x/y",
    .expect = { .sub = "x/y" },
  },
  {
    .name = "label_trailing_separator_is_ignored",
    .roots = { { .name = "/bar/" } },
    .sub = "/bar/x",
    .expect = { .sub = "x" },
  },
  {
    .name = "dot_label_does_not_match_absolute",
    .roots = { { .name = "." }, { .name = "/cfg" } },
    .sub = "/cfg/a",
    .expect = { .root = 1, .sub = "a" },
  },
  {
    .name = "unmatched_absolute_falls_back_to_cwd",
    .roots = { { .name = "." }, { .name = "/cfg" } },
    .sub = "/other",
    .expect = { .sub = "/other" },
  },
  {
    .name = "unlabeled_root_never_wins",
    .roots = { { .err = SP_ERR_SYS_UNSUPPORTED }, { .name = "/bar" } },
    .sub = "/bar/x",
    .expect = { .root = 1, .sub = "x" },
  },
  {
    .name = "no_roots_is_invalid",
    .sub = "x",
    .expect = { .invalid = true, .sub = "x" },
  },
};

static sp_err_t run(sp_test_t* t, at_root_test_t* c) {
  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_root = fake_get_root;
  vt.get_root_name = fake_get_root_name;

  active = c;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  sp_path_t path = sp_path_at_root(sp_cstr_as_str(c->sub));
  sp_sys_set_vtable(saved);
  active = SP_NULLPTR;

  sp_sys_fd_t dir = c->expect.invalid ? SP_SYS_INVALID_FD : (sp_sys_fd_t)(AT_ROOT_FD_BASE + c->expect.root);
  sp_expect_eq(t, path.dir, dir);
  sp_expect_str_eq(t, path.sub, sp_cstr_as_str(c->expect.sub));
  return SP_OK;
}

sp_test_each_fn(fs, at_root, at_root_test_t, tests, run, .serial = true);
