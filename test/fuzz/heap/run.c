#include "fuzz.h"

static err_t commit_alloc(sp_mem_heap_t* heap, model_t* model, const op_t* op, u8* ptr) {
  must(sp_align_up(ptr, SP_MEM_ALIGNMENT) == sp_uptr(ptr), ERR_ALIGN);
  try(oracle_overlap(model, SP_NULLPTR, ptr, op->size));

  slot_t* slot = &model->slots[op->slot];
  *slot = (slot_t) { .ptr = ptr, .size = op->size, .fill = op->fill, .live = true };
  sp_mem_fill_u8(ptr, op->size, op->fill);
  return oracle_slot(heap, slot);
}

static err_t commit_realloc(sp_mem_heap_t* heap, model_t* model, const op_t* op, u8* ptr) {
  slot_t* slot = &model->slots[op->slot];
  if (!op->size) {
    must(!ptr, ERR_ZERO);
    *slot = sp_zero_s(slot_t);
    return ERR_OK;
  }

  must(ptr, ERR_NULL);
  must(sp_align_up(ptr, SP_MEM_ALIGNMENT) == sp_uptr(ptr), ERR_ALIGN);
  try(oracle_prefix(slot, ptr, op->size));
  try(oracle_overlap(model, slot, ptr, op->size));

  *slot = (slot_t) { .ptr = ptr, .size = op->size, .fill = op->fill, .live = true };
  sp_mem_fill_u8(ptr, op->size, op->fill);
  return oracle_slot(heap, slot);
}

static err_t run_alloc(sp_mem_heap_t* heap, model_t* model, const op_t* op) {
  u8* ptr = (u8*)sp_mem_heap_alloc(heap, op->size);
  must(ptr, ERR_NULL);
  try(oracle_fresh(ptr, op->size));
  return commit_alloc(heap, model, op, ptr);
}

static err_t run_alloc_uninit(sp_mem_heap_t* heap, model_t* model, const op_t* op) {
  u8* ptr = (u8*)sp_mem_heap_alloc_uninitialized(heap, op->size);
  must(ptr, ERR_NULL);
  return commit_alloc(heap, model, op, ptr);
}

static err_t run_free(sp_mem_heap_t* heap, model_t* model, const op_t* op) {
  slot_t* slot = &model->slots[op->slot];
  try(oracle_bytes(slot));
  try(oracle_slot(heap, slot));

  sp_mem_heap_free(heap, slot->ptr);
  *slot = sp_zero_s(slot_t);
  return ERR_OK;
}

static err_t run_realloc(sp_mem_heap_t* heap, model_t* model, const op_t* op) {
  slot_t* slot = &model->slots[op->slot];
  try(oracle_bytes(slot));

  u8* ptr = (u8*)sp_mem_heap_realloc(heap, slot->ptr, op->size);
  if (ptr && op->size > slot->size) try(oracle_fresh(ptr + slot->size, op->size - slot->size));
  return commit_realloc(heap, model, op, ptr);
}

static err_t run_realloc_uninit(sp_mem_heap_t* heap, model_t* model, const op_t* op) {
  slot_t* slot = &model->slots[op->slot];
  try(oracle_bytes(slot));

  u8* ptr = (u8*)sp_mem_heap_realloc_uninitialized(heap, slot->ptr, op->size);
  return commit_realloc(heap, model, op, ptr);
}

static err_t run_op(sp_mem_heap_t* heap, model_t* model, const op_t* op) {
  switch (op->kind) {
    case OP_ALLOC: return run_alloc(heap, model, op);
    case OP_ALLOC_UNINIT: return run_alloc_uninit(heap, model, op);
    case OP_FREE: return run_free(heap, model, op);
    case OP_REALLOC: return run_realloc(heap, model, op);
    case OP_REALLOC_UNINIT: return run_realloc_uninit(heap, model, op);
    case OP_COUNT: break;
  }
  sp_unreachable_return(ERR_OK);
}

static err_t drain(sp_mem_heap_t* heap, model_t* model) {
  sp_carr_for(model->slots, it) {
    if (!model->slots[it].live) continue;
    op_t op = { .kind = OP_FREE, .slot = (u32)it };
    try(run_free(heap, model, &op));
  }
  return oracle_drained(heap);
}

err_t run_trace(const trace_t* trace, shape_t* shape) {
  sp_mem_heap_t* heap = sp_mem_heap_new();
  model_t model = sp_zero;

  err_t err = ERR_OK;
  for (u64 it = 0; it < trace->count && !err; it++) {
    err = run_op(heap, &model, &trace->ops[it]);
    if (!err) err = oracle_heap(heap, &model);
  }

  if (!err) *shape = shape_of(heap);
  if (!err) err = drain(heap, &model);
  if (!err) err = oracle_heap(heap, &model);

  sp_mem_heap_destroy(heap);
  return err;
}
