#define SP_IMPLEMENTATION
#include "sp.h"

s32 main() {
  sp_str_t world = sp_str_lit("world");
  sp_str_t message = sp_fmt("hello, {}!", world);
  sp_log_str(message);

  SP_EXIT_SUCCESS();
}
