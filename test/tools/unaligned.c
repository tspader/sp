#ifndef SP_IMPLEMENTATION
  #define SP_IMPLEMENTATION
#endif
#include "sp.h"

s32 main(s32 num_args, const c8** args) {
  // u8 buf [16] = {0};
  // u32* p = (u32*)(buf + 1);
  // volatile u32 v = *p;
  // sp_log("loaded {} from misaligned address {}", sp_fmt_uint(v), sp_fmt_ptr(p));

  sp_da(u64) arr = sp_zero();
  sp_da_push(arr, 69);
  sp_da_for(arr, it) {
    sp_log("arr[{.gray}] -> {}", sp_fmt_uint(it), sp_fmt_uint(arr[it]));
  }
  return 0;
}
