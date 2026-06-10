#include "mem.h"

typedef enum {
  HEAP_OP_NONE = 0,
  HEAP_OP_ALLOC,
  HEAP_OP_FREE,
  HEAP_OP_REALLOC,
  HEAP_OP_WRITE,
} heap_op_kind_t;

typedef struct {
  heap_op_kind_t kind;
  u32 ref;
  u32 src;
  u64 size;
  u64 offset;
  u64 value;
} heap_op_t;

typedef struct {
  u32 a;
  u32 b;
} heap_pair_t;

typedef struct {
  u64 offset;
  u64 value;
} heap_data_t;

typedef struct {
  u32 ref;
  u64 bucket;
  u64 zeroed;
  bool large;
  heap_data_t data [4];
} heap_ref_check_t;

typedef struct {
  heap_pair_t same [8];
  heap_pair_t different [8];
  heap_pair_t same_ptr [8];
  heap_pair_t different_ptr [8];
  heap_ref_check_t refs [8];
  u64 num_spans;
  u64 num_large;
  u64 recycled;
  struct { u64 used; u64 reserved; } bytes;
} heap_expect_t;

#define HEAP_MAX_REFS 32

typedef struct {
  heap_op_t ops [32];
  heap_expect_t expect;
} heap_test_t;

static u64 count_heap_spans(sp_mem_heap_t* heap) {
  u64 n = 0;
  sp_for(b, SP_MEM_HEAP_NUM_BUCKETS) {
    for (sp_mem_heap_span_t* s = heap->buckets[b].partial; s; s = s->next) n++;
    for (sp_mem_heap_span_t* s = heap->buckets[b].full; s; s = s->next) n++;
  }
  return n;
}

static void run_heap_test(s32* utest_result, heap_test_t t) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  void* ptrs [HEAP_MAX_REFS] = sp_zero;

  sp_carr_for(t.ops, i) {
    heap_op_t* op = &t.ops[i];
    if (op->kind == HEAP_OP_NONE) break;
    switch (op->kind) {
      case HEAP_OP_NONE: break;
      case HEAP_OP_ALLOC:
        ptrs[op->ref] = sp_mem_heap_alloc(heap, op->size);
        EXPECT_NE(ptrs[op->ref], SP_NULLPTR);
        EXPECT_EQ((uintptr_t)ptrs[op->ref] & (SP_MEM_ALIGNMENT - 1), 0u);
        break;
      case HEAP_OP_FREE:
        sp_mem_heap_free(heap, ptrs[op->ref]);
        ptrs[op->ref] = SP_NULLPTR;
        break;
      case HEAP_OP_REALLOC:
        ptrs[op->ref] = sp_mem_heap_realloc(heap, ptrs[op->src], op->size);
        if (op->size) {
          EXPECT_NE(ptrs[op->ref], SP_NULLPTR);
          EXPECT_EQ((uintptr_t)ptrs[op->ref] & (SP_MEM_ALIGNMENT - 1), 0u);
        }
        else {
          EXPECT_EQ(ptrs[op->ref], SP_NULLPTR);
        }
        break;
      case HEAP_OP_WRITE:
        ((u8*)ptrs[op->ref])[op->offset] = (u8)op->value;
        break;
    }
  }

  heap_expect_t* e = &t.expect;

  sp_carr_for(e->same, i) {
    heap_pair_t p = e->same[i];
    if (!p.a && !p.b) break;
    sp_mem_heap_span_t* sa = sp_mem_heap_find_span(heap, ptrs[p.a]);
    sp_mem_heap_span_t* sb = sp_mem_heap_find_span(heap, ptrs[p.b]);
    EXPECT_NE(sa, SP_NULLPTR);
    EXPECT_EQ(sa, sb);
  }

  sp_carr_for(e->different, i) {
    heap_pair_t p = e->different[i];
    if (!p.a && !p.b) break;
    sp_mem_heap_span_t* sa = sp_mem_heap_find_span(heap, ptrs[p.a]);
    sp_mem_heap_span_t* sb = sp_mem_heap_find_span(heap, ptrs[p.b]);
    EXPECT_NE(sa, sb);
  }

  sp_carr_for(e->same_ptr, i) {
    heap_pair_t p = e->same_ptr[i];
    if (!p.a && !p.b) break;
    EXPECT_EQ(ptrs[p.a], ptrs[p.b]);
  }

  sp_carr_for(e->different_ptr, i) {
    heap_pair_t p = e->different_ptr[i];
    if (!p.a && !p.b) break;
    EXPECT_NE(ptrs[p.a], ptrs[p.b]);
  }

  sp_carr_for(e->refs, i) {
    heap_ref_check_t* r = &e->refs[i];
    if (!r->ref && !r->bucket && !r->zeroed && !r->large && !r->data[0].offset && !r->data[0].value) break;

    u8* p = (u8*)ptrs[r->ref];
    EXPECT_NE(p, SP_NULLPTR);
    if (!p) continue;

    if (r->bucket) {
      sp_mem_heap_span_t* s = sp_mem_heap_find_span(heap, p);
      EXPECT_NE(s, SP_NULLPTR);
      if (s) EXPECT_EQ((u64)s->bucket, (u64)sp_mem_heap_bucket_of(r->bucket));
    }
    if (r->zeroed) {
      sp_for(j, r->zeroed) EXPECT_EQ(p[j], 0u);
    }
    if (r->large) {
      EXPECT_EQ(sp_mem_heap_find_span(heap, p), SP_NULLPTR);
    }
    sp_carr_for(r->data, j) {
      heap_data_t* d = &r->data[j];
      if (!d->offset && !d->value) break;
      EXPECT_EQ(p[d->offset], (u8)d->value);
    }
  }

  u64 num_large = 0;
  for (sp_mem_heap_large_t* l = heap->larges; l; l = l->next) num_large++;

  u64 num_recycled = 0;
  for (sp_mem_heap_span_t* s = heap->recycled; s; s = s->next) num_recycled++;

  EXPECT_EQ(count_heap_spans(heap), e->num_spans);
  EXPECT_EQ(num_large, e->num_large);
  EXPECT_EQ(num_recycled, e->recycled);
  EXPECT_EQ(heap->bytes_used, e->bytes.used);
  EXPECT_EQ(heap->bytes_reserved, e->bytes.reserved);

  sp_mem_heap_destroy(heap);
}

UTEST_F(mem, heap_alloc_returns_aligned_nonnull) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 16 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 64 },
      { .kind = HEAP_OP_ALLOC, .ref = 2, .size = 1024 },
      { .kind = HEAP_OP_ALLOC, .ref = 3, .size = 8192 },
    },
    .expect = {
      .num_spans = 3,
      .num_large = 1,
      .recycled = 12,
      .bytes = { .used = 16 + 64 + 1344 + 8192, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE + 12288 },
    },
  });
}

UTEST_F(mem, heap_alloc_is_zeroed) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 128 },
    },
    .expect = {
      .refs = {
        { .ref = 0, .zeroed = 128 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 128, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_alloc_distinct_pointers) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 32 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 32 },
      { .kind = HEAP_OP_ALLOC, .ref = 2, .size = 32 },
    },
    .expect = {
      .different_ptr = {
        { 0, 1 }, { 1, 2 }, { 0, 2 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 3 * 32, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_bucket_of_sizes) {
  EXPECT_EQ(sp_mem_heap_bucket_of(0), 0u);
  EXPECT_EQ(sp_mem_heap_bucket_of(1), 0u);
  EXPECT_EQ(sp_mem_heap_bucket_of(16), 0u);
  EXPECT_EQ(sp_mem_heap_bucket_of(17), 1u);
  EXPECT_EQ(sp_mem_heap_bucket_of(32), 1u);
  EXPECT_EQ(sp_mem_heap_bucket_of(48), 2u);
  EXPECT_EQ(sp_mem_heap_bucket_of(64), 3u);
  EXPECT_EQ(sp_mem_heap_bucket_of(96), 4u);
  EXPECT_EQ(sp_mem_heap_bucket_of(128), 5u);
  EXPECT_EQ(sp_mem_heap_bucket_of(192), 6u);
  EXPECT_EQ(sp_mem_heap_bucket_of(256), 7u);
  EXPECT_EQ(sp_mem_heap_bucket_of(257), 8u);
  EXPECT_EQ(sp_mem_heap_bucket_of(336), 8u);
  EXPECT_EQ(sp_mem_heap_bucket_of(448), 9u);
  EXPECT_EQ(sp_mem_heap_bucket_of(576), 10u);
  EXPECT_EQ(sp_mem_heap_bucket_of(672), 11u);
  EXPECT_EQ(sp_mem_heap_bucket_of(800), 12u);
  EXPECT_EQ(sp_mem_heap_bucket_of(1008), 13u);
  EXPECT_EQ(sp_mem_heap_bucket_of(1009), 14u);
  EXPECT_EQ(sp_mem_heap_bucket_of(1344), 14u);
  EXPECT_EQ(sp_mem_heap_bucket_of(1345), 15u);
  EXPECT_EQ(sp_mem_heap_bucket_of(2016), 15u);
  EXPECT_EQ(sp_mem_heap_bucket_of(2017), (u32)SP_MEM_HEAP_NUM_BUCKETS);
  EXPECT_EQ(sp_mem_heap_bucket_of(4096), (u32)SP_MEM_HEAP_NUM_BUCKETS);
}

UTEST_F(mem, heap_small_allocs_share_a_span) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 64 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 64 },
      { .kind = HEAP_OP_ALLOC, .ref = 2, .size = 64 },
    },
    .expect = {
      .same = {
        { 0, 1 }, { 1, 2 },
      },
      .refs = {
        { .ref = 0, .bucket = 64 },
        { .ref = 1, .bucket = 64 },
        { .ref = 2, .bucket = 64 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 3 * 64, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_different_buckets_use_different_spans) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 32 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 128 },
    },
    .expect = {
      .different = {
        { 0, 1 },
      },
      .refs = {
        { .ref = 0, .bucket = 32 },
        { .ref = 1, .bucket = 128 },
      },
      .num_spans = 2,
      .recycled = 13,
      .bytes = { .used = 32 + 128, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_large_alloc_bypasses_spans) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 8192 },
    },
    .expect = {
      .refs = {
        { .ref = 0, .large = true },
      },
      .num_spans = 0,
      .num_large = 1,
      .bytes = { .used = 8192, .reserved = 4096 + 12288 },
    },
  });
}

UTEST_F(mem, heap_packed_bucket_fits_four_1008_chunks) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 2, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 3, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 4, .size = 1008 },
    },
    .expect = {
      .same = {
        { 0, 1 }, { 1, 2 }, { 2, 3 },
      },
      .different = {
        { 3, 4 },
      },
      .num_spans = 2,
      .recycled = 13,
      .bytes = { .used = 5 * 1008, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_full_span_overflows_to_new_span) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 800 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 800 },
      { .kind = HEAP_OP_ALLOC, .ref = 2, .size = 800 },
      { .kind = HEAP_OP_ALLOC, .ref = 3, .size = 800 },
      { .kind = HEAP_OP_ALLOC, .ref = 4, .size = 800 },
      { .kind = HEAP_OP_ALLOC, .ref = 5, .size = 800 },
    },
    .expect = {
      .same = {
        { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 },
      },
      .different = {
        { 4, 5 },
      },
      .num_spans = 2,
      .recycled = 13,
      .bytes = { .used = 6 * 800, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_free_from_full_span_reuses_slot) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 2, .size = 1008 },
      { .kind = HEAP_OP_ALLOC, .ref = 3, .size = 1008 },
      { .kind = HEAP_OP_FREE,  .ref = 1 },
      { .kind = HEAP_OP_ALLOC, .ref = 4, .size = 1008 },
    },
    .expect = {
      .same = {
        { 0, 4 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 4 * 1008, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_empty_span_is_recycled) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 64 },
      { .kind = HEAP_OP_FREE,  .ref = 0 },
    },
    .expect = {
      .num_spans = 0,
      .recycled = 15,
      .bytes = { .used = 0, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_recycled_span_is_reused) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 64 },
      { .kind = HEAP_OP_FREE,  .ref = 0 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 256 },
    },
    .expect = {
      .refs = {
        { .ref = 1, .bucket = 256 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 256, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_chunk_reuse_is_zeroed) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 32 },
      { .kind = HEAP_OP_WRITE, .ref = 0, .offset = 5, .value = 0xAB },
      { .kind = HEAP_OP_FREE,  .ref = 0 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 32 },
    },
    .expect = {
      .refs = {
        { .ref = 1, .zeroed = 32 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 32, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_alloc_zero_returns_smallest_chunk) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 0 },
    },
    .expect = {
      .refs = {
        { .ref = 0, .bucket = 16 },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 16, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_within_bucket_keeps_pointer) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC,   .ref = 0, .size = 20 },
      { .kind = HEAP_OP_WRITE,   .ref = 0, .offset = 10, .value = 7 },
      { .kind = HEAP_OP_REALLOC, .ref = 1, .src = 0, .size = 25 },
    },
    .expect = {
      .same_ptr = {
        { 0, 1 },
      },
      .refs = {
        { .ref = 1, .data = { { .offset = 10, .value = 7 } } },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 32, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_shrink_then_grow_reveals_zeroes) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC,   .ref = 0, .size = 32 },
      { .kind = HEAP_OP_WRITE,   .ref = 0, .offset = 10, .value = 7 },
      { .kind = HEAP_OP_WRITE,   .ref = 0, .offset = 30, .value = 9 },
      { .kind = HEAP_OP_REALLOC, .ref = 1, .src = 0, .size = 20 },
      { .kind = HEAP_OP_REALLOC, .ref = 2, .src = 1, .size = 31 },
    },
    .expect = {
      .same_ptr = {
        { 0, 2 },
      },
      .refs = {
        { .ref = 2, .data = { { .offset = 10, .value = 7 }, { .offset = 30, .value = 0 } } },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 32, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_grows_and_preserves_bytes) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC,   .ref = 0, .size = 16 },
      { .kind = HEAP_OP_WRITE,   .ref = 0, .offset = 3, .value = 7 },
      { .kind = HEAP_OP_REALLOC, .ref = 1, .src = 0, .size = 64 },
    },
    .expect = {
      .different_ptr = {
        { 0, 1 },
      },
      .refs = {
        { .ref = 1, .data = { { .offset = 3, .value = 7 } } },
      },
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 64, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_large_in_place) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC,   .ref = 0, .size = 5000 },
      { .kind = HEAP_OP_WRITE,   .ref = 0, .offset = 4999, .value = 9 },
      { .kind = HEAP_OP_REALLOC, .ref = 1, .src = 0, .size = 6000 },
    },
    .expect = {
      .same_ptr = {
        { 0, 1 },
      },
      .refs = {
        { .ref = 1, .large = true, .data = { { .offset = 4999, .value = 9 }, { .offset = 5999, .value = 0 } } },
      },
      .num_spans = 0,
      .num_large = 1,
      .bytes = { .used = 6000, .reserved = 4096 + 8192 },
    },
  });
}

UTEST_F(mem, heap_realloc_large_to_small_moves) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC,   .ref = 0, .size = 5000 },
      { .kind = HEAP_OP_WRITE,   .ref = 0, .offset = 10, .value = 3 },
      { .kind = HEAP_OP_REALLOC, .ref = 1, .src = 0, .size = 64 },
    },
    .expect = {
      .different_ptr = {
        { 0, 1 },
      },
      .refs = {
        { .ref = 1, .bucket = 64, .data = { { .offset = 10, .value = 3 } } },
      },
      .num_spans = 1,
      .num_large = 0,
      .recycled = 14,
      .bytes = { .used = 64, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_null_acts_as_alloc) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_REALLOC, .ref = 0, .src = 31, .size = 64 },
    },
    .expect = {
      .num_spans = 1,
      .recycled = 14,
      .bytes = { .used = 64, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_zero_frees) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC,   .ref = 0, .size = 64 },
      { .kind = HEAP_OP_REALLOC, .ref = 1, .src = 0, .size = 0 },
    },
    .expect = {
      .num_spans = 0,
      .num_large = 0,
      .recycled = 15,
      .bytes = { .used = 0, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_bytes_accounting) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 64 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 8192 },
    },
    .expect = {
      .num_spans = 1,
      .num_large = 1,
      .recycled = 14,
      .bytes = { .used = 64 + 8192, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE + 12288 },
    },
  });
}

UTEST_F(mem, heap_drained_heap_retains_segment) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0, .size = 64 },
      { .kind = HEAP_OP_ALLOC, .ref = 1, .size = 8192 },
      { .kind = HEAP_OP_FREE,  .ref = 0 },
      { .kind = HEAP_OP_FREE,  .ref = 1 },
    },
    .expect = {
      .num_spans = 0,
      .recycled = 15,
      .bytes = { .used = 0, .reserved = 4096 + SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_exhausted_segment_grows_new_segment) {
  run_heap_test(&ur, (heap_test_t){
    .ops = {
      { .kind = HEAP_OP_ALLOC, .ref = 0,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 1,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 2,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 3,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 4,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 5,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 6,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 7,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 8,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 9,  .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 10, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 11, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 12, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 13, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 14, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 15, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 16, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 17, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 18, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 19, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 20, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 21, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 22, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 23, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 24, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 25, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 26, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 27, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 28, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 29, .size = 2016 },
      { .kind = HEAP_OP_ALLOC, .ref = 30, .size = 2016 },
    },
    .expect = {
      .different_ptr = {
        { 0, 30 },
      },
      .num_spans = 16,
      .recycled = 14,
      .bytes = { .used = 31 * 2016, .reserved = 4096 + 2 * SP_MEM_HEAP_SEGMENT_SIZE },
    },
  });
}

UTEST_F(mem, heap_realloc_huge_size_fails_cleanly) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  u8* p = sp_ptr_cast(u8*, sp_mem_heap_alloc(heap, 5000));
  EXPECT_NE(p, SP_NULLPTR);
  if (p) {
    p[100] = 42;
    EXPECT_EQ(sp_mem_heap_realloc(heap, p, SP_LIMIT_U64_MAX - 16), SP_NULLPTR);
    EXPECT_EQ(p[100], 42u);
    EXPECT_EQ(heap->bytes_used, (u64)5000);
    sp_mem_heap_free(heap, p);
  }
  EXPECT_EQ(sp_mem_heap_alloc(heap, SP_LIMIT_U64_MAX - 16), SP_NULLPTR);
  sp_mem_heap_destroy(heap);
}

UTEST_F(mem, heap_null_heap_ops_are_noops) {
  u8 byte = 7;
  EXPECT_EQ(sp_mem_heap_alloc(SP_NULLPTR, 64), SP_NULLPTR);
  EXPECT_EQ(sp_mem_heap_realloc(SP_NULLPTR, &byte, 64), SP_NULLPTR);
  EXPECT_EQ(sp_mem_heap_find_span(SP_NULLPTR, &byte), SP_NULLPTR);
  sp_mem_heap_free(SP_NULLPTR, &byte);
  EXPECT_EQ(byte, 7u);
  sp_mem_heap_destroy(SP_NULLPTR);
}

UTEST_F(mem, heap_as_allocator_routes_through_sp_alloc) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);

  u8* p = sp_ptr_cast(u8*, sp_alloc(mem, 64));
  EXPECT_NE(p, SP_NULLPTR);
  EXPECT_EQ((uintptr_t)p & (SP_MEM_ALIGNMENT - 1), 0u);

  u8* q = sp_ptr_cast(u8*, sp_realloc(mem, p, 64, 128));
  EXPECT_NE(q, SP_NULLPTR);

  sp_free(mem, q, 128);
  sp_mem_heap_destroy(heap);
}
