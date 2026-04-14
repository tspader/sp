#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

typedef struct {
  sp_atomic_s32_t* accumulator;
  s32 value;
} test_t;

s32 fn(void* user_data) {
  test_t* test = (test_t*)user_data;
  sp_atomic_s32_add(test->accumulator, test->value);
  return 0;
}

UTEST(thread, hello) {
  SKIP_ON_FREESTANDING();
  sp_thread_t threads [4] = sp_zero();
  test_t data [4] = sp_zero();

  sp_atomic_s32_t accumulator = sp_zero();
  sp_carr_for(threads, it) {
    data[it].accumulator = &accumulator;
    data[it].value = it;
    sp_thread_init(&threads[it], fn, &data[it]);
  }

  sp_carr_for(threads, it) {
    sp_thread_join(&threads[it]);
  }

  EXPECT_EQ(sp_atomic_s32_get(&accumulator), (0 + 1 + 2 + 3));
}
