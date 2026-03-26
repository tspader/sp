#define SP_FREESTANDING
#define SP_BUILTIN
#define SP_IMPLEMENTATION
#include "sp.h"

sp_rt_t sp_rt;

void handle_interrupt(sp_os_signal_t signal) {
  sp_log(sp_str_lit("hello, {:fg brightcyan}"), SP_FMT_CSTR("signal"));
}

s32 main(s32 num_args, const c8** args) {
  sp_log(sp_str_lit("hello, {:fg brightcyan}"), SP_FMT_CSTR("world"));
  sp_os_register_signal_handler(SP_OS_SIGNAL_INTERRUPT, handle_interrupt);
  SP_LOG("sp_rt.dummy: {}", SP_FMT_U32(sp_rt.dummy));
  return sp_rt.dummy;
}

SP_ENTRY(main)

