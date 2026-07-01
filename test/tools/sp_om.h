#ifndef SP_OM_H
#define SP_OM_H

#include "sp.h"

#define sp_om_body(K, V)                                                       \
  {                                                                            \
    struct { sp_mem_arena_t* data; sp_mem_arena_t* metadata; } arenas;         \
    sp_da(V*) order;                                                           \
    sp_ht(K, V*) index;                                                        \
    V* temp;                                                                   \
  }

#define sp_om(K, V) struct sp_om_body(K, V)*

#define sp_om_new_ex(sm, darena, marena)                                       \
  do {                                                                         \
    (sm) = sp_void_cast(sm, sp_alloc_hint(sp_mem_os_new(), &(sm), sizeof(*(sm)))); \
    (sm)->arenas.data = darena;                                                \
    (sm)->arenas.metadata = marena;                                           \
    sp_da_init(sp_mem_arena_as_allocator((sm)->arenas.metadata), (sm)->order); \
    sp_ht_init(sp_mem_arena_as_allocator((sm)->arenas.metadata), (sm)->index); \
  } while (0)

#define sp_om_new(sm)                                                          \
  sp_om_new_ex(sm,                                                             \
    sp_mem_arena_new_ex(sp_mem_os_new(), 4096, 0), \
    sp_mem_arena_new_ex(sp_mem_os_new(), 4096, 0))

#define sp_om_ensure(sm)                                                       \
  if (!(sm)) {      \
    sp_om_new(sm);  \
  }

#define sp_om_set_fns(sm, hash_fn, cmp_fn)                                       \
  do {                                                                           \
    sp_om_ensure(sm);                                                            \
    sp_ht_set_fns((sm)->index, hash_fn, cmp_fn);                                 \
  } while (0)

#define sp_om_alloc_entry(sm)                                                  \
  sp_alloc_hint(sp_mem_arena_as_allocator((sm)->arenas.data),                  \
    &(sm)->temp, sizeof(*(sm)->temp))

#define sp_om_insert(sm, key, val)                                             \
  do {                                                                         \
    sp_om_ensure(sm);                                                          \
    if (sp_ht_getp((sm)->index, (key)) != SP_NULLPTR) {                        \
      break;                                                                   \
    }                                                                          \
    (sm)->temp = sp_void_cast((sm)->temp, sp_om_alloc_entry(sm));              \
    *(sm)->temp = (val);                                                       \
    sp_da_push((sm)->order, (sm)->temp);                                       \
    sp_ht_insert((sm)->index, (sm)->index->tmp_key, (sm)->temp);              \
  } while (0)

#define sp_om_free(sm)                                                         \
  do {                                                                         \
    if ((sm)) {                                                                \
      sp_mem_arena_destroy((sm)->arenas.data);                                 \
      sp_mem_arena_destroy((sm)->arenas.metadata);                             \
      sp_free(sp_mem_os_new(), sm, sizeof(*(sm)));                             \
      (sm) = SP_NULLPTR;                                                       \
    }                                                                          \
  } while (0)

#define sp_om_get(sm, key)        (!(sm) ? SP_NULLPTR : *sp_ht_getp((sm)->index, (key)))
#define sp_om_getp(sm, key)       (!(sm) ? SP_NULLPTR : sp_ht_getp((sm)->index, (key)))
#define sp_om_has(sm, key)        ((sm) && sp_ht_getp((sm)->index, (key)) != SP_NULLPTR)
#define sp_om_at(sm, n)           ((sm)->order[(n)])
#define sp_om_size(sm)            (!(sm) ? 0 : sp_da_size((sm)->order))
#define sp_om_empty(sm)           (!(sm) ? true : (sp_da_size((sm)->order) == 0))
#define sp_om_for(sm, it)         for (u32 it = 0; it < sp_om_size(sm); it++)
#define sp_om_back(sm)            (*sp_da_back((sm)->order))

#define sp_alloc_hint(mem, dummy, size) sp_alloc(mem, size)


////////////
// STR_OM //
////////////
#define sp_str_om(T) sp_om(sp_str_t, T)

#define sp_str_om_ensure(om) \
  if (!(om)) { \
    sp_str_om_init(om); \
  }

#define sp_str_om_init(om) \
  sp_om_new(om); \
  sp_om_set_fns(om, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key)

#define sp_str_om_insert(om, key, val)                                             \
  do { \
    sp_str_om_ensure(om); \
    sp_om_insert(om, (key), (val)); \
  } while (0)

#define sp_str_om_new(om)           sp_str_om_init(om);
#define sp_str_om_free(om)          sp_om_free(om)
#define sp_str_om_get(om, key)      sp_om_get(om, (key))
#define sp_str_om_getp(om, key)     sp_om_getp(om, (key))
#define sp_str_om_has(om, key)      sp_om_has(om, (key))
#define sp_str_om_at(om, n)         sp_om_at(om, n)
#define sp_str_om_size(om)          sp_om_size(om)
#define sp_str_om_empty(om)         sp_om_empty(om)
#define sp_str_om_for(om, it)       sp_om_for(om, it)
#define sp_str_om_back(om)          sp_om_back(om)



#endif
