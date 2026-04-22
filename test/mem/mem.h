#ifndef MEM_TEST_H
#define MEM_TEST_H

#include "sp.h"
#include "test.h"
#include "utest.h"

#ifndef SP_MEM_ALIGNMENT
  #define SP_MEM_ALIGNMENT 16
#endif

#define EXPECT_ALIGNED(ptr) EXPECT_EQ(sp_align_up(ptr, SP_MEM_ALIGNMENT), ptr)

struct mem {
  u8 placeholder;
};

UTEST_F_SETUP(mem) {
  (void)utest_fixture;
  sp_tls_set(sp_rt.tls.key, SP_NULLPTR);
  sp_tls_rt_get();
}

UTEST_F_TEARDOWN(mem) {
  (void)utest_fixture;
  sp_tls_set(sp_rt.tls.key, SP_NULLPTR);
  sp_tls_rt_get();
}

u32 get_total_scratch_bytes_used() {
  u32 total = 0;

  sp_tls_rt_t* tls = sp_tls_rt_get();
  sp_carr_for(tls->scratch, it) {
    total += sp_mem_arena_bytes_used(tls->scratch[it]);
  }
  return total;
}

#endif
