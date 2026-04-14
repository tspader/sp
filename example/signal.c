#define SP_IMPLEMENTATION
#include "sp.h"

void handle_interrupt(sp_os_signal_t signal, void* userdata) {
  sp_log("received signal: {.fg brightcyan}", sp_fmt_int((s32)signal));
  sp_atomic_s32_set((sp_atomic_s32_t*)userdata, 1);
}

s32 run(s32 num_args, const c8** args) {
  sp_atomic_s32_t shutdown = SP_ZERO_INITIALIZE();

  sp_log("hello, {.fg brightcyan}", sp_fmt_cstr("world"));
  sp_os_register_signal_handler(SP_OS_SIGNAL_INTERRUPT, handle_interrupt, &shutdown);
  sp_log("handler registered, send SIGINT to test");

  /* spin so we can test ctrl+c */
  while (!sp_atomic_s32_get(&shutdown)) {
    sp_sleep_ns(100000000);
  }

  sp_log("shutting down");

  return 0;
}

SP_ENTRY(run)
