#define SP_IMPLEMENTATION
#include "sp.h"

s32 main() {
  sp_str_t world = sp_str_lit("world");
  sp_str_t message = sp_fmt_a(sp_context_get_allocator(), "hello, {}!", world).value;
  sp_log_str_a(message);

  SP_EXIT_SUCCESS();
}
