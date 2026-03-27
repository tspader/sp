#define SP_FREESTANDING
#define SP_DEFINE_BUILTINS
#define SP_IMPLEMENTATION
#include "sp.h"

sp_atomic_s32_t shutdown;

void handle_interrupt(sp_os_signal_t signal) {
  sp_log(sp_str_lit("received signal: {:fg brightcyan}"), SP_FMT_S32((s32)signal));
  sp_atomic_s32_set(&shutdown, 1);
}

s32 main(s32 num_args, const c8** args) {
  sp_log(sp_str_lit("hello, {:fg brightcyan}"), SP_FMT_CSTR("world"));
  sp_os_register_signal_handler(SP_OS_SIGNAL_INTERRUPT, handle_interrupt);
  sp_log(sp_str_lit("handler registered, send SIGINT to test"));

  /* spin so we can test ctrl+c */
  while (!sp_atomic_s32_get(&shutdown)) {
    sp_sys_timespec_t ts = { .tv_sec = 0, .tv_nsec = 100000000 };
    sp_sys_nanosleep(&ts, SP_NULLPTR);
  }

  sp_log(sp_str_lit("shutting down"));

  return 0;
}

SP_ENTRY(main)
