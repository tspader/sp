#include "sp/sp_test.h"

#define AXIS_MAX 8

typedef enum {
  AXIS_A,
  AXIS_B,
  AXIS_C,
} axis_kind_t;

typedef struct {
  const c8* name;
  axis_kind_t kind;
  u64 n;
  u16 w;
} axis_row_t;

typedef struct {
  const c8* names [AXIS_MAX];
  axis_row_t rows [AXIS_MAX];
} axis_expect_t;

typedef struct {
  const c8* name;
  sp_test_decl_t decl;
  axis_expect_t expect;
} axis_case_t;

static sp_err_t axis_stub(sp_test_t* t, const void* it);
static const c8* axis_kind_name(s64 value);

static const axis_row_t axis_one [] = {
  { .name = "R", .n = 7 },
};

static const axis_row_t axis_two [] = {
  { .name = "R", .n = 1 },
  { .name = "S", .n = 2 },
};

static const axis_case_t axis_cases [] = {
  {
    .name = "values",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_one,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_one),
      .case_name_offset = offsetof(axis_row_t, name) + 1,
      .axes = { sp_test_axis(axis_row_t, kind, AXIS_A, AXIS_C) },
    },
    .expect = {
      .names = { "S.T.R.kind=0", "S.T.R.kind=2" },
      .rows = {
        { .name = "R", .kind = AXIS_A, .n = 7 },
        { .name = "R", .kind = AXIS_C, .n = 7 },
      },
    },
  },
  {
    .name = "named",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_one,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_one),
      .case_name_offset = offsetof(axis_row_t, name) + 1,
      .axes = { sp_test_axis_named(axis_row_t, kind, axis_kind_name, AXIS_A, AXIS_C) },
    },
    .expect = {
      .names = { "S.T.R.kind=A", "S.T.R.kind=C" },
      .rows = {
        { .name = "R", .kind = AXIS_A, .n = 7 },
        { .name = "R", .kind = AXIS_C, .n = 7 },
      },
    },
  },
  {
    .name = "range",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_one,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_one),
      .case_name_offset = offsetof(axis_row_t, name) + 1,
      .axes = { sp_test_axis_range(axis_row_t, w, 0, 6, 3) },
    },
    .expect = {
      .names = { "S.T.R.w=0", "S.T.R.w=3", "S.T.R.w=6" },
      .rows = {
        { .name = "R", .n = 7 },
        { .name = "R", .n = 7, .w = 3 },
        { .name = "R", .n = 7, .w = 6 },
      },
    },
  },
  {
    .name = "cross",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_one,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_one),
      .case_name_offset = offsetof(axis_row_t, name) + 1,
      .axes = {
        sp_test_axis(axis_row_t, kind, AXIS_A, AXIS_B),
        sp_test_axis_range(axis_row_t, w, 1, 2, 1),
      },
    },
    .expect = {
      .names = {
        "S.T.R.kind=0.w=1",
        "S.T.R.kind=0.w=2",
        "S.T.R.kind=1.w=1",
        "S.T.R.kind=1.w=2",
      },
      .rows = {
        { .name = "R", .kind = AXIS_A, .n = 7, .w = 1 },
        { .name = "R", .kind = AXIS_A, .n = 7, .w = 2 },
        { .name = "R", .kind = AXIS_B, .n = 7, .w = 1 },
        { .name = "R", .kind = AXIS_B, .n = 7, .w = 2 },
      },
    },
  },
  {
    .name = "rows",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_two,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_two),
      .case_name_offset = offsetof(axis_row_t, name) + 1,
      .axes = { sp_test_axis(axis_row_t, kind, AXIS_A, AXIS_B) },
    },
    .expect = {
      .names = {
        "S.T.R.kind=0",
        "S.T.R.kind=1",
        "S.T.S.kind=0",
        "S.T.S.kind=1",
      },
      .rows = {
        { .name = "R", .kind = AXIS_A, .n = 1 },
        { .name = "R", .kind = AXIS_B, .n = 1 },
        { .name = "S", .kind = AXIS_A, .n = 2 },
        { .name = "S", .kind = AXIS_B, .n = 2 },
      },
    },
  },
  {
    .name = "sweep",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .stride = sizeof(axis_row_t),
      .axes = { sp_test_axis(axis_row_t, kind, AXIS_B) },
    },
    .expect = {
      .names = { "S.T.kind=1" },
      .rows = {
        { .kind = AXIS_B },
      },
    },
  },
  {
    .name = "anon",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_two,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_two),
      .axes = { sp_test_axis(axis_row_t, kind, AXIS_A) },
    },
    .expect = {
      .names = { "S.T.0.kind=0", "S.T.1.kind=0" },
      .rows = {
        { .name = "R", .kind = AXIS_A, .n = 1 },
        { .name = "S", .kind = AXIS_A, .n = 2 },
      },
    },
  },
  {
    .name = "plain",
    .decl = {
      .name = "T",
      .kind = SP_TEST_DECL_EACH,
      .each = axis_stub,
      .cases = axis_one,
      .stride = sizeof(axis_row_t),
      .count = sp_carr_len(axis_one),
      .case_name_offset = offsetof(axis_row_t, name) + 1,
    },
    .expect = {
      .names = { "S.T.R" },
      .rows = {
        { .name = "R", .n = 7 },
      },
    },
  },
};

static sp_err_t axis_stub(sp_test_t* t, const void* it) {
  SP_UNUSED(t);
  SP_UNUSED(it);
  return SP_OK;
}

static const c8* axis_kind_name(s64 value) {
  switch ((axis_kind_t)value) {
    case AXIS_A: return "A";
    case AXIS_B: return "B";
    case AXIS_C: return "C";
  }
  return "?";
}

static sp_str_t axis_name_str(const c8* name) {
  return sp_cstr_as_str(name ? name : "");
}

sp_test_each(axis, expand, axis_case_t, axis_cases) {
  sp_mem_t mem = sp_test_arena(t);

  sp_da(sp_test_instance_t) instances = sp_da_new(mem, sp_test_instance_t);
  sp_test_expand(mem, "S", &it->decl, false, SP_NULLPTR, &instances);

  u32 want = 0;
  while (want < AXIS_MAX && it->expect.names[want]) want++;
  sp_must_eq(t, (u32)sp_da_size(instances), want);

  sp_for(at, want) {
    sp_test_kv(t, "instance", sp_test_format(t, "{}", sp_fmt_uint(at)));

    sp_expect_str_eq_c(t, sp_cstr_as_str(instances[at].name), it->expect.names[at]);

    const axis_row_t* got = (const axis_row_t*)instances[at].arg;
    const axis_row_t* row = &it->expect.rows[at];
    sp_expect_str_eq(t, axis_name_str(got->name), axis_name_str(row->name));
    sp_expect_eq(t, (u32)got->kind, (u32)row->kind);
    sp_expect_eq(t, got->n, row->n);
    sp_expect_eq(t, got->w, row->w);
  }
  sp_test_kv_clear(t, SP_NULLPTR);

  return SP_OK;
}
