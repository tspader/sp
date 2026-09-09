#define SP_IMPLEMENTATION
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  s64 value;
} row_t;

static sp_err_t run_fn(sp_test_t* t);
static sp_err_t run_each(sp_test_t* t, const void* arg);

static const s32 token = 33;

static const row_t rows [] = {
  { .name = "R", .value = 1 },
  { .name = "S", .value = 2 },
};

static const sp_test_entry_t entries [] = {
  {
    .kind = SP_TEST_ENTRY_SUITE,
    .suite = { .name = "manual", .serial = true },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = {
      .suite = "manual",
      .name = "fn",
      .kind = SP_TEST_DECL_FN,
      .fn = run_fn,
      .user = &token,
    },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = {
      .suite = "manual",
      .name = "each",
      .kind = SP_TEST_DECL_EACH,
      .each = {
        .fn = run_each,
        .cases = rows,
        .stride = sizeof(row_t),
        .count = sp_carr_len(rows),
        .named = true,
        .case_name_offset = offsetof(row_t, name),
      },
    },
  },
  sp_zero,
};

static sp_err_t run_fn(sp_test_t* t) {
  sp_expect_str_eq_c(t, sp_test_get_name(t), "manual.fn");
  sp_expect(t, sp_test_user(t) == &token);
  return SP_OK;
}

static sp_err_t run_each(sp_test_t* t, const void* arg) {
  const row_t* row = (const row_t*)arg;
  sp_expect_str_eq(t, sp_test_get_name(t), sp_test_format(t, "manual.each.{}", sp_fmt_cstr(row->name)));

  const row_t* want = SP_NULLPTR;
  sp_carr_for(rows, it) {
    if (sp_cstr_equal(rows[it].name, row->name)) want = &rows[it];
  }
  sp_must(t, want != SP_NULLPTR);
  sp_expect_eq(t, row->value, want->value);
  return SP_OK;
}

static sp_err_t child_pass(sp_test_t* t);
static sp_err_t child_fail(sp_test_t* t);
static sp_err_t child_leak(sp_test_t* t);
static sp_err_t child_skip(sp_test_t* t);
static sp_err_t child_each(sp_test_t* t, const void* arg);

static const sp_test_entry_t child_entries [] = {
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = { .suite = "child", .name = "pass", .kind = SP_TEST_DECL_FN, .fn = child_pass },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = { .suite = "child", .name = "fail", .kind = SP_TEST_DECL_FN, .fn = child_fail },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = { .suite = "child", .name = "leak", .kind = SP_TEST_DECL_FN, .fn = child_leak },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = { .suite = "child", .name = "skip", .kind = SP_TEST_DECL_FN, .fn = child_skip },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = { .suite = "child", .name = "declared", .kind = SP_TEST_DECL_FN, .fn = child_pass, .keep = true },
  },
  {
    .kind = SP_TEST_ENTRY_TEST,
    .decl = {
      .suite = "child",
      .name = "each",
      .kind = SP_TEST_DECL_EACH,
      .each = {
        .fn = child_each,
        .cases = rows,
        .stride = sizeof(row_t),
        .count = sp_carr_len(rows),
        .named = true,
        .case_name_offset = offsetof(row_t, name),
      },
      .keep = true,
    },
  },
  sp_zero,
};

static sp_err_t child_touch(sp_test_t* t) {
  sp_str_t path = sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  return sp_fs_create_file_cstr(path, "A");
}

static sp_err_t child_pass(sp_test_t* t) {
  sp_must_ok(t, child_touch(t));
  return SP_OK;
}

static sp_err_t child_fail(sp_test_t* t) {
  sp_must_ok(t, child_touch(t));
  sp_test_fail(t, "F");
  return SP_OK;
}

static sp_err_t child_leak(sp_test_t* t) {
  sp_must_ok(t, child_touch(t));
  (void)sp_alloc(sp_test_mem(t), 8);
  return SP_OK;
}

static sp_err_t child_skip(sp_test_t* t) {
  sp_must_ok(t, child_touch(t));
  return sp_test_skip(t, "S");
}

static sp_err_t child_each(sp_test_t* t, const void* arg) {
  SP_UNUSED(arg);
  sp_must_ok(t, child_touch(t));
  return SP_OK;
}

static s32 entry(s32 argc, const c8** argv) {
  if (argc > 1 && sp_cstr_equal(argv[1], "child")) {
    return sp_test_main(argc - 1, argv + 1, child_entries);
  }
  return sp_test_main(argc, argv, entries);
}

SP_MAIN(entry)
