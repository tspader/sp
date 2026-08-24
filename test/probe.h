#ifndef TEST_PROBE_H
#define TEST_PROBE_H

#include "sp.h"
#include "sp/sp_test.h"

#if !defined(SP_WASM) && !defined(SP_FREESTANDING)

static s32 spawn_probe(sp_test_t* t, const c8* op, s64 handle) {
  sp_mem_t mem = sp_test_arena(t);
  sp_ps_output_t out = sp_ps_run(mem, (sp_ps_config_t) {
    .command = sp_fs_get_exe_path(mem),
    .args = {
      sp_str_lit("probe"),
      sp_cstr_as_str(op),
      sp_test_format(t, "{}", sp_fmt_int(handle)),
    },
    .io = {
      .in =  { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
  });
  return out.status.exit_code;
}

#endif

#endif
