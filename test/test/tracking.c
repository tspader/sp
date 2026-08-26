#include "sp/sp_test.h"

#define TRACK_MAX_OPS   8
#define TRACK_MAX_SLOTS 26

typedef enum {
  TRACK_OP_NONE,
  TRACK_OP_ALLOC,
  TRACK_OP_FREE,
  TRACK_OP_REALLOC,
} track_op_kind_t;

typedef struct {
  c8 slot;
  u32 size;
} track_alloc_t;

typedef struct {
  c8 slot;
  u32 size;
  bool wild;
} track_free_t;

typedef struct {
  c8 from;
  c8 slot;
  u32 size;
  u32 old;
  bool wild;
} track_realloc_t;

typedef struct {
  track_op_kind_t kind;
  union {
    track_alloc_t alloc;
    track_free_t free;
    track_realloc_t realloc;
  };
} track_op_t;

typedef struct {
  u32 live;
  u32 bytes;
  u32 double_frees;
  u32 wild_frees;
  u32 bad_sizes;
} track_expect_t;

typedef struct {
  const c8* name;
  u32 repeat;
  track_op_t ops [TRACK_MAX_OPS];
  track_expect_t expect;
} track_case_t;

static const track_case_t track_cases [] = {
  {
    .name = "balance",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 64 } },
      { .kind = TRACK_OP_FREE, .free = { .slot = 'A' } },
    },
  },
  {
    .name = "alloc_zero",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A' } },
    },
  },
  {
    .name = "free_null",
    .ops = {
      { .kind = TRACK_OP_FREE },
    },
  },
  {
    .name = "accounting",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 16 } },
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'B', .size = 32 } },
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'C', .size = 64 } },
      { .kind = TRACK_OP_FREE, .free = { .slot = 'B' } },
    },
    .expect = { .live = 2, .bytes = 80 },
  },
  {
    .name = "double_free",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 64 } },
      { .kind = TRACK_OP_FREE, .free = { .slot = 'A' } },
      { .kind = TRACK_OP_FREE, .free = { .slot = 'A' } },
    },
    .expect = { .double_frees = 1 },
  },
  {
    .name = "wild_free",
    .ops = {
      { .kind = TRACK_OP_FREE, .free = { .wild = true } },
    },
    .expect = { .wild_frees = 1 },
  },
  {
    .name = "free_bad_size",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 32 } },
      { .kind = TRACK_OP_FREE, .free = { .slot = 'A', .size = 33 } },
    },
    .expect = { .bad_sizes = 1 },
  },
  {
    .name = "realloc_grow",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 8 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A', .slot = 'B', .size = 4096 } },
    },
    .expect = { .live = 1, .bytes = 4096 },
  },
  {
    .name = "realloc_shrink",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 64 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A', .slot = 'B', .size = 8 } },
    },
    .expect = { .live = 1, .bytes = 8 },
  },
  {
    .name = "realloc_same_size",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 64 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A', .slot = 'B', .size = 64 } },
    },
    .expect = { .live = 1, .bytes = 64 },
  },
  {
    .name = "realloc_null",
    .ops = {
      { .kind = TRACK_OP_REALLOC, .realloc = { .slot = 'A', .size = 32 } },
    },
    .expect = { .live = 1, .bytes = 32 },
  },
  {
    .name = "realloc_zero",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 32 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A' } },
    },
  },
  {
    .name = "realloc_stale",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 8 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A', .slot = 'B', .size = 4096 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A', .slot = 'C', .size = 16 } },
    },
    .expect = { .live = 1, .bytes = 4096, .double_frees = 1 },
  },
  {
    .name = "realloc_wild",
    .ops = {
      { .kind = TRACK_OP_REALLOC, .realloc = { .wild = true, .slot = 'A', .size = 16 } },
    },
    .expect = { .wild_frees = 1 },
  },
  {
    .name = "realloc_bad_size",
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 32 } },
      { .kind = TRACK_OP_REALLOC, .realloc = { .from = 'A', .slot = 'B', .size = 64, .old = 31 } },
    },
    .expect = { .live = 1, .bytes = 64, .bad_sizes = 1 },
  },
  {
    .name = "churn",
    .repeat = 4096,
    .ops = {
      { .kind = TRACK_OP_ALLOC, .alloc = { .slot = 'A', .size = 64 } },
      { .kind = TRACK_OP_FREE, .free = { .slot = 'A' } },
    },
  },
};

static u8 track_wild [16];

static u32 track_idx(c8 slot) {
  return (u32)(slot - 'A');
}

static void track_expect_bytes(sp_test_t* t, sp_mem_t mem, const void* bytes, u64 len, u8 fill) {
  u8* want = (u8*)sp_alloc(mem, len);
  sp_mem_fill_u8(want, len, fill);
  sp_expect_mem_eq(t, bytes, want, len);
}

sp_test_each(tracking, ops, track_case_t, track_cases) {
  sp_mem_t arena = sp_test_arena(t);

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_test_tracking_t k;
  sp_test_tracking_init(&k, arena, sp_mem_heap_as_allocator(heap));
  sp_mem_t mem = sp_test_tracking_as_allocator(&k);

  u32 repeat = it->repeat ? it->repeat : 1;
  sp_for(cycle, repeat) {
    void* ptr [TRACK_MAX_SLOTS] = sp_zero;
    u64 size [TRACK_MAX_SLOTS] = sp_zero;
    bool live [TRACK_MAX_SLOTS] = sp_zero;

    if (it->repeat) sp_test_kv(t, "cycle", sp_test_format(t, "{}", sp_fmt_uint(cycle)));

    sp_carr_for(it->ops, n) {
      const track_op_t* op = &it->ops[n];
      if (!op->kind) break;
      sp_test_kv(t, "op", sp_test_format(t, "{}", sp_fmt_uint(n)));

      switch (op->kind) {
        case TRACK_OP_NONE: break;
        case TRACK_OP_ALLOC: {
          void* p = sp_alloc(mem, op->alloc.size);
          if (!op->alloc.size) {
            sp_expect(t, !p);
            break;
          }
          sp_must(t, p);
          track_expect_bytes(t, arena, p, op->alloc.size, 0);
          sp_mem_fill_u8(p, op->alloc.size, (u8)op->alloc.slot);

          u32 slot = track_idx(op->alloc.slot);
          ptr[slot] = p;
          size[slot] = op->alloc.size;
          live[slot] = true;
          break;
        }
        case TRACK_OP_FREE: {
          void* p = SP_NULLPTR;
          u64 passed = op->free.size;
          if (op->free.wild) {
            p = track_wild;
          }
          else if (op->free.slot) {
            u32 slot = track_idx(op->free.slot);
            p = ptr[slot];
            if (!passed) passed = size[slot];
            if (live[slot]) track_expect_bytes(t, arena, p, size[slot], (u8)op->free.slot);
            live[slot] = false;
          }
          sp_free(mem, p, passed);
          break;
        }
        case TRACK_OP_REALLOC: {
          void* old_ptr = SP_NULLPTR;
          bool src_live = false;
          u64 old_actual = 0;
          if (op->realloc.wild) {
            old_ptr = track_wild;
          }
          else if (op->realloc.from) {
            u32 slot = track_idx(op->realloc.from);
            old_ptr = ptr[slot];
            src_live = live[slot];
            old_actual = size[slot];
            if (src_live) track_expect_bytes(t, arena, old_ptr, old_actual, (u8)op->realloc.from);
          }

          u64 passed = op->realloc.old ? op->realloc.old : old_actual;
          void* q = sp_realloc(mem, old_ptr, passed, op->realloc.size);

          if (!op->realloc.size) {
            sp_expect(t, !q);
            if (src_live) live[track_idx(op->realloc.from)] = false;
            break;
          }
          if (old_ptr && !src_live) {
            sp_expect(t, !q);
            break;
          }

          sp_must(t, q);
          if (src_live && old_actual == op->realloc.size) {
            sp_expect_eq(t, q, old_ptr);
          }

          u64 kept = sp_min(old_actual, (u64)op->realloc.size);
          track_expect_bytes(t, arena, q, kept, (u8)op->realloc.from);
          track_expect_bytes(t, arena, (u8*)q + kept, op->realloc.size - kept, 0);

          if (op->realloc.from) live[track_idx(op->realloc.from)] = false;
          sp_mem_fill_u8(q, op->realloc.size, (u8)op->realloc.slot);

          u32 slot = track_idx(op->realloc.slot);
          ptr[slot] = q;
          size[slot] = op->realloc.size;
          live[slot] = true;
          break;
        }
      }
    }

    sp_for(slot, TRACK_MAX_SLOTS) {
      if (!live[slot]) continue;
      track_expect_bytes(t, arena, ptr[slot], size[slot], (u8)('A' + slot));
    }
  }
  sp_test_kv_clear_all(t);

  sp_expect_eq(t, k.live_count, it->expect.live);
  sp_expect_eq(t, k.live_bytes, it->expect.bytes);
  sp_expect_eq(t, k.double_frees, it->expect.double_frees);
  sp_expect_eq(t, k.wild_frees, it->expect.wild_frees);
  sp_expect_eq(t, k.bad_sizes, it->expect.bad_sizes);

  sp_test_tracking_deinit(&k);
  sp_mem_heap_destroy(heap);
  return SP_OK;
}
