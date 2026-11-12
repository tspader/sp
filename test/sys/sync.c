#include "harness.h"

static const sys_case_t sys_sync_cases [] = {
  {
    .name = "written_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "A" } },
      { .kind = SYS_STEP_SYNC },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
};

sp_test_each_fn(sys, sync, sys_case_t, sys_sync_cases, sys_case_run);
