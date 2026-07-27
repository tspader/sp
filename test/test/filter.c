#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* filter;
  const c8* expect [8];
} filter_case_t;

static const c8* const filter_instances [] = {
  "A.S",
  "A.T/X",
  "A.T/Y",
  "AB.S",
  "B.S",
  "B.T/X",
};

static const filter_case_t filter_cases [] = {
  {
    .name = "none",
    .expect = { "A.S", "A.T/X", "A.T/Y", "AB.S", "B.S", "B.T/X" },
  },
  {
    .name = "instance",
    .filter = "A.S",
    .expect = { "A.S" },
  },
  {
    .name = "case",
    .filter = "A.T/X",
    .expect = { "A.T/X" },
  },
  {
    .name = "test",
    .filter = "A.T",
    .expect = { "A.T/X", "A.T/Y" },
  },
  {
    .name = "suite",
    .filter = "A",
    .expect = { "A.S", "A.T/X", "A.T/Y" },
  },
  {
    .name = "test_star",
    .filter = "A.*",
    .expect = { "A.S", "A.T/X", "A.T/Y" },
  },
  {
    .name = "case_star",
    .filter = "A.T/*",
    .expect = { "A.T/X", "A.T/Y" },
  },
  {
    .name = "suite_star",
    .filter = "*.S",
    .expect = { "A.S", "AB.S", "B.S" },
  },
  {
    .name = "case_across",
    .filter = "*/X",
    .expect = { "A.T/X", "B.T/X" },
  },
  {
    .name = "alternates",
    .filter = "{B,A.T}",
    .expect = { "A.T/X", "A.T/Y", "B.S", "B.T/X" },
  },
  {
    .name = "bare_case",
    .filter = "X",
  },
};

sp_test_each(runner, filter, filter_case_t, filter_cases) {
  sp_mem_t mem = sp_test_arena(t);

  sp_glob_t* glob = SP_NULLPTR;
  if (it->filter) {
    glob = sp_glob_new(mem, it->filter);
    sp_must(t, glob);
  }

  sp_da(sp_str_t) kept = sp_da_new(mem, sp_str_t);
  sp_carr_for(filter_instances, in) {
    if (sp_test_filtered(glob, filter_instances[in])) continue;
    sp_da_push(kept, sp_cstr_as_str(filter_instances[in]));
  }

  sp_must_strs_eq(t, kept, sp_da_size(kept), it->expect);
  return SP_OK;
}
