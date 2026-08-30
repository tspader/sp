#include "ops/ops.h"
#include "gen/gen.h"
#include "oracle/oracle.h"

static slot_t* get_first_dead(state_t* state) {
  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) return &state->slots[it];
  }
  sp_unreachable_return(SP_NULLPTR);
}

static err_t alloc_commit(state_t* state, u8* ptr, u64 size) {
  if (sp_align_up(ptr, SP_MEM_ALIGNMENT) != sp_uptr(ptr)) return ERR_ALIGN;
  try(oracle_overlap(state, SP_NULLPTR, ptr, size));

  slot_t* slot = get_first_dead(state);
  *slot = (slot_t) { .ptr = ptr, .size = size, .fill = gen_fill(state), .live = true };
  state->live++;
  sp_mem_fill_u8(ptr, size, slot->fill);
  return oracle_slot(state->heap, slot);
}

static err_t realloc_commit(state_t* state, slot_t* slot, u8* ptr, u64 size) {
  if (!size) {
    if (ptr) return ERR_ZERO;
    *slot = sp_zero_s(slot_t);
    state->live--;
    return ERR_OK;
  }

  if (!ptr) return ERR_NULL;
  if (sp_align_up(ptr, SP_MEM_ALIGNMENT) != sp_uptr(ptr)) return ERR_ALIGN;
  try(oracle_prefix(slot, ptr, size));
  try(oracle_overlap(state, slot, ptr, size));

  *slot = (slot_t) { .ptr = ptr, .size = size, .fill = gen_fill(state), .live = true };
  sp_mem_fill_u8(ptr, size, slot->fill);
  return oracle_slot(state->heap, slot);
}

static err_t op_alloc(state_t* state) {
  u64 size = gen_size(state);
  u8* ptr = (u8*)sp_mem_heap_alloc(state->heap, size);
  if (!ptr) return ERR_NULL;
  try(oracle_fresh(ptr, size));
  return alloc_commit(state, ptr, size);
}

static err_t op_alloc_uninit(state_t* state) {
  u64 size = gen_size(state);
  u8* ptr = (u8*)sp_mem_heap_alloc_uninitialized(state->heap, size);
  if (!ptr) return ERR_NULL;
  return alloc_commit(state, ptr, size);
}

static err_t op_free(state_t* state) {
  slot_t* slot = get_live_slot(state);
  try(oracle_bytes(slot));
  try(oracle_slot(state->heap, slot));
  sp_mem_heap_free(state->heap, slot->ptr);
  *slot = sp_zero_s(slot_t);
  state->live--;
  return ERR_OK;
}

static err_t op_realloc(state_t* state) {
  slot_t* slot = get_live_slot(state);
  u64 size = gen_size(state);
  try(oracle_bytes(slot));

  u8* ptr = (u8*)sp_mem_heap_realloc(state->heap, slot->ptr, size);
  if (ptr && size > slot->size) try(oracle_fresh(ptr + slot->size, size - slot->size));
  return realloc_commit(state, slot, ptr, size);
}

static err_t op_realloc_uninit(state_t* state) {
  slot_t* slot = get_live_slot(state);
  u64 size = gen_size(state);
  try(oracle_bytes(slot));

  u8* ptr = (u8*)sp_mem_heap_realloc_uninitialized(state->heap, slot->ptr, size);
  return realloc_commit(state, slot, ptr, size);
}

err_t step(state_t* state) {
  switch (sample_op(state)) {
    case OP_ALLOC: return op_alloc(state);
    case OP_ALLOC_UNINIT: return op_alloc_uninit(state);
    case OP_FREE: return op_free(state);
    case OP_REALLOC: return op_realloc(state);
    case OP_REALLOC_UNINIT: return op_realloc_uninit(state);
    case OP_COUNT: break;
  }
  sp_unreachable_return(ERR_OK);
}

err_t drain(state_t* state) {
  sp_carr_for(state->slots, it) {
    slot_t* slot = &state->slots[it];
    if (!slot->live) continue;
    try(oracle_bytes(slot));
    try(oracle_slot(state->heap, slot));
    sp_mem_heap_free(state->heap, slot->ptr);
    *slot = sp_zero_s(slot_t);
    state->live--;
  }
  return oracle_drained(state->heap);
}
