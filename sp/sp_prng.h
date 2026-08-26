#ifndef SP_PRNG_H
#define SP_PRNG_H

#include "sp.h"

typedef struct {
  u64 state;
} sp_prng_t;

SP_API sp_prng_t sp_prng_new(u64 seed);
SP_API u64       sp_prng_next(sp_prng_t* prng);
SP_API u64       sp_prng_below(sp_prng_t* prng, u64 bound);
SP_API u64       sp_prng_range(sp_prng_t* prng, u64 min, u64 max);
SP_API bool      sp_prng_chance(sp_prng_t* prng, u64 numerator, u64 denominator);
SP_API u32       sp_prng_weighted(sp_prng_t* prng, const u64* weights, u32 count);
SP_API void      sp_prng_shuffle(sp_prng_t* prng, void* base, u64 count, u64 stride);
SP_API void      sp_prng_swarm(sp_prng_t* prng, u64* weights, u32 count);
SP_API f32       sp_prng_f32(sp_prng_t* prng);
SP_API f32       sp_prng_f32_range(sp_prng_t* prng, f32 min, f32 max);
SP_API sp_prng_t sp_prng_stream(u64 seed, sp_str_t name);
SP_API sp_prng_t sp_prng_iter(sp_prng_t base, u64 iter);
SP_API bool      sp_prng_parse_seed(sp_str_t str, u64* seed);
SP_API u64       sp_prng_entropy();

#define SP_PRNG_MAX_ENTRIES 16

#define sp_prng_bind(T, FIELD) offsetof(T, FIELD)

typedef enum {
  SP_PRNG_KIND_NONE,
  SP_PRNG_KIND_RANGE,
  SP_PRNG_KIND_CHANCE,
  SP_PRNG_KIND_SWARM,
} sp_prng_kind_t;

typedef struct {
  sp_prng_kind_t kind;
  const c8* name;
  union {
    struct { u64 min; u64 max; u64 bind; } range;
    struct { u64 numerator; u64 denominator; u64 bind; } chance;
    struct { u32 count; u64 bind; } swarm;
  };
} sp_prng_entry_t;

typedef struct {
  sp_prng_entry_t entries [SP_PRNG_MAX_ENTRIES];
} sp_prng_profile_desc_t;

typedef struct {
  sp_prng_entry_t entries [SP_PRNG_MAX_ENTRIES];
  u32 count;
} sp_prng_profile_t;

SP_API sp_prng_profile_t* sp_prng_profile_new(sp_mem_t mem, sp_prng_profile_desc_t desc);
SP_API void               sp_prng_generate(sp_prng_t* prng, const sp_prng_profile_t* profile, void* data);

#endif

#if defined(SP_IMPLEMENTATION) && !defined(SP_PRNG_IMPLEMENTATION)
  #define SP_PRNG_IMPLEMENTATION
#endif

#if defined(SP_PRNG_IMPLEMENTATION) && !defined(SP_PRNG_C)
#define SP_PRNG_C

sp_prng_t sp_prng_new(u64 seed) {
  return (sp_prng_t) { .state = seed };
}

u64 sp_prng_next(sp_prng_t* prng) {
  prng->state += 0x9e3779b97f4a7c15u;
  u64 mixed = prng->state;
  mixed = (mixed ^ (mixed >> 30)) * 0xbf58476d1ce4e5b9u;
  mixed = (mixed ^ (mixed >> 27)) * 0x94d049bb133111ebu;
  return mixed ^ (mixed >> 31);
}

u64 sp_prng_below(sp_prng_t* prng, u64 bound) {
  return bound ? sp_prng_next(prng) % bound : 0;
}

u64 sp_prng_range(sp_prng_t* prng, u64 min, u64 max) {
  sp_assert(min <= max);
  return min + sp_prng_below(prng, max - min + 1);
}

bool sp_prng_chance(sp_prng_t* prng, u64 numerator, u64 denominator) {
  return sp_prng_below(prng, denominator) < numerator;
}

u32 sp_prng_weighted(sp_prng_t* prng, const u64* weights, u32 count) {
  u64 total = 0;
  sp_for(it, count) {
    total += weights[it];
  }
  if (!total) {
    return (u32)sp_prng_below(prng, count);
  }

  u64 pick = sp_prng_below(prng, total);
  sp_for(it, count) {
    if (pick < weights[it]) {
      return it;
    }
    pick -= weights[it];
  }
  sp_unreachable_return(0);
}

void sp_prng_shuffle(sp_prng_t* prng, void* base, u64 count, u64 stride) {
  u8* bytes = (u8*)base;
  for (u64 it = count; it > 1; it--) {
    u64 jt = sp_prng_below(prng, it);
    u8* a = bytes + (it - 1) * stride;
    u8* b = bytes + jt * stride;
    for (u64 kt = 0; kt < stride; kt++) {
      u8 held = a[kt];
      a[kt] = b[kt];
      b[kt] = held;
    }
  }
}

void sp_prng_swarm(sp_prng_t* prng, u64* weights, u32 count) {
  u32 enabled = (u32)sp_prng_range(prng, 1, count);
  u32 remaining = count;
  sp_for(it, count) {
    if (sp_prng_below(prng, remaining) < enabled) {
      weights[it] = sp_prng_range(prng, 1, 100);
      enabled--;
    }
    else {
      weights[it] = 0;
    }
    remaining--;
  }
}

f32 sp_prng_f32(sp_prng_t* prng) {
  return (f32)(sp_prng_next(prng) >> 40) * 0x1p-24f;
}

f32 sp_prng_f32_range(sp_prng_t* prng, f32 min, f32 max) {
  return min + sp_prng_f32(prng) * (max - min);
}

sp_prng_t sp_prng_stream(u64 seed, sp_str_t name) {
  return (sp_prng_t) { .state = sp_hash_bytes(name.data, name.len, seed) };
}

sp_prng_t sp_prng_iter(sp_prng_t base, u64 iter) {
  return (sp_prng_t) { .state = base.state ^ ((iter + 1) * 0x9e3779b97f4a7c15u) };
}

bool sp_prng_parse_seed(sp_str_t str, u64* seed) {
  if (sp_str_starts_with(str, sp_str_lit("0x")) || sp_str_starts_with(str, sp_str_lit("0X"))) {
    return sp_parse_hex_ex(str, seed);
  }
  return sp_parse_u64_ex(str, seed);
}

u64 sp_prng_entropy() {
  sp_tm_epoch_t now = sp_tm_now_epoch();
  sp_prng_t prng = (sp_prng_t) { .state = now.s ^ ((u64)now.ns << 32) };
  return sp_prng_next(&prng);
}

sp_prng_profile_t* sp_prng_profile_new(sp_mem_t mem, sp_prng_profile_desc_t desc) {
  sp_prng_profile_t* profile = sp_alloc_type(mem, sp_prng_profile_t);
  sp_carr_for_until(desc.entries, it, desc.entries[it].kind) {
    profile->entries[profile->count++] = desc.entries[it];
  }
  return profile;
}

void sp_prng_generate(sp_prng_t* prng, const sp_prng_profile_t* profile, void* data) {
  u8* bytes = (u8*)data;
  sp_for(it, profile->count) {
    const sp_prng_entry_t* entry = &profile->entries[it];
    switch (entry->kind) {
      case SP_PRNG_KIND_NONE: break;
      case SP_PRNG_KIND_RANGE: {
        *(u64*)(bytes + entry->range.bind) = sp_prng_range(prng, entry->range.min, entry->range.max);
        break;
      }
      case SP_PRNG_KIND_CHANCE: {
        *(bool*)(bytes + entry->chance.bind) = sp_prng_chance(prng, entry->chance.numerator, entry->chance.denominator);
        break;
      }
      case SP_PRNG_KIND_SWARM: {
        sp_prng_swarm(prng, (u64*)(bytes + entry->swarm.bind), entry->swarm.count);
        break;
      }
    }
  }
}

#endif
