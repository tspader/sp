# problem
`sp_ring_buffer_t` requires fixed capacity at init time. For use cases like BFS directory traversal, the queue size is unknown. Need a growable ring queue with `sp_da`-style intrusive pointer semantics.

# solution
Implement `sp_rq` - an intrusive pointer ring queue that:
- Uses `T*` pointer type with hidden header (like `sp_da`)
- Grows automatically on push when full (doubles capacity)
- Provides FIFO semantics: push to tail, pop from head
- Pop returns nothing, peek returns pointer to head
- Default initial capacity of 8

## header
```c
typedef struct sp_ring_queue {
    s32 head;     // index of front element
    s32 size;     // current element count
    s32 capacity; // max elements before grow
} sp_ring_queue;

#define sp_rq(T) T*
```

## api

| macro/function | description |
|----------------|-------------|
| `sp_rq(T)` | type alias for `T*` |
| `sp_rq_head(arr)` | get header pointer |
| `sp_rq_size(arr)` | element count |
| `sp_rq_capacity(arr)` | max capacity |
| `sp_rq_empty(arr)` | true if size == 0 |
| `sp_rq_full(arr)` | true if size == capacity |
| `sp_rq_push(arr, val)` | push to tail, grow if full |
| `sp_rq_pop(arr)` | advance head, decrement size (noop if empty) |
| `sp_rq_peek(arr)` | pointer to head element (NULL if empty) |
| `sp_rq_back(arr)` | pointer to tail element (NULL if empty) |
| `sp_rq_at(arr, i)` | access element i (0 = head) |
| `sp_rq_clear(arr)` | reset size/head to 0 |
| `sp_rq_free(arr)` | free and null |
| `sp_rq_for(arr, it)` | iterate head to tail |
| `sp_rq_rfor(arr, it)` | iterate tail to head |

## grow/linearize
On resize (when push to full queue):
1. Allocate new buffer: `new_cap * elem_size + sizeof(sp_ring_queue)`
2. Copy head..end of old buffer to new buffer at index 0
3. Copy 0..wrap_point (if wrapped) after that
4. Set new head = 0, preserve size
5. Free old buffer

```
old: capacity=4, head=2, size=4
     [C][D][A][B]
          ^head

new: capacity=8, head=0, size=4
     [A][B][C][D][_][_][_][_]
      ^head
```

# plan

## phase 1: core structure and accessors
### tasks
- Add `sp_ring_queue` struct after `sp_dyn_array` in sp.h
- Add `sp_rq(T)` type macro
- Add `sp_rq_head`, `sp_rq_size`, `sp_rq_capacity`, `sp_rq_empty`, `sp_rq_full` accessor macros
- Add `sp_rq_clear`, `sp_rq_free` macros
- Add `SP_API void* sp_rq_grow_impl(void* arr, u32 elem_size, u32 new_cap)` function declaration
- Implement `sp_rq_grow_impl` in implementation section (linearizes on grow)

### references
- sp.h:940-943 - `sp_dyn_array` struct pattern
- sp.h:970-971 - `sp_dyn_array_head` macro pattern
- sp.h:3046-3066 - `sp_dyn_array_resize_impl` for realloc pattern
- sp.h:866 - `sp_mem_copy`
- sp.h:859 - `sp_alloc`
- sp.h:861 - `sp_free`

## phase 2: push/pop/peek/at
### tasks
- Add `sp_rq_push(arr, val)` macro - auto-init, grow if needed, push to tail
- Add `sp_rq_pop(arr)` macro - advance head, decrement size (noop if empty)
- Add `sp_rq_peek(arr)` macro - return pointer to head (NULL if empty)
- Add `sp_rq_back(arr)` macro - return pointer to tail (NULL if empty)
- Add `sp_rq_at(arr, i)` index macro (wraps around)

### references
- sp.h:1006-1014 - `sp_dyn_array_push` macro pattern
- sp.h:3168-3174 - `sp_ring_buffer_push` index calculation
- sp.h:3196-3202 - `sp_ring_buffer_pop` head advancement

## phase 3: iteration
### tasks
- Add `sp_rq_for(arr, it)` iteration macro (head to tail order)
- Add `sp_rq_rfor(arr, it)` iteration macro (tail to head order)

### references
- sp.h:967 - `sp_dyn_array_for` pattern
- sp.h:1293-1294 - `sp_ring_buffer_for` / `sp_ring_buffer_rfor` pattern

## unit tests
Tests in test/rb.c, run with `spn test --target rb`

| test | description |
|------|-------------|
| null_init | size/capacity/empty on NULL pointer |
| push_pop_fifo | push 3, peek/pop in FIFO order |
| full_and_grow | push to capacity, verify full, push triggers grow |
| wrap_access | push/pop to wrap head, verify at() and back() |
| iterate_wrapped | sp_rq_for after wrap, verify order |
| iterate_reverse | sp_rq_rfor, verify tail-to-head order |
| empty_ops | pop/peek/back on empty = noop/NULL |
| clear | push items, clear, verify empty |
| free | push items, free, verify NULL |
| struct_type | push/pop struct preserves all fields |
| capacity_one | single-slot queue boundary case |
