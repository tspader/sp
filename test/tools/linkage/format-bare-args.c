#define SP_IMPLEMENTATION
#include "sp.h"

s32 main() {
  sp_str_t world = sp_str_lit("world");
  sp_str_t message = sp_format("hello, {}!", world);
  sp_log(message);

  SP_EXIT_SUCCESS();
}
