#ifndef FUZZ_HEAP_TYPES_H
#define FUZZ_HEAP_TYPES_H

#include "sp.h"
#include "sp/sp_prng.h"

#define MAX_SLOTS 64

typedef enum {
  ERR_OK,
  ERR_NULL,
  ERR_ZERO,
  ERR_ALIGN,
  ERR_DIRTY,
  ERR_TAIL,
  ERR_BYTES,
  ERR_OVERLAP,
  ERR_IDENTITY,
  ERR_ACCOUNTING,
  ERR_RESERVED,
  ERR_LEAK,
  ERR_SPAN,
  ERR_LARGE,
  ERR_DRAIN,
  ERR_COUNT,
} err_t;

typedef enum {
  OP_ALLOC,
  OP_ALLOC_UNINIT,
  OP_FREE,
  OP_REALLOC,
  OP_REALLOC_UNINIT,
  OP_COUNT,
} op_t;

typedef enum {
  SIZE_TINY,
  SIZE_SMALL,
  SIZE_EDGE,
  SIZE_LARGE,
  SIZE_HUGE,
  SIZE_COUNT,
} size_class_t;

typedef struct {
  u64 ops [OP_COUNT];
  u64 sizes [SIZE_COUNT];
  bool big;
  u64 steps;
  u64 big_steps;
  u64 max_live;
  u64 big_max_live;
} profile_t;

typedef struct {
  u8* ptr;
  u64 size;
  u8 fill;
  bool live;
} slot_t;

typedef struct {
  sp_prng_t prng;
  profile_t profile;
  u32 max_live;
  u32 live;
  struct { u64 model; u64 seen; } peak;
  sp_mem_heap_t* heap;
  slot_t slots [MAX_SLOTS];
} state_t;

#define try(expr) do { err_t __err = (expr); if (__err) return __err; } while (0)

#endif
