#define SP_IMPLEMENTATION
#include "sp.h"

s32 run(s32 num_args, const c8** args) {
  sp_da(u32) years = sp_zero();
  sp_da_push(years, 1969);
  sp_da_push(years, 1972);
  sp_da_for(years, it) {
    sp_log("year: {}", sp_fmt_int(years[it]));
  }
  return 0;
}
SP_ENTRY(run)
