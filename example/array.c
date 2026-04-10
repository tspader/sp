#define SP_IMPLEMENTATION
#include "sp.h"

s32 main(s32 num_args, const c8** args) {
  sp_da(u32) years = sp_zero_initialize();
  sp_da_push(years, 1969);
  sp_da_push(years, 1972);
  sp_da_for(years, it) {
    sp_log("year: {}", SP_FMT_S32(years[it]));
  }

}
