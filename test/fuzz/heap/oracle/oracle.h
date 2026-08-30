#ifndef FUZZ_HEAP_ORACLE_H
#define FUZZ_HEAP_ORACLE_H

#include "types.h"

err_t oracle_heap(state_t* state);
err_t oracle_slot(sp_mem_heap_t* heap, const slot_t* slot);
err_t oracle_bytes(const slot_t* slot);
err_t oracle_fresh(const u8* ptr, u64 len);
err_t oracle_prefix(const slot_t* slot, const u8* ptr, u64 size);
err_t oracle_overlap(const state_t* state, const slot_t* skip, const u8* ptr, u64 size);
err_t oracle_drained(sp_mem_heap_t* heap);

#endif
