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

static s32 entry(s32 argc, const c8** argv) {
  return sp_test_main(argc, argv, entries);
}

SP_MAIN(entry)
