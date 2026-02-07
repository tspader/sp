#define SP_IMPLEMENTATION
#include "sp.h"

typedef struct {
  s32 foo;
  sp_str_t bar;
  f32 baz [4];
  const c8* qux;
} struct_t;

s32 main(s32 num_args, const c8** args) {
  sp_ht(s32, s32) s32_to_s32 = SP_NULLPTR;
  sp_ht(s32, struct_t) s32_to_struct = SP_NULLPTR;
  sp_str_ht(s32) str_to_s32 = SP_NULLPTR;
  sp_cstr_ht(s32) cstr_to_s32 = SP_NULLPTR;

  sp_ht_insert(s32_to_s32, 1, 100);
  sp_ht_insert(s32_to_s32, 2, 200);
  sp_ht_insert(s32_to_s32, 3, 300);

  sp_str_t hello = sp_str_lit("hello");
  sp_str_t world = sp_str_lit("world");
  sp_str_ht_insert(str_to_s32, hello, 42);
  sp_str_ht_insert(str_to_s32, world, 99);

  sp_cstr_ht_insert(cstr_to_s32, "foo", 1);
  sp_cstr_ht_insert(cstr_to_s32, "bar", 2);

  struct_t s1 = {.foo = 42, .bar = sp_str_lit("test"), .baz = {1.0f, 2.0f, 3.0f, 4.0f}, .qux = "qux1"};
  struct_t s2 = {.foo = 99, .bar = sp_str_lit("hello"), .baz = {5.0f, 6.0f, 7.0f, 8.0f}, .qux = "qux2"};
  sp_ht_insert(s32_to_struct, 10, s1);
  sp_ht_insert(s32_to_struct, 20, s2);

  __asm__ volatile("int3");
  return 0;
}
