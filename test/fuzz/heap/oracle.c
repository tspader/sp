#include "fuzz.h"

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

static u64 expected_bytes(const model_t* model) {
  u64 total = 0;
  sp_carr_for(model->slots, it) {
    if (!model->slots[it].live) continue;
    total += granted(model->slots[it].size);
  }
  return total;
}

static err_t check_span(sp_mem_heap_t* heap, sp_mem_heap_span_t* span, u32 bucket) {
  must(span->magic == SP_MEM_HEAP_SPAN_MAGIC, ERR_SPAN);
  must(span->bucket == bucket, ERR_SPAN);
  must(span->heap == heap, ERR_SPAN);
  must(span->in_use, ERR_SPAN);

  u64 bucket_size = sp_mem_heap_bucket_size(bucket);
  u8* base = (u8*)span + sizeof(sp_mem_heap_span_t);
  u8* end = (u8*)span + SP_MEM_HEAP_SPAN_SIZE;
  u32 num_chunks = (u32)((u64)(end - base) / bucket_size);
  must(span->in_use <= num_chunks, ERR_SPAN);

  u32 free_chunks = 0;
  for (void* chunk = span->free_head; chunk; chunk = *(void**)chunk) {
    must((u8*)chunk >= base && (u8*)chunk < end, ERR_SPAN);
    must(!((u64)((u8*)chunk - base) % bucket_size), ERR_SPAN);
    must(++free_chunks <= num_chunks, ERR_SPAN);
  }
  must(span->in_use + free_chunks == num_chunks, ERR_SPAN);
  return ERR_OK;
}

static err_t home_small(sp_mem_heap_t* heap, const slot_t* slot, u32 bucket) {
  sp_mem_heap_span_t* span = sp_mem_heap_find_span(heap, slot->ptr);
  must(span, ERR_IDENTITY);
  must(span->bucket == bucket, ERR_IDENTITY);
  return ERR_OK;
}

static err_t home_large(sp_mem_heap_t* heap, const slot_t* slot) {
  must(!sp_mem_heap_find_span(heap, slot->ptr), ERR_IDENTITY);

  sp_mem_heap_large_t* large = ((sp_mem_heap_large_t*)slot->ptr) - 1;
  must(large->magic == SP_MEM_HEAP_LARGE_MAGIC, ERR_IDENTITY);
  must(large->heap == heap, ERR_IDENTITY);
  must(large->size == slot->size, ERR_IDENTITY);
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
    must(bytes_are(slot->ptr + slot->size, sp_mem_heap_bucket_size(bucket) - slot->size, 0), ERR_TAIL);
    return ERR_OK;
  }

  try(home_large(heap, slot));
  sp_mem_heap_large_t* large = ((sp_mem_heap_large_t*)slot->ptr) - 1;
  u64 usable = large->capacity - sizeof(sp_mem_heap_large_t);
  must(bytes_are(slot->ptr + slot->size, usable - slot->size, 0), ERR_TAIL);
  return ERR_OK;
}

err_t oracle_bytes(const slot_t* slot) {
  must(bytes_are(slot->ptr, slot->size, slot->fill), ERR_BYTES);
  return ERR_OK;
}

err_t oracle_fresh(const u8* ptr, u64 len) {
  must(bytes_are(ptr, len, 0), ERR_DIRTY);
  return ERR_OK;
}

err_t oracle_prefix(const slot_t* slot, const u8* ptr, u64 size) {
  must(bytes_are(ptr, sp_min(slot->size, size), slot->fill), ERR_BYTES);
  return ERR_OK;
}

err_t oracle_overlap(const model_t* model, const slot_t* skip, const u8* ptr, u64 size) {
  u64 lo = sp_uptr(ptr);
  u64 hi = lo + granted(size);
  sp_carr_for(model->slots, it) {
    const slot_t* slot = &model->slots[it];
    if (!slot->live || slot == skip) continue;
    must(lo >= (u64)sp_uptr(slot->ptr) + granted(slot->size) || sp_uptr(slot->ptr) >= hi, ERR_OVERLAP);
  }
  return ERR_OK;
}

err_t oracle_heap(sp_mem_heap_t* heap, model_t* model) {
  u64 used = 0;
  u64 spans = 0;
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    sp_mem_heap_bucket_t* lists = &heap->buckets[bucket];
    must(!lists->partial || !lists->partial->prev, ERR_SPAN);
    must(!lists->full || !lists->full->prev, ERR_SPAN);

    for (sp_mem_heap_span_t* span = lists->partial; span; span = span->next) {
      must(span->free_head, ERR_SPAN);
      try(check_span(heap, span, bucket));
      must(!span->next || span->next->prev == span, ERR_SPAN);
      used += (u64)span->in_use * sp_mem_heap_bucket_size(bucket);
      spans++;
    }
    for (sp_mem_heap_span_t* span = lists->full; span; span = span->next) {
      must(!span->free_head, ERR_SPAN);
      try(check_span(heap, span, bucket));
      must(!span->next || span->next->prev == span, ERR_SPAN);
      used += (u64)span->in_use * sp_mem_heap_bucket_size(bucket);
      spans++;
    }
  }

  u64 recycled = 0;
  must(!heap->recycled || !heap->recycled->prev, ERR_SPAN);
  for (sp_mem_heap_span_t* span = heap->recycled; span; span = span->next) {
    must(span->magic != SP_MEM_HEAP_SPAN_MAGIC, ERR_SPAN);
    must(!span->next || span->next->prev == span, ERR_SPAN);
    recycled++;
  }

  u64 reserved = sp_align_offset(sizeof(sp_mem_heap_t), SP_MEM_HEAP_SPAN_SIZE);
  must(!heap->larges || !heap->larges->prev, ERR_LARGE);
  for (sp_mem_heap_large_t* large = heap->larges; large; large = large->next) {
    must(large->magic == SP_MEM_HEAP_LARGE_MAGIC, ERR_LARGE);
    must(large->heap == heap, ERR_LARGE);
    must(!large->next || large->next->prev == large, ERR_LARGE);
    must(large->size > SP_MEM_HEAP_MAX_SMALL, ERR_LARGE);
    must(!(large->capacity % SP_MEM_HEAP_SPAN_SIZE), ERR_LARGE);
    must(large->capacity - sizeof(sp_mem_heap_large_t) >= large->size, ERR_LARGE);
    used += large->size;
    reserved += large->capacity;
  }

  u64 segments = 0;
  for (sp_mem_heap_segment_t* segment = heap->segments; segment; segment = segment->next) {
    segments++;
  }
  reserved += segments * SP_MEM_HEAP_SEGMENT_SIZE;

  must(spans + recycled == segments * (SP_MEM_HEAP_SEGMENT_SIZE / SP_MEM_HEAP_SPAN_SIZE - 1), ERR_LEAK);
  must(used == heap->bytes_used, ERR_ACCOUNTING);
  must(used == expected_bytes(model), ERR_ACCOUNTING);
  must(reserved == heap->bytes_reserved, ERR_RESERVED);

  model->peak = sp_max(model->peak, reserved);
  must(heap->peak_reserved >= model->peak, ERR_PEAK);
  model->peak = heap->peak_reserved;

  sp_carr_for(model->slots, it) {
    if (!model->slots[it].live) continue;
    try(check_home(heap, &model->slots[it]));
  }
  return ERR_OK;
}

err_t oracle_drained(sp_mem_heap_t* heap) {
  must(!heap->bytes_used, ERR_DRAIN);
  must(!heap->larges, ERR_DRAIN);
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    must(!heap->buckets[bucket].partial, ERR_DRAIN);
    must(!heap->buckets[bucket].full, ERR_DRAIN);
  }
  return ERR_OK;
}

shape_t shape_of(sp_mem_heap_t* heap) {
  shape_t shape = {
    .reserved = heap->bytes_reserved,
    .peak = heap->peak_reserved,
  };
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    for (sp_mem_heap_span_t* span = heap->buckets[bucket].partial; span; span = span->next) shape.spans++;
    for (sp_mem_heap_span_t* span = heap->buckets[bucket].full; span; span = span->next) shape.spans++;
  }
  for (sp_mem_heap_span_t* span = heap->recycled; span; span = span->next) shape.recycled++;
  for (sp_mem_heap_large_t* large = heap->larges; large; large = large->next) shape.larges++;
  for (sp_mem_heap_segment_t* segment = heap->segments; segment; segment = segment->next) shape.segments++;
  return shape;
}
