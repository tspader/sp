#include "sp/sp_test.h"

typedef enum {
  MAKE_AS_STR,
  MAKE_FROM_CSTR,
  MAKE_FROM_CSTR_N,
  MAKE_COPY,
  MAKE_TO_CSTR,
  MAKE_CSTR_FROM_STR,
} make_t;

typedef struct {
  const c8* value;
  bool aliases;
} expect_t;

typedef struct {
  const c8* name;
  make_t kind;
  const c8* input;
  u32 n;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "as_str",
    .kind = MAKE_AS_STR,
    .input = "hello world",
    .expect = { .value = "hello world", .aliases = true }
  },
  {
    .name = "as_str_empty",
    .kind = MAKE_AS_STR,
    .input = "",
    .expect = { .value = "", .aliases = true }
  },
  {
    .name = "as_str_null",
    .kind = MAKE_AS_STR,
    .expect = { .value = "", .aliases = true }
  },
  {
    .name = "from_cstr",
    .kind = MAKE_FROM_CSTR,
    .input = "hello world",
    .expect = { .value = "hello world" }
  },
  {
    .name = "from_cstr_empty",
    .kind = MAKE_FROM_CSTR,
    .input = "",
    .expect = { .value = "" }
  },
  {
    .name = "from_cstr_null",
    .kind = MAKE_FROM_CSTR,
    .expect = { .value = "", .aliases = true }
  },
  {
    .name = "from_cstr_n",
    .kind = MAKE_FROM_CSTR_N,
    .input = "hello world",
    .n = 5,
    .expect = { .value = "hello" }
  },
  {
    .name = "from_cstr_n_clamped",
    .kind = MAKE_FROM_CSTR_N,
    .input = "hi",
    .n = 10,
    .expect = { .value = "hi" }
  },
  {
    .name = "from_cstr_n_zero",
    .kind = MAKE_FROM_CSTR_N,
    .input = "hello",
    .expect = { .value = "" }
  },
  {
    .name = "copy",
    .kind = MAKE_COPY,
    .input = "hello world",
    .expect = { .value = "hello world" }
  },
  {
    .name = "copy_empty",
    .kind = MAKE_COPY,
    .input = "",
    .expect = { .value = "" }
  },
  {
    .name = "to_cstr",
    .kind = MAKE_TO_CSTR,
    .input = "hello world",
    .expect = { .value = "hello world" }
  },
  {
    .name = "to_cstr_empty",
    .kind = MAKE_TO_CSTR,
    .input = "",
    .expect = { .value = "" }
  },
  {
    .name = "cstr_from_str",
    .kind = MAKE_CSTR_FROM_STR,
    .input = "hello world",
    .expect = { .value = "hello world" }
  },
  {
    .name = "cstr_from_str_empty",
    .kind = MAKE_CSTR_FROM_STR,
    .input = "",
    .expect = { .value = "" }
  },
};

sp_test_each(str, make, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_str_t result = sp_zero;

  switch (it->kind) {
    case MAKE_AS_STR:        { result = input; break; }
    case MAKE_FROM_CSTR:     { result = sp_str_from_cstr(mem, it->input); break; }
    case MAKE_FROM_CSTR_N:   { result = sp_str_from_cstr_n(mem, it->input, it->n); break; }
    case MAKE_COPY:          { result = sp_str_copy(mem, input); break; }
    case MAKE_TO_CSTR:       { result = sp_cstr_as_str(sp_str_to_cstr(mem, input)); break; }
    case MAKE_CSTR_FROM_STR: { result = sp_cstr_as_str(sp_cstr_from_str(mem, input)); break; }
  }

  bool aliases = result.data == it->input;
  sp_expect_str_eq_c(t, result, it->expect.value);
  sp_expect_eq(t, aliases, it->expect.aliases);
  return SP_OK;
}
