#include "sp.h"
#include "sp/sp_test.h"

#define MEM_MAX_STEPS 4
#define MEM_MAX_SLOTS 2
#define MEM_FILL 0xAA
#define MEM_FIXED_CAPACITY 512

typedef enum {
  MEM_STEP_NONE,
  MEM_STEP_ALLOC,
  MEM_STEP_ALLOC_UNINITIALIZED,
  MEM_STEP_REALLOC,
  MEM_STEP_REALLOC_UNINITIALIZED,
  MEM_STEP_FREE,
  MEM_STEP_CLEAR,
} mem_step_kind_t;

typedef enum {
  MEM_MEM_ARENA,
  MEM_MEM_FIXED,
  MEM_MEM_OS,
  MEM_MEM_HEAP,
} mem_mem_kind_t;

typedef struct {
  u64 from;
  u64 to;
} mem_range_t;

typedef struct {
  bool same;
  mem_range_t zero;
  mem_range_t fill;
} mem_expect_t;

typedef struct {
  mem_step_kind_t kind;
  u32 slot;
  u64 size;
  bool fill;
  mem_expect_t expect;
} mem_step_t;

typedef struct {
  const c8* name;
  mem_step_t steps [MEM_MAX_STEPS];
} mem_case_t;

typedef struct {
  u8* ptr;
  u64 size;
} mem_slot_t;

static const c8* mem_step_name(mem_step_kind_t kind) {
  switch (kind) {
    case MEM_STEP_NONE:        return "none";
    case MEM_STEP_ALLOC:       return "alloc";
    case MEM_STEP_ALLOC_UNINITIALIZED:   return "alloc_uninitialized";
    case MEM_STEP_REALLOC:     return "realloc";
    case MEM_STEP_REALLOC_UNINITIALIZED: return "realloc_uninitialized";
    case MEM_STEP_FREE:        return "free";
    case MEM_STEP_CLEAR:       return "clear";
  }
  SP_UNREACHABLE_RETURN("");
}

static void mem_check_range(sp_test_t* t, u8* ptr, mem_range_t range, u8 value) {
  for (u64 it = range.from; it < range.to; it++) {
    if (ptr[it] != value) {
      sp_test_fail(t, "byte {} is {} but expected {}", sp_fmt_uint(it), sp_fmt_uint(ptr[it]), sp_fmt_uint(value));
      return;
    }
  }
}

static sp_err_t mem_case_run(sp_test_t* t, mem_mem_kind_t kind, mem_case_t* c) {
  sp_mem_arena_t* arena = SP_NULLPTR;
  sp_mem_heap_t* heap = SP_NULLPTR;
  sp_mem_fixed_t fixed;
  u8 buffer [MEM_FIXED_CAPACITY];
  sp_mem_t mem;

  switch (kind) {
    case MEM_MEM_ARENA: {
      arena = sp_mem_arena_new(sp_mem_os_new());
      mem = sp_mem_arena_as_allocator(arena);
      break;
    }
    case MEM_MEM_FIXED: {
      fixed = sp_mem_fixed(buffer, MEM_FIXED_CAPACITY);
      mem = sp_mem_fixed_as_allocator(&fixed);
      break;
    }
    case MEM_MEM_OS: {
      mem = sp_mem_os_new();
      break;
    }
    case MEM_MEM_HEAP: {
      heap = sp_mem_heap_new();
      mem = sp_mem_heap_as_allocator(heap);
      break;
    }
  }

  mem_slot_t slots [MEM_MAX_SLOTS] = sp_zero;

  sp_carr_for(c->steps, it) {
    mem_step_t* step = &c->steps[it];
    if (step->kind == MEM_STEP_NONE) break;
    sp_test_kv(t, "step", sp_test_format(t, "[{}] {}", sp_fmt_uint(it), sp_fmt_cstr(mem_step_name(step->kind))));

    mem_slot_t* slot = &slots[step->slot];

    if (step->kind == MEM_STEP_FREE) {
      sp_free(mem, slot->ptr, slot->size);
      slot->ptr = SP_NULLPTR;
      slot->size = 0;
      continue;
    }

    if (step->kind == MEM_STEP_CLEAR) {
      switch (kind) {
        case MEM_MEM_ARENA: sp_mem_arena_clear(arena); break;
        case MEM_MEM_FIXED: sp_mem_fixed_clear(&fixed); break;
        case MEM_MEM_OS:    SP_UNREACHABLE_CASE();
        case MEM_MEM_HEAP:  SP_UNREACHABLE_CASE();
      }
      sp_carr_for(slots, s) {
        slots[s].ptr = SP_NULLPTR;
        slots[s].size = 0;
      }
      continue;
    }

    u8* old = slot->ptr;
    u8* ptr = SP_NULLPTR;
    switch (step->kind) {
      case MEM_STEP_ALLOC:       ptr = (u8*)sp_alloc(mem, step->size); break;
      case MEM_STEP_ALLOC_UNINITIALIZED:   ptr = (u8*)sp_alloc_uninitialized(mem, step->size); break;
      case MEM_STEP_REALLOC:     ptr = (u8*)sp_realloc(mem, slot->ptr, slot->size, step->size); break;
      case MEM_STEP_REALLOC_UNINITIALIZED: ptr = (u8*)sp_realloc_uninitialized(mem, slot->ptr, slot->size, step->size); break;
      case MEM_STEP_NONE:        SP_UNREACHABLE_CASE();
      case MEM_STEP_FREE:        SP_UNREACHABLE_CASE();
      case MEM_STEP_CLEAR:       SP_UNREACHABLE_CASE();
    }

    if (!ptr) {
      sp_test_fail(t, "allocation returned null");
      goto done;
    }
    if (sp_align_up(ptr, SP_MEM_ALIGNMENT) != sp_uptr(ptr)) {
      sp_test_fail(t, "pointer is not aligned");
    }
    slot->ptr = ptr;
    slot->size = step->size;

    if (step->fill) {
      sp_mem_fill_u8(ptr, step->size, MEM_FILL);
    }

    if (step->expect.same && ptr != old) {
      sp_test_fail(t, "expected pointer to be stable across resize");
    }
    mem_check_range(t, ptr, step->expect.zero, 0x00);
    mem_check_range(t, ptr, step->expect.fill, MEM_FILL);
  }

done:
  switch (kind) {
    case MEM_MEM_ARENA: {
      sp_mem_arena_destroy(arena);
      break;
    }
    case MEM_MEM_HEAP: {
      sp_mem_heap_destroy(heap);
      break;
    }
    case MEM_MEM_OS: {
      sp_carr_for(slots, it) {
        sp_free(mem, slots[it].ptr, slots[it].size);
      }
      break;
    }
    case MEM_MEM_FIXED: {
      break;
    }
  }
  return SP_OK;
}

static const mem_case_t mem_cases [] = {
  {
    .name = "alloc_uninitialized_is_writable",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 64, .fill = true, .expect = { .fill = { 0, 64 } } },
    },
  },
  {
    .name = "realloc_uninitialized_from_null_allocates",
    .steps = {
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 64, .fill = true, .expect = { .fill = { 0, 64 } } },
    },
  },
  {
    .name = "realloc_uninitialized_grow_preserves_contents",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 20, .fill = true },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 28, .expect = { .fill = { 0, 20 } } },
    },
  },
  {
    .name = "realloc_uninitialized_shrink_preserves_contents",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 64, .fill = true },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 32, .expect = { .fill = { 0, 32 } } },
    },
  },
  {
    .name = "alloc_zeroes_after_uninitialized_free",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 64, .fill = true },
      { .kind = MEM_STEP_FREE },
      { .kind = MEM_STEP_ALLOC, .size = 64, .expect = { .zero = { 0, 64 } } },
    },
  },
  {
    .name = "realloc_zeroes_tail_grown_from_uninitialized",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 20, .fill = true },
      { .kind = MEM_STEP_REALLOC, .size = 28, .expect = { .zero = { 20, 28 }, .fill = { 0, 20 } } },
    },
  },
  {
    .name = "realloc_zeroes_tail_across_relocation",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 20, .fill = true },
      { .kind = MEM_STEP_REALLOC, .size = 160, .expect = { .zero = { 20, 160 }, .fill = { 0, 20 } } },
    },
  },
  {
    .name = "uninitialized_shrink_keeps_realloc_zeroing",
    .steps = {
      { .kind = MEM_STEP_ALLOC, .size = 32, .fill = true },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 20 },
      { .kind = MEM_STEP_REALLOC, .size = 28, .expect = { .zero = { 20, 28 }, .fill = { 0, 20 } } },
    },
  },
};

static const mem_case_t mem_bump_cases [] = {
  {
    .name = "realloc_uninitialized_extends_top_in_place",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 16, .fill = true },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 32, .expect = { .same = true, .fill = { 0, 16 } } },
    },
  },
  {
    .name = "realloc_extends_top_in_place_from_uninitialized",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 16, .fill = true },
      { .kind = MEM_STEP_REALLOC, .size = 32, .expect = { .same = true, .zero = { 16, 32 }, .fill = { 0, 16 } } },
    },
  },
  {
    .name = "realloc_uninitialized_relocates_when_not_top",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 20, .fill = true },
      { .kind = MEM_STEP_ALLOC, .slot = 1, .size = 16 },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 40, .expect = { .fill = { 0, 20 } } },
    },
  },
  {
    .name = "alloc_zeroes_after_uninitialized_clear",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 64, .fill = true },
      { .kind = MEM_STEP_CLEAR },
      { .kind = MEM_STEP_ALLOC, .size = 64, .expect = { .zero = { 0, 64 } } },
    },
  },
};

static const mem_case_t mem_heap_cases [] = {
  {
    .name = "realloc_uninitialized_cross_bucket_preserves_contents",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 20, .fill = true },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 160, .expect = { .fill = { 0, 20 } } },
    },
  },
  {
    .name = "large_realloc_zeroes_tail_grown_from_uninitialized",
    .steps = {
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .size = 3000, .fill = true },
      { .kind = MEM_STEP_REALLOC, .size = 3500, .expect = { .zero = { 3000, 3500 }, .fill = { 0, 3000 } } },
    },
  },
  {
    .name = "large_uninitialized_shrink_keeps_realloc_zeroing",
    .steps = {
      { .kind = MEM_STEP_ALLOC, .size = 3000, .fill = true },
      { .kind = MEM_STEP_REALLOC_UNINITIALIZED, .size = 2500 },
      { .kind = MEM_STEP_REALLOC, .size = 2800, .expect = { .zero = { 2500, 2800 }, .fill = { 0, 2500 } } },
    },
  },
  {
    .name = "alloc_zeroes_reused_chunk",
    .steps = {
      { .kind = MEM_STEP_ALLOC, .size = 20 },
      { .kind = MEM_STEP_ALLOC_UNINITIALIZED, .slot = 1, .size = 20, .fill = true },
      { .kind = MEM_STEP_FREE, .slot = 1 },
      { .kind = MEM_STEP_ALLOC, .slot = 1, .size = 20, .expect = { .zero = { 0, 20 } } },
    },
  },
};

static sp_err_t mem_run_arena(sp_test_t* t, mem_case_t* c) {
  return mem_case_run(t, MEM_MEM_ARENA, c);
}

static sp_err_t mem_run_fixed(sp_test_t* t, mem_case_t* c) {
  return mem_case_run(t, MEM_MEM_FIXED, c);
}

static sp_err_t mem_run_os(sp_test_t* t, mem_case_t* c) {
  return mem_case_run(t, MEM_MEM_OS, c);
}

static sp_err_t mem_run_heap(sp_test_t* t, mem_case_t* c) {
  return mem_case_run(t, MEM_MEM_HEAP, c);
}

sp_test_each_fn(mem, uninitialized_arena, mem_case_t, mem_cases, mem_run_arena);
sp_test_each_fn(mem, uninitialized_fixed, mem_case_t, mem_cases, mem_run_fixed);
sp_test_each_fn(mem, uninitialized_os, mem_case_t, mem_cases, mem_run_os);
sp_test_each_fn(mem, uninitialized_heap, mem_case_t, mem_cases, mem_run_heap);
sp_test_each_fn(mem, uninitialized_bump_arena, mem_case_t, mem_bump_cases, mem_run_arena);
sp_test_each_fn(mem, uninitialized_bump_fixed, mem_case_t, mem_bump_cases, mem_run_fixed);
sp_test_each_fn(mem, uninitialized_heap_only, mem_case_t, mem_heap_cases, mem_run_heap);
