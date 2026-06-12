#define SP_IMPLEMENTATION
#include "sp.h"

s32 run(s32 num_args, const c8** args) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);
  sp_da(u32) years = sp_da_new(mem, u32);
  sp_da_push(years, 1969);
  sp_da_push(years, 1972);
  sp_da_for(years, it) {
    sp_log("year: {}", sp_fmt_int(years[it]));
  }
  return 0;
}
SP_MAIN(run)
