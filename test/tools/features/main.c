#define SP_IMPLEMENTATION
#include "sp.h"

typedef struct {
  sp_atomic_s32_t* accumulator;
  s32 value;
  sp_thread_t thread;
} test_t;

s32 fn(void* user_data) {
  test_t* test = (test_t*)user_data;
  sp_atomic_s32_add(test->accumulator, test->value);
  return 0;
}

#define NUM_THREADS 4

int main(void) {
  sp_print("running {} threads...", sp_fmt_uint(NUM_THREADS));

  test_t threads [NUM_THREADS] = sp_zero();

  sp_atomic_s32_t accumulator = sp_zero();
  sp_carr_for(threads, it) {
    threads[it].accumulator = &accumulator;
    threads[it].value = it;
    sp_thread_init(&threads[it].thread, fn, &threads[it]);
  }

  sp_carr_for(threads, it) {
    sp_thread_join(&threads[it].thread);
  }

  sp_log("{.green}", sp_fmt_cstr("ok"));

  return 0;
}
