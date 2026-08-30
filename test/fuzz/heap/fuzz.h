#ifndef FUZZ_HEAP_H
#define FUZZ_HEAP_H

#include "sp.h"
#include "sp/sp_prng.h"

#define MAX_SLOTS 64
#define MAX_STEPS 2048

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
  ERR_PEAK,
  ERR_LEAK,
  ERR_SPAN,
  ERR_LARGE,
  ERR_DRAIN,
  ERR_FLIP,
  ERR_COUNT,
} err_t;

typedef enum {
  OP_ALLOC,
  OP_ALLOC_UNINIT,
  OP_FREE,
  OP_REALLOC,
  OP_REALLOC_UNINIT,
  OP_COUNT,
} op_kind_t;

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
  op_kind_t kind;
  u32 slot;
  u64 size;
  u8 fill;
} op_t;

typedef struct {
  profile_t profile;
  u64 count;
  op_t ops [MAX_STEPS];
} trace_t;

typedef struct {
  u8* ptr;
  u64 size;
  u8 fill;
  bool live;
} slot_t;

typedef struct {
  slot_t slots [MAX_SLOTS];
  u64 peak;
} model_t;

typedef struct {
  u64 segments;
  u64 spans;
  u64 recycled;
  u64 larges;
  u64 reserved;
  u64 peak;
} shape_t;

#define try(expr) do { err_t __err = (expr); if (__err) return __err; } while (0)
#define must(expr, err) do { if (!(expr)) return err; } while (0)

sp_prng_profile_t* profile_new(sp_mem_t mem);
trace_t            gen_trace(sp_prng_t* prng, const sp_prng_profile_t* profile);
trace_t            flip_trace(trace_t trace);

err_t run_trace(const trace_t* trace, shape_t* shape);

err_t   oracle_bytes(const slot_t* slot);
err_t   oracle_fresh(const u8* ptr, u64 len);
err_t   oracle_prefix(const slot_t* slot, const u8* ptr, u64 size);
err_t   oracle_overlap(const model_t* model, const slot_t* skip, const u8* ptr, u64 size);
err_t   oracle_slot(sp_mem_heap_t* heap, const slot_t* slot);
err_t   oracle_heap(sp_mem_heap_t* heap, model_t* model);
err_t   oracle_drained(sp_mem_heap_t* heap);
shape_t shape_of(sp_mem_heap_t* heap);

#endif
