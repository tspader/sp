#ifndef SP_IMPLEMENTATION
  #define SP_IMPLEMENTATION
#endif
#include "sp.h"

s32 main(s32 num_args, const c8** args) {
  sp_log("hello, {:fg brightcyan}!", SP_FMT_CSTR("world"));
}
