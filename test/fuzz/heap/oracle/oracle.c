#include "oracle/oracle.h"

static u64 granted(u64 size) {
  u32 bucket = sp_mem_heap_bucket_of(size);
  return bucket < SP_MEM_HEAP_NUM_BUCKETS ? sp_mem_heap_bucket_size(bucket) : size;
}

static bool bytes_are(const u8* bytes, u64 len, u8 want) {
  sp_for(it, len) {
    if (bytes[it] != want) return false;
  }
  return true;
}

static u64 expected_bytes(const state_t* state) {
  u64 total = 0;
  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) continue;
    total += granted(state->slots[it].size);
  }
  return total;
}

static err_t check_span(sp_mem_heap_t* heap, sp_mem_heap_span_t* span, u32 bucket) {
  if (span->magic != SP_MEM_HEAP_SPAN_MAGIC) return ERR_SPAN;
  if (span->bucket != bucket) return ERR_SPAN;
  if (span->heap != heap) return ERR_SPAN;
  if (!span->in_use) return ERR_SPAN;

  u64 bucket_size = sp_mem_heap_bucket_size(bucket);
  u8* base = (u8*)span + sizeof(sp_mem_heap_span_t);
  u8* end = (u8*)span + SP_MEM_HEAP_SPAN_SIZE;
  u32 num_chunks = (u32)((u64)(end - base) / bucket_size);
  if (span->in_use > num_chunks) return ERR_SPAN;

  u32 free_chunks = 0;
  for (void* chunk = span->free_head; chunk; chunk = *(void**)chunk) {
    if ((u8*)chunk < base || (u8*)chunk >= end) return ERR_SPAN;
    if ((u64)((u8*)chunk - base) % bucket_size) return ERR_SPAN;
    if (++free_chunks > num_chunks) return ERR_SPAN;
  }
  if (span->in_use + free_chunks != num_chunks) return ERR_SPAN;
  return ERR_OK;
}

static err_t home_small(sp_mem_heap_t* heap, const slot_t* slot, u32 bucket) {
  sp_mem_heap_span_t* span = sp_mem_heap_find_span(heap, slot->ptr);
  if (!span) return ERR_IDENTITY;
  if (span->bucket != bucket) return ERR_IDENTITY;
  return ERR_OK;
}

static err_t home_large(sp_mem_heap_t* heap, const slot_t* slot) {
  if (sp_mem_heap_find_span(heap, slot->ptr)) return ERR_IDENTITY;

  sp_mem_heap_large_t* large = ((sp_mem_heap_large_t*)slot->ptr) - 1;
  if (large->magic != SP_MEM_HEAP_LARGE_MAGIC) return ERR_IDENTITY;
  if (large->heap != heap) return ERR_IDENTITY;
  if (large->size != slot->size) return ERR_IDENTITY;
  for (sp_mem_heap_large_t* it = heap->larges; it; it = it->next) {
    if (it == large) return ERR_OK;
  }
  return ERR_IDENTITY;
}

static err_t check_home(sp_mem_heap_t* heap, const slot_t* slot) {
  u32 bucket = sp_mem_heap_bucket_of(slot->size);
  if (bucket < SP_MEM_HEAP_NUM_BUCKETS) return home_small(heap, slot, bucket);
  return home_large(heap, slot);
}

err_t oracle_slot(sp_mem_heap_t* heap, const slot_t* slot) {
  u32 bucket = sp_mem_heap_bucket_of(slot->size);
  if (bucket < SP_MEM_HEAP_NUM_BUCKETS) {
    try(home_small(heap, slot, bucket));
    if (!bytes_are(slot->ptr + slot->size, sp_mem_heap_bucket_size(bucket) - slot->size, 0)) return ERR_TAIL;
    return ERR_OK;
  }

  try(home_large(heap, slot));
  sp_mem_heap_large_t* large = ((sp_mem_heap_large_t*)slot->ptr) - 1;
  u64 usable = large->capacity - sizeof(sp_mem_heap_large_t);
  if (!bytes_are(slot->ptr + slot->size, usable - slot->size, 0)) return ERR_TAIL;
  return ERR_OK;
}

err_t oracle_bytes(const slot_t* slot) {
  if (!bytes_are(slot->ptr, slot->size, slot->fill)) return ERR_BYTES;
  return ERR_OK;
}

err_t oracle_fresh(const u8* ptr, u64 len) {
  if (!bytes_are(ptr, len, 0)) return ERR_DIRTY;
  return ERR_OK;
}

err_t oracle_prefix(const slot_t* slot, const u8* ptr, u64 size) {
  if (!bytes_are(ptr, sp_min(slot->size, size), slot->fill)) return ERR_BYTES;
  return ERR_OK;
}

err_t oracle_overlap(const state_t* state, const slot_t* skip, const u8* ptr, u64 size) {
  u64 lo = sp_uptr(ptr);
  u64 hi = lo + granted(size);
  sp_carr_for(state->slots, it) {
    const slot_t* slot = &state->slots[it];
    if (!slot->live || slot == skip) continue;
    if (lo < (u64)sp_uptr(slot->ptr) + granted(slot->size) && sp_uptr(slot->ptr) < hi) return ERR_OVERLAP;
  }
  return ERR_OK;
}

err_t oracle_heap(state_t* state) {
  sp_mem_heap_t* heap = state->heap;

  u64 used = 0;
  u64 spans = 0;
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    sp_mem_heap_bucket_t* lists = &heap->buckets[bucket];
    if (lists->partial && lists->partial->prev) return ERR_SPAN;
    if (lists->full && lists->full->prev) return ERR_SPAN;

    for (sp_mem_heap_span_t* span = lists->partial; span; span = span->next) {
      if (!span->free_head) return ERR_SPAN;
      try(check_span(heap, span, bucket));
      if (span->next && span->next->prev != span) return ERR_SPAN;
      used += (u64)span->in_use * sp_mem_heap_bucket_size(bucket);
      spans++;
    }
    for (sp_mem_heap_span_t* span = lists->full; span; span = span->next) {
      if (span->free_head) return ERR_SPAN;
      try(check_span(heap, span, bucket));
      if (span->next && span->next->prev != span) return ERR_SPAN;
      used += (u64)span->in_use * sp_mem_heap_bucket_size(bucket);
      spans++;
    }
  }

  u64 recycled = 0;
  if (heap->recycled && heap->recycled->prev) return ERR_SPAN;
  for (sp_mem_heap_span_t* span = heap->recycled; span; span = span->next) {
    if (span->magic == SP_MEM_HEAP_SPAN_MAGIC) return ERR_SPAN;
    if (span->next && span->next->prev != span) return ERR_SPAN;
    recycled++;
  }

  u64 reserved = sp_align_offset(sizeof(sp_mem_heap_t), SP_MEM_HEAP_SPAN_SIZE);
  if (heap->larges && heap->larges->prev) return ERR_LARGE;
  for (sp_mem_heap_large_t* large = heap->larges; large; large = large->next) {
    if (large->magic != SP_MEM_HEAP_LARGE_MAGIC) return ERR_LARGE;
    if (large->heap != heap) return ERR_LARGE;
    if (large->next && large->next->prev != large) return ERR_LARGE;
    if (large->size <= SP_MEM_HEAP_MAX_SMALL) return ERR_LARGE;
    if (large->capacity % SP_MEM_HEAP_SPAN_SIZE) return ERR_LARGE;
    if (large->capacity - sizeof(sp_mem_heap_large_t) < large->size) return ERR_LARGE;
    used += large->size;
    reserved += large->capacity;
  }

  u64 segments = 0;
  for (sp_mem_heap_segment_t* segment = heap->segments; segment; segment = segment->next) {
    segments++;
  }
  reserved += segments * SP_MEM_HEAP_SEGMENT_SIZE;

  if (spans + recycled != segments * (SP_MEM_HEAP_SEGMENT_SIZE / SP_MEM_HEAP_SPAN_SIZE - 1)) return ERR_LEAK;
  if (used != heap->bytes_used) return ERR_ACCOUNTING;
  if (used != expected_bytes(state)) return ERR_ACCOUNTING;

  if (reserved != heap->bytes_reserved) return ERR_RESERVED;
  state->peak.model = sp_max(state->peak.model, reserved);
  if (heap->peak_reserved < state->peak.model) return ERR_RESERVED;
  if (heap->peak_reserved < state->peak.seen) return ERR_RESERVED;
  state->peak.seen = heap->peak_reserved;

  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) continue;
    try(check_home(heap, &state->slots[it]));
  }
  return ERR_OK;
}

err_t oracle_drained(sp_mem_heap_t* heap) {
  if (heap->bytes_used) return ERR_DRAIN;
  if (heap->larges) return ERR_DRAIN;
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    if (heap->buckets[bucket].partial) return ERR_DRAIN;
    if (heap->buckets[bucket].full) return ERR_DRAIN;
  }
  return ERR_OK;
}
