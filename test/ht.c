#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct sp_ht {
  sp_mem_tracking_t tracking;
  sp_mem_t mem;
};

UTEST_F_SETUP(sp_ht) {
  sp_mem_tracking_init(&ut.tracking);
  ut.mem = sp_mem_tracking_as_allocator(&ut.tracking);
}

UTEST_F_TEARDOWN(sp_ht) {
  EXPECT_TRUE(sp_mem_tracking_ok(&ut.tracking));
  sp_mem_tracking_deinit(&ut.tracking);
}

struct siphash {
  sp_mem_tracking_t tracking;
  sp_mem_t mem;
};

UTEST_F_SETUP(siphash) {
  sp_mem_tracking_init(&ut.tracking);
  ut.mem = sp_mem_tracking_as_allocator(&ut.tracking);
}

UTEST_F_TEARDOWN(siphash) {
  EXPECT_TRUE(sp_mem_tracking_ok(&ut.tracking));
  sp_mem_tracking_deinit(&ut.tracking);
}

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  s32 id;
  s32 type;
} compound_key_t;

UTEST_F(sp_ht, basic_operations) {
  sp_ht(int, float) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  EXPECT_EQ(sp_ht_size(ht), 0u);
  EXPECT_TRUE(sp_ht_empty(ht));
  EXPECT_FALSE(sp_ht_getp(ht, 42));

  sp_ht_insert(ut.mem, ht, 42, 3.14f);
  EXPECT_EQ(sp_ht_size(ht), 1u);
  EXPECT_FALSE(sp_ht_empty(ht));
  EXPECT_TRUE(sp_ht_getp(ht, 42));
  EXPECT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ut.mem, ht, 10, 1.5f);
  sp_ht_insert(ut.mem, ht, 20, 2.5f);
  sp_ht_insert(ut.mem, ht, 30, 3.5f);
  EXPECT_EQ(sp_ht_size(ht), 4u);

  EXPECT_EQ(*sp_ht_getp(ht, 10), 1.5f);
  EXPECT_EQ(*sp_ht_getp(ht, 20), 2.5f);
  EXPECT_EQ(*sp_ht_getp(ht, 30), 3.5f);
  EXPECT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ut.mem, ht, 42, 6.28f);
  EXPECT_EQ(*sp_ht_getp(ht, 42), 6.28f);
  EXPECT_EQ(sp_ht_size(ht), 4u);

  sp_ht_erase(ht, 20);
  EXPECT_FALSE(sp_ht_getp(ht, 20));
  EXPECT_EQ(sp_ht_size(ht), 3u);

  sp_ht_clear(ht);
  EXPECT_EQ(sp_ht_size(ht), 0u);
  EXPECT_TRUE(sp_ht_empty(ht));

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, pointer_retrieval) {
  sp_ht(u32, double) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  sp_ht_insert(ut.mem, ht, 100, 123.456);
  sp_ht_insert(ut.mem, ht, 200, 789.012);

  double* ptr1 = sp_ht_getp(ht, 100);
  EXPECT_NE(ptr1, SP_NULLPTR);
  EXPECT_EQ(*ptr1, 123.456);

  *ptr1 = 999.999;
  EXPECT_EQ(*sp_ht_getp(ht, 100), 999.999);

  double* ptr2 = sp_ht_getp(ht, 999);
  EXPECT_EQ(ptr2, SP_NULLPTR);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, struct_values) {
  sp_ht(int, vec3_t) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  vec3_t v1 = {1.0f, 2.0f, 3.0f};
  vec3_t v2 = {4.0f, 5.0f, 6.0f};
  vec3_t v3 = {7.0f, 8.0f, 9.0f};

  sp_ht_insert(ut.mem, ht, 1, v1);
  sp_ht_insert(ut.mem, ht, 2, v2);
  sp_ht_insert(ut.mem, ht, 3, v3);

  vec3_t retrieved = *sp_ht_getp(ht, 2);
  EXPECT_EQ(retrieved.x, 4.0f);
  EXPECT_EQ(retrieved.y, 5.0f);
  EXPECT_EQ(retrieved.z, 6.0f);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, struct_keys) {
  sp_ht(compound_key_t, const char*) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  compound_key_t k1 = {100, 1};
  compound_key_t k2 = {200, 2};
  compound_key_t k3 = {300, 3};

  sp_ht_insert(ut.mem, ht, k1, "First");
  sp_ht_insert(ut.mem, ht, k2, "Second");
  sp_ht_insert(ut.mem, ht, k3, "Third");

  EXPECT_EQ(sp_ht_size(ht), 3u);

  compound_key_t lookup = {200, 2};
  EXPECT_TRUE(sp_ht_getp(ht, lookup));
  const char* value = *sp_ht_getp(ht, lookup);
  EXPECT_STREQ(value, "Second");

  compound_key_t missing = {200, 3};
  EXPECT_FALSE(sp_ht_getp(ht, missing));

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, string_keys) {
  sp_ht(u64, int) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  const char* s1 = "apple";
  const char* s2 = "banana";
  const char* s3 = "cherry";

  u64 k1 = sp_hash_cstr(s1);
  u64 k2 = sp_hash_cstr(s2);
  u64 k3 = sp_hash_cstr(s3);

  sp_ht_insert(ut.mem, ht, k1, 10);
  sp_ht_insert(ut.mem, ht, k2, 20);
  sp_ht_insert(ut.mem, ht, k3, 30);

  u64 lookup = sp_hash_cstr("banana");
  EXPECT_TRUE(sp_ht_getp(ht, lookup));
  EXPECT_EQ(*sp_ht_getp(ht, lookup), 20);

  lookup = sp_hash_cstr("dragonfruit");
  EXPECT_FALSE(sp_ht_getp(ht, lookup));

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, collision_handling) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  for (s32 i = 0; i < 100; i++) {
      sp_ht_insert(ut.mem, ht, i, i * 100);
  }

  EXPECT_EQ(sp_ht_size(ht), 100u);

  for (s32 i = 0; i < 100; i++) {
      EXPECT_TRUE(sp_ht_getp(ht, i));
      EXPECT_EQ(*sp_ht_getp(ht, i), i * 100);
  }

  for (s32 i = 0; i < 100; i += 3) {
      sp_ht_erase(ht, i);
  }

  for (s32 i = 0; i < 100; i++) {
      if (i % 3 == 0) {
          EXPECT_FALSE(sp_ht_getp(ht, i));
      } else {
          EXPECT_TRUE(sp_ht_getp(ht, i));
          EXPECT_EQ(*sp_ht_getp(ht, i), i * 100);
      }
  }

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, iteration) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  for (s32 i = 0; i < 10; i++) {
    sp_ht_insert(ut.mem, ht, i * 10, i);
  }

  s32 count = 0;
  s32 sum = 0;

  for (sp_ht_it_t it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    s32 val = *sp_ht_it_getp(ht, it);

    count++;
    sum += (s32)val;
  }

  EXPECT_EQ(count, 10);
  EXPECT_EQ(sum, 45);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, edge_cases) {
  sp_ht(int, int) ht1 = SP_NULLPTR;
  sp_ht_init(ut.mem, ht1);
  EXPECT_EQ(sp_ht_size(ht1), 0u);
  EXPECT_TRUE(sp_ht_empty(ht1));
  EXPECT_FALSE(sp_ht_getp(ht1, 42));

  sp_ht_clear(ht1);
  sp_ht_free(ut.mem, ht1);

  sp_ht(int, int) ht2 = SP_NULLPTR;
  sp_ht_init(ut.mem, ht2);
  sp_ht_insert(ut.mem, ht2, 1, 100);
  sp_ht_erase(ht2, 1);
  EXPECT_EQ(sp_ht_size(ht2), 0u);
  sp_ht_free(ut.mem, ht2);

  sp_ht(int, int) ht3 = SP_NULLPTR;
  sp_ht_init(ut.mem, ht3);
  sp_ht_insert(ut.mem, ht3, 1, 100);
  sp_ht_erase(ht3, 999);
  EXPECT_EQ(sp_ht_size(ht3), 1u);
  sp_ht_free(ut.mem, ht3);
}

UTEST_F(sp_ht, pathological_all_same_hash) {
  sp_ht(u32, u32) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  u64 cap = sp_ht_capacity(ht);
  if (cap < 2) {
    sp_ht_insert(ut.mem, ht, 0, 0);
    sp_ht_insert(ut.mem, ht, 1, 0);
    cap = sp_ht_capacity(ht);
  }

  for (u32 i = 0; i < cap; i++) {
    sp_ht_insert(ut.mem, ht, i, i * 100);
  }

  for (u32 i = 0; i < cap; i++) {
    EXPECT_TRUE(sp_ht_getp(ht, i));
    EXPECT_EQ(i * 100, *sp_ht_getp(ht, i));
  }

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, duplicate_key_insert_size_bug) {
  sp_ht(u32, u32) table = SP_NULLPTR;
  sp_ht_init(ut.mem, table);

  sp_ht_insert(ut.mem, table, 42, 100);
  EXPECT_EQ(sp_ht_size(table), 1u);
  EXPECT_EQ(*sp_ht_getp(table, 42), 100u);

  sp_ht_insert(ut.mem, table, 42, 200);
  EXPECT_EQ(sp_ht_size(table), 1u);
  EXPECT_EQ(*sp_ht_getp(table, 42), 200u);

  sp_ht_insert(ut.mem, table, 99, 300);
  EXPECT_EQ(sp_ht_size(table), 2u);
  EXPECT_EQ(*sp_ht_getp(table, 42), 200u);
  EXPECT_EQ(*sp_ht_getp(table, 99), 300u);

  sp_ht_free(ut.mem, table);
}

UTEST_F(sp_ht, iterator_yields_inactive_entry_at_slot_zero) {
  sp_ht(u64, u64) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  sp_ht_insert(ut.mem, ht, 0, 999);
  sp_ht_erase(ht, 0);

  EXPECT_EQ(sp_ht_size(ht), 0u);

  u64 num_entries = 0;
  for (sp_ht_it_t it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    num_entries++;
  }

  EXPECT_EQ(num_entries, 0u);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, collision) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  for (u32 i = 0; i < 8; i++) {
    sp_ht_insert(ut.mem, ht, i, i);
  }

  u64 capacity = sp_ht_capacity(ht);
  s32 keys [3] = { -1, -1, -1 };
  s32 num_found = 0;

  for (u32 candidate = 0; candidate < 1000; candidate++) {
    sp_hash_t hash = sp_hash_bytes(&candidate, sizeof(candidate), SP_HT_HASH_SEED);

    u64 bucket = hash % capacity;
    if (bucket == 0) {
      keys[num_found++] = candidate;
    }

    if (num_found == SP_CARR_LEN(keys)) {
      break;
    }
  }

  EXPECT_EQ(num_found, 3);

  sp_ht_clear(ht);

  sp_ht_insert(ut.mem, ht, keys[0], 0);
  sp_ht_insert(ut.mem, ht, keys[1], 1);
  sp_ht_insert(ut.mem, ht, keys[2], 2);

  EXPECT_TRUE(sp_ht_getp(ht, keys[0]));
  EXPECT_TRUE(sp_ht_getp(ht, keys[1]));
  EXPECT_TRUE(sp_ht_getp(ht, keys[2]));

  EXPECT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  EXPECT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  EXPECT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_erase(ht, keys[0]);

  EXPECT_FALSE(sp_ht_getp(ht, keys[0]));
  EXPECT_TRUE(sp_ht_getp(ht, keys[1]));
  EXPECT_TRUE(sp_ht_getp(ht, keys[2]));

  EXPECT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  EXPECT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_insert(ut.mem, ht, keys[0], 0);

  EXPECT_EQ(sp_ht_size(ht), 3u);
  EXPECT_TRUE(sp_ht_getp(ht, keys[0]));
  EXPECT_EQ(*sp_ht_getp(ht, keys[0]), 0);

  sp_ht_erase(ht, keys[2]);

  EXPECT_TRUE(sp_ht_getp(ht, keys[0]));
  EXPECT_TRUE(sp_ht_getp(ht, keys[1]));
  EXPECT_FALSE(sp_ht_getp(ht, keys[2]));

  EXPECT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  EXPECT_EQ(*sp_ht_getp(ht, keys[1]), 1);

  sp_ht_insert(ut.mem, ht, keys[2], 2);
  EXPECT_TRUE(sp_ht_getp(ht, keys[2]));
  EXPECT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, iterator_returns_zero_entries_for_populated_table) {
  sp_ht(u64, u64) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  sp_ht_insert(ut.mem, ht, 1, 100);
  sp_ht_insert(ut.mem, ht, 2, 200);

  u64 count = 0;
  for (sp_ht_it_t it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    count++;
  }

  EXPECT_EQ(count, 2u);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, null_safety) {
  sp_ht(s32, s32) null_ht = NULL;

  EXPECT_EQ(sp_ht_size(null_ht), 0u);
  EXPECT_EQ(sp_ht_capacity(null_ht), 0u);
  EXPECT_TRUE(sp_ht_empty(null_ht));
  EXPECT_FALSE(sp_ht_getp(null_ht, 42));

  s32* default_val = sp_ht_getp(null_ht, 42);
  EXPECT_EQ(default_val, SP_NULLPTR);

  sp_ht_it_t it = sp_ht_it_init(null_ht);
  EXPECT_EQ(it, 0u);
  EXPECT_FALSE(sp_ht_it_valid(null_ht, it));

  sp_ht_it_advance(null_ht, it);

  s32* iter_key = sp_ht_it_getkp(null_ht, it);
  s32* iter_val = sp_ht_it_getp(null_ht, it);
  EXPECT_EQ(iter_key, SP_NULLPTR);
  EXPECT_EQ(iter_val, SP_NULLPTR);

  sp_ht_clear(null_ht);
  sp_ht_erase(null_ht, 42);
  sp_ht_free(ut.mem, null_ht);

  sp_ht_init(ut.mem, null_ht);
  sp_ht_insert(ut.mem, null_ht, 42, 100);
  EXPECT_NE(null_ht, SP_NULLPTR);
  EXPECT_EQ(sp_ht_size(null_ht), 1u);
  EXPECT_EQ(*sp_ht_getp(null_ht, 42), 100);

  sp_ht_free(ut.mem, null_ht);
}

UTEST_F(sp_ht, string_key_custom_hash) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  sp_ht(sp_str_t, int) ht = SP_NULLPTR;
  sp_ht_init_ex(ut.mem, ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_str_t ka = sp_str_copy(s.mem, sp_str_lit("hello"));
  sp_str_t kb = sp_str_copy(s.mem, sp_str_lit("world"));
  sp_str_t kc = sp_str_copy(s.mem, sp_str_lit("test"));

  sp_ht_insert(ut.mem, ht, ka, 100);
  sp_ht_insert(ut.mem, ht, kb, 200);
  sp_ht_insert(ut.mem, ht, kc, 300);

  EXPECT_TRUE(sp_ht_getp(ht, ka));
  EXPECT_TRUE(sp_ht_getp(ht, kb));
  EXPECT_TRUE(sp_ht_getp(ht, kc));

  EXPECT_EQ(*sp_ht_getp(ht, ka), 100);
  EXPECT_EQ(*sp_ht_getp(ht, kb), 200);
  EXPECT_EQ(*sp_ht_getp(ht, kc), 300);

  sp_str_t ka_copy = sp_str_copy(s.mem, sp_str_lit("hello"));
  sp_str_t kb_copy = sp_str_copy(s.mem, sp_str_lit("world"));

  EXPECT_TRUE(sp_ht_getp(ht, ka_copy));
  EXPECT_TRUE(sp_ht_getp(ht, kb_copy));

  EXPECT_EQ(*sp_ht_getp(ht, ka_copy), 100);
  EXPECT_EQ(*sp_ht_getp(ht, kb_copy), 200);

  sp_str_t kd = sp_str_copy(s.mem, sp_str_lit("missing"));
  EXPECT_FALSE(sp_ht_getp(ht, kd));
  EXPECT_EQ(sp_ht_getp(ht, kd), SP_NULLPTR);

  sp_ht_free(ut.mem, ht);
  sp_mem_end_scratch(s);
}

UTEST_F(sp_ht, str_key) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();

  sp_ht(sp_str_t, s32) ht = SP_NULLPTR;
  sp_str_ht_init(ut.mem, ht);

  sp_str_t ka = sp_str_copy(s.mem, sp_str_lit("hello"));
  sp_str_t kb = sp_str_copy(s.mem, sp_str_lit("world"));
  sp_str_t kc = sp_str_copy(s.mem, sp_str_lit("test"));

  sp_str_ht_insert(ut.mem, ht, ka, 100);
  sp_str_ht_insert(ut.mem, ht, kb, 200);
  sp_str_ht_insert(ut.mem, ht, kc, 300);

  EXPECT_TRUE(sp_str_ht_get(ht, ka));
  EXPECT_TRUE(sp_str_ht_get(ht, kb));
  EXPECT_TRUE(sp_str_ht_get(ht, kc));

  EXPECT_EQ(*sp_str_ht_get(ht, ka), 100);
  EXPECT_EQ(*sp_str_ht_get(ht, kb), 200);
  EXPECT_EQ(*sp_str_ht_get(ht, kc), 300);

  sp_str_t ka_copy = sp_str_copy(s.mem, sp_str_lit("hello"));
  sp_str_t kb_copy = sp_str_copy(s.mem, sp_str_lit("world"));

  EXPECT_TRUE(sp_str_ht_get(ht, ka_copy));
  EXPECT_TRUE(sp_str_ht_get(ht, kb_copy));

  EXPECT_EQ(*sp_str_ht_get(ht, ka_copy), 100);
  EXPECT_EQ(*sp_str_ht_get(ht, kb_copy), 200);

  sp_str_t kd = sp_str_copy(s.mem, sp_str_lit("missing"));
  EXPECT_FALSE(sp_str_ht_get(ht, kd));
  EXPECT_EQ(sp_str_ht_get(ht, kd), SP_NULLPTR);

  sp_ht_free(ut.mem, ht);
  sp_mem_end_scratch(s);
}

UTEST_F(sp_ht, cstr_key) {
  sp_ht(const c8*, s32) ht = SP_NULLPTR;
  sp_cstr_ht_init(ut.mem, ht);

  sp_cstr_ht_insert(ut.mem, ht, "hello", 100);
  sp_cstr_ht_insert(ut.mem, ht, "world", 200);
  sp_cstr_ht_insert(ut.mem, ht, "test", 300);

  EXPECT_TRUE(sp_cstr_ht_get(ht, "hello"));
  EXPECT_TRUE(sp_cstr_ht_get(ht, "world"));
  EXPECT_TRUE(sp_cstr_ht_get(ht, "test"));

  EXPECT_EQ(*sp_cstr_ht_get(ht, "hello"), 100);
  EXPECT_EQ(*sp_cstr_ht_get(ht, "world"), 200);
  EXPECT_EQ(*sp_cstr_ht_get(ht, "test"), 300);

  EXPECT_FALSE(sp_cstr_ht_get(ht, "missing"));
  EXPECT_EQ(sp_cstr_ht_get(ht, "missing"), SP_NULLPTR);

  sp_cstr_ht_erase(ht, "world");
  EXPECT_FALSE(sp_cstr_ht_get(ht, "world"));
  EXPECT_EQ(sp_cstr_ht_size(ht), 2u);

  sp_cstr_ht_free(ut.mem, ht);
}

UTEST_F(siphash, consistency) {
  const char* data = "Hello, World!";
  u64 seed = 0x12345678;

  u64 hash1 = sp_hash_bytes((void*)data, sp_cstr_len(data), seed);
  u64 hash2 = sp_hash_bytes((void*)data, sp_cstr_len(data), seed);

  EXPECT_EQ(hash1, hash2);

  u64 hash3 = sp_hash_bytes((void*)data, sp_cstr_len(data), seed + 1);
  EXPECT_NE(hash1, hash3);
}

UTEST_F(siphash, different_lengths) {
  u64 seed = 0xABCDEF;

  u8 data1[1] = {0x42};
  u8 data2[7] = {1, 2, 3, 4, 5, 6, 7};
  u8 data3[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  u8 data4[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  u8 data5[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  u8 data6[17] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

  u64 h1 = sp_hash_bytes(data1, sizeof(data1), seed);
  u64 h2 = sp_hash_bytes(data2, sizeof(data2), seed);
  u64 h3 = sp_hash_bytes(data3, sizeof(data3), seed);
  u64 h4 = sp_hash_bytes(data4, sizeof(data4), seed);
  u64 h5 = sp_hash_bytes(data5, sizeof(data5), seed);
  u64 h6 = sp_hash_bytes(data6, sizeof(data6), seed);

  EXPECT_NE(h1, h2);
  EXPECT_NE(h2, h3);
  EXPECT_NE(h3, h4);
  EXPECT_NE(h4, h5);
  EXPECT_NE(h5, h6);
}

UTEST_F(siphash, collision_resistance) {
  u64 seed = 0x31415926;

  const s32 count = 1000;
  u64* hashes = sp_alloc_n(ut.mem, u64, count);

  for (s32 i = 0; i < count; i++) {
      hashes[i] = sp_hash_bytes(&i, sizeof(i), seed);
  }

  s32 collisions = 0;
  for (s32 i = 0; i < count; i++) {
      for (s32 j = i + 1; j < count; j++) {
          if (hashes[i] == hashes[j]) {
              collisions++;
          }
      }
  }

  EXPECT_EQ(collisions, 0);

  sp_free(ut.mem, hashes, count * sizeof(u64));
}

UTEST_F(sp_ht, hash_table_with_dyn_array_values) {
    typedef int* int_array;
    sp_ht(int, int_array) ht = SP_NULLPTR;
    sp_ht_init(ut.mem, ht);

    for (s32 i = 0; i < 5; i++) {
        sp_da(int) arr = SP_NULLPTR;

        for (s32 j = 0; j < 10; j++) {
            sp_da_push(ut.mem, arr, i * 100 + j);
        }

        sp_ht_insert(ut.mem, ht, i, arr);
    }

    for (s32 i = 0; i < 5; i++) {
        EXPECT_TRUE(sp_ht_getp(ht, i));

        int_array arr = *sp_ht_getp(ht, i);
        EXPECT_EQ(sp_da_size(arr), 10u);

        for (s32 j = 0; j < 10; j++) {
            EXPECT_EQ(arr[j], i * 100 + j);
        }
    }

    for (s32 i = 0; i < 5; i++) {
        int_array arr = *sp_ht_getp(ht, i);
        sp_da_free(ut.mem, arr);
    }

    sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, multiple_arrays_in_hash_table) {
  sp_ht(int, void*) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  for (s32 key = 0; key < 5; key++) {
    sp_da(int) arr = SP_NULLPTR;

    for (s32 j = 0; j < 20; j++) {
        sp_da_push(ut.mem, arr, key * 1000 + j);
    }

    sp_ht_insert(ut.mem, ht, key, (void*)arr);
  }

  for (s32 key = 0; key < 5; key++) {
    EXPECT_TRUE(sp_ht_getp(ht, key));

    int* arr = (int*)*sp_ht_getp(ht, key);
    EXPECT_EQ(sp_da_size(arr), 20u);

    for (s32 j = 0; j < 20; j++) {
        EXPECT_EQ(arr[j], key * 1000 + j);
    }
  }

  for (s32 key = 0; key < 5; key++) {
    int* arr = (int*)*sp_ht_getp(ht, key);
    sp_da_free(ut.mem, arr);
  }

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, front_null_table) {
  sp_ht(int, int) ht = NULL;
  EXPECT_EQ(sp_ht_front(ht), SP_NULLPTR);
}

UTEST_F(sp_ht, front_empty_table) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);
  EXPECT_EQ(sp_ht_front(ht), SP_NULLPTR);
  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, front_single_item) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);
  sp_ht_insert(ut.mem, ht, 42, 100);

  int* front = sp_ht_front(ht);
  EXPECT_NE(front, SP_NULLPTR);
  EXPECT_EQ(*front, 100);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, front_multiple_items) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);
  sp_ht_insert(ut.mem, ht, 1, 10);
  sp_ht_insert(ut.mem, ht, 2, 20);
  sp_ht_insert(ut.mem, ht, 3, 30);

  int* front = sp_ht_front(ht);
  EXPECT_NE(front, SP_NULLPTR);

  bool found = false;
  sp_ht_for(ht, it) {
    if (sp_ht_it_getp(ht, it) == front) {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, front_after_erase) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);
  sp_ht_insert(ut.mem, ht, 1, 10);
  sp_ht_insert(ut.mem, ht, 2, 20);

  int* first_front = sp_ht_front(ht);
  EXPECT_NE(first_front, SP_NULLPTR);

  int first_val = *first_front;

  if (first_val == 10) {
    sp_ht_erase(ht, 1);
    int* new_front = sp_ht_front(ht);
    EXPECT_NE(new_front, SP_NULLPTR);
    EXPECT_EQ(*new_front, 20);
  } else {
    sp_ht_erase(ht, 2);
    int* new_front = sp_ht_front(ht);
    EXPECT_NE(new_front, SP_NULLPTR);
    EXPECT_EQ(*new_front, 10);
  }

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, for_kv_iteration) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);
  sp_ht(s32, bool) visited = SP_NULLPTR;
  sp_ht_init(ut.mem, visited);

  sp_for(it, 10) {
    sp_ht_insert(ut.mem, ht, it * 10, it);
  }

  s32 count = 0;
  s32 key_sum = 0;
  s32 val_sum = 0;

  sp_ht_for_kv(ht, it) {
    sp_ht_insert(ut.mem, visited, *it.key, true);
    key_sum += *it.key;
    val_sum += *it.val;
    count++;
  }

  EXPECT_EQ(count, 10);
  EXPECT_EQ(key_sum, 450);
  EXPECT_EQ(val_sum, 45);

  sp_for(it, 10) {
    EXPECT_NE(sp_ht_getp(visited, it * 10), SP_NULLPTR);
  }

  sp_ht_free(ut.mem, ht);
  sp_ht_free(ut.mem, visited);
}

UTEST_F(sp_ht, for_kv_string_keys) {
  sp_ht(sp_str_t, s32) ht = SP_NULLPTR;
  sp_ht_init_ex(ut.mem, ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_ht(sp_str_t, bool) visited = SP_NULLPTR;
  sp_ht_init_ex(ut.mem, visited, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_ht_insert(ut.mem, ht, sp_str_lit("one"), 1);
  sp_ht_insert(ut.mem, ht, sp_str_lit("two"), 2);
  sp_ht_insert(ut.mem, ht, sp_str_lit("three"), 3);

  s32 count = 0;
  s32 val_sum = 0;

  sp_ht_for_kv(ht, it) {
    EXPECT_FALSE(sp_str_empty(*it.key));
    sp_ht_insert(ut.mem, visited, *it.key, true);
    val_sum += *it.val;
    count++;
  }

  EXPECT_EQ(count, 3);
  EXPECT_EQ(val_sum, 6);
  EXPECT_NE(sp_ht_getp(visited, sp_str_lit("one")), SP_NULLPTR);
  EXPECT_NE(sp_ht_getp(visited, sp_str_lit("two")), SP_NULLPTR);
  EXPECT_NE(sp_ht_getp(visited, sp_str_lit("three")), SP_NULLPTR);

  sp_ht_free(ut.mem, ht);
  sp_ht_free(ut.mem, visited);
}

UTEST_F(sp_ht, for_kv_empty_table) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  s32 count = 0;
  sp_ht_for_kv(ht, it) {
    (void)it;
    count++;
  }

  EXPECT_EQ(count, 0);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, for_kv_null_table) {
  sp_ht(s32, s32) ht = SP_NULLPTR;

  s32 count = 0;
  sp_ht_for_kv(ht, it) {
    (void)it;
    count++;
  }

  EXPECT_EQ(count, 0);
}

// Issue tests: reproduce bugs documented in doc/hash-table.md

static sp_hash_t hash_constant(void* key, u64 size) {
  (void)key;
  (void)size;
  return 42;
}

static bool cmp_s32(void* ka, void* kb, u64 size) {
  (void)size;
  return *(s32*)ka == *(s32*)kb;
}

// Hash collision: insert uses hash equality instead of key equality for probing.
// Constant hash forces all keys to same bucket; different keys overwrite each other.
UTEST_F(sp_ht, issue_hash_collision_overwrites_different_keys) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init_ex(ut.mem, ht, hash_constant, cmp_s32);

  sp_ht_insert(ut.mem, ht, 100, 1);
  sp_ht_insert(ut.mem, ht, 200, 2);
  sp_ht_insert(ut.mem, ht, 300, 3);

  EXPECT_EQ(sp_ht_size(ht), 3u);
  EXPECT_TRUE(sp_ht_getp(ht, 100));
  EXPECT_TRUE(sp_ht_getp(ht, 200));
  EXPECT_TRUE(sp_ht_getp(ht, 300));

  s32* val_a = sp_ht_getp(ht, 100);
  s32* val_b = sp_ht_getp(ht, 200);
  s32* val_c = sp_ht_getp(ht, 300);

  EXPECT_NE(val_a, SP_NULLPTR);
  EXPECT_NE(val_b, SP_NULLPTR);
  EXPECT_NE(val_c, SP_NULLPTR);

  if (val_a && val_b && val_c) {
    EXPECT_EQ(*val_a, 1);
    EXPECT_EQ(*val_b, 2);
    EXPECT_EQ(*val_c, 3);
  }

  sp_ht_free(ut.mem, ht);
}

static sp_hash_t sp_test_identity_hash(void* key, u64 size) {
  (void)size;
  return (sp_hash_t)(*(s32*)key);
}

// Rehash: resize extends array but doesn't rehash existing entries.
// Insert a key whose slot changes after resize, verify it moves correctly.
UTEST_F(sp_ht, issue_rehash_breaks_lookups_after_resize) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init_ex(ut.mem, ht, sp_test_identity_hash, cmp_s32);

  sp_ht_insert(ut.mem, ht, 1, 0);
  u64 initial_capacity = sp_ht_capacity(ht);

  s32 ka = (s32)initial_capacity;
  sp_ht_insert(ut.mem, ht, ka, 2000);

  u64 slot_before_resize = (u64)ka % initial_capacity;
  EXPECT_EQ(ht->data[slot_before_resize].key, ka);

  for (s32 i = 10000; sp_ht_capacity(ht) == initial_capacity; i++) {
    sp_ht_insert(ut.mem, ht, i, 0);
  }
  EXPECT_GT(sp_ht_capacity(ht), initial_capacity);

  u64 slot_after_resize = (u64)ka % sp_ht_capacity(ht);
  EXPECT_NE(slot_before_resize, slot_after_resize);

  EXPECT_NE(ht->data[slot_before_resize].key, ka);
  EXPECT_EQ(ht->data[slot_after_resize].key, ka);
  EXPECT_EQ(ht->data[slot_after_resize].state, SP_HT_ENTRY_ACTIVE);

  sp_ht_free(ut.mem, ht);
}

static u32 g_hash_call_count = 0;

static sp_hash_t sp_test_counting_hash(void* key, u64 size) {
  g_hash_call_count++;
  return sp_hash_bytes(key, size, SP_HT_HASH_SEED);
}

static bool sp_test_counting_compare(void* ka, void* kb, u64 size) {
  return sp_mem_is_equal(ka, kb, size);
}

// Linear scan: lookup probes entire capacity instead of stopping at INACTIVE.
// With 1 element in cap=64 table, missing key lookup should be O(1), not O(N).
UTEST_F(sp_ht, issue_linear_scan_calls_hash_too_many_times) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init_ex(ut.mem, ht, sp_test_counting_hash, sp_test_counting_compare);

  for (s32 i = 0; i < 32; i++) {
    sp_ht_insert(ut.mem, ht, i, i);
  }
  u64 capacity = sp_ht_capacity(ht);
  sp_ht_clear(ht);

  g_hash_call_count = 0;
  sp_ht_insert(ut.mem, ht, 999, 42);

  g_hash_call_count = 0;
  s32* val = sp_ht_getp(ht, 999);
  u32 found_calls = g_hash_call_count;

  EXPECT_NE(val, SP_NULLPTR);
  EXPECT_EQ(*val, 42);

  g_hash_call_count = 0;
  s32* missing = sp_ht_getp(ht, 12345);
  u32 missing_calls = g_hash_call_count;

  EXPECT_EQ(missing, SP_NULLPTR);

  u32 max_acceptable = 10;
  EXPECT_LT(found_calls, max_acceptable);
  EXPECT_LT(missing_calls, max_acceptable);

  (void)capacity;
  sp_ht_free(ut.mem, ht);
}

static s32 g_key_eval_count = 0;
static s32 g_val_eval_count = 0;

static s32 eval_key(void) {
  g_key_eval_count++;
  return 42;
}

static s32 eval_val(void) {
  g_val_eval_count++;
  return 100;
}

UTEST_F(sp_ht, issue_insert_evaluates_args_multiple_times) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init(ut.mem, ht);

  g_key_eval_count = 0;
  g_val_eval_count = 0;

  sp_ht_insert(ut.mem, ht, eval_key(), eval_val());

  EXPECT_EQ(g_key_eval_count, 1);
  EXPECT_EQ(g_val_eval_count, 1);

  sp_ht_free(ut.mem, ht);
}

// The key and value are stored before the table has anywhere to put them, and either may read
// the table
UTEST_F(sp_ht, insert_allocates_a_null_table) {
  typedef struct { u64 a; f32 b; c8 name [48]; } wide_t;

  sp_ht(u32, u64) ids = SP_NULLPTR;
  sp_ht(u64, wide_t) wides = SP_NULLPTR;
  wide_t wide = { .a = 0xDEADBEEFCAFEBABEull, .b = 2.5f, .name = "wide" };

  sp_for(it, 500) {
    ASSERT_EQ(sp_ht_insert(ut.mem, ids, (u32)(sp_ht_size(ids) * 3), sp_ht_size(ids) + 10), SP_OK);
  }
  EXPECT_EQ(sp_ht_size(ids), 500u);
  sp_for(it, 500) {
    u64* value = sp_ht_getp(ids, (u32)(it * 3));
    ASSERT_TRUE(value != SP_NULLPTR);
    EXPECT_EQ(*value, it + 10);
  }

  ASSERT_EQ(sp_ht_insert(ut.mem, wides, 0x0123456789ABCDEFull, wide), SP_OK);
  wide_t* found = sp_ht_getp(wides, 0x0123456789ABCDEFull);
  ASSERT_TRUE(found != SP_NULLPTR);
  EXPECT_EQ(found->a, wide.a);
  EXPECT_EQ(found->b, wide.b);
  EXPECT_STREQ(found->name, "wide");

  sp_ht_free(ut.mem, ids);
  sp_ht_free(ut.mem, wides);
  EXPECT_TRUE(ids == SP_NULLPTR);
}

// The string tables pick up their hash and compare when the first insert allocates them
UTEST_F(sp_ht, string_tables_need_no_init) {
  sp_str_ht(u32) strs = SP_NULLPTR;
  sp_cstr_ht(u32) cstrs = SP_NULLPTR;
  c8 a [] = "walrus";
  c8 b [] = "walrus";

  ASSERT_EQ(sp_str_ht_insert(ut.mem, strs, sp_cstr_as_str(a), 1), SP_OK);
  ASSERT_EQ(sp_cstr_ht_insert(ut.mem, cstrs, a, 2), SP_OK);

  ASSERT_TRUE(sp_str_ht_get(strs, sp_cstr_as_str(b)) != SP_NULLPTR);
  EXPECT_EQ(*sp_str_ht_get(strs, sp_cstr_as_str(b)), 1u);
  ASSERT_TRUE(sp_cstr_ht_get(cstrs, b) != SP_NULLPTR);
  EXPECT_EQ(*sp_cstr_ht_get(cstrs, b), 2u);

  sp_str_ht_free(ut.mem, strs);
  sp_cstr_ht_free(ut.mem, cstrs);
}

// A table takes two allocations to create. Whichever fails, the caller hears that error, the
// table stays null, and nothing leaks.
UTEST_F(sp_ht, insert_returns_the_allocators_error) {
  sp_for(at, 2) {
    sp_mem_failing_t failing = { .backing = ut.mem, .err = at ? SP_ERR_IO_EOF : SP_ERR_SYS_BUSY, .at = at };
    sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
    sp_ht(u32, u32) ht = SP_NULLPTR;

    EXPECT_EQ(sp_ht_insert(mem, ht, 1, 2), failing.err);
    EXPECT_TRUE(ht == SP_NULLPTR);
    EXPECT_EQ(ut.tracking.live_count, 0u);

    failing.at = failing.count + at;
    EXPECT_EQ(sp_ht_init(mem, ht), failing.err);
    EXPECT_TRUE(ht == SP_NULLPTR);
    EXPECT_EQ(ut.tracking.live_count, 0u);

    ASSERT_EQ(sp_ht_insert(mem, ht, 1, 2), SP_OK);
    EXPECT_EQ(*sp_ht_getp(ht, 1), 2u);
    sp_ht_free(mem, ht);
  }
}

static s32 ht_eval_seeing(void* ht, void** seen) {
  *seen = ht;
  return 42;
}

UTEST_F(sp_ht, failed_first_insert_does_not_evaluate_the_key_or_value) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 0 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_ht(s32, s32) ht = SP_NULLPTR;

  g_key_eval_count = 0;
  g_val_eval_count = 0;

  EXPECT_EQ(sp_ht_insert(mem, ht, eval_key(), eval_val()), SP_ERR_SYS_BUSY);
  EXPECT_EQ(g_key_eval_count, 0);
  EXPECT_EQ(g_val_eval_count, 0);
  EXPECT_TRUE(ht == SP_NULLPTR);

  ASSERT_EQ(sp_ht_insert(mem, ht, eval_key(), eval_val()), SP_OK);
  EXPECT_EQ(g_key_eval_count, 1);
  EXPECT_EQ(g_val_eval_count, 1);
  EXPECT_EQ(*sp_ht_getp(ht, 42), 100);

  sp_ht_free(mem, ht);
}

UTEST_F(sp_ht, first_insert_evaluates_against_the_real_table) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  void* key_saw = SP_NULLPTR;
  void* val_saw = SP_NULLPTR;

  ASSERT_EQ(sp_ht_insert(ut.mem, ht, ht_eval_seeing(ht, &key_saw), ht_eval_seeing(ht, &val_saw)), SP_OK);
  EXPECT_TRUE(key_saw != SP_NULLPTR);
  EXPECT_TRUE(key_saw == (void*)ht);
  EXPECT_TRUE(val_saw == (void*)ht);
  EXPECT_EQ(sp_ht_size(ht), 1u);
  EXPECT_EQ(*sp_ht_getp(ht, 42), 42);

  sp_ht_free(ut.mem, ht);
}

UTEST_F(sp_ht, failed_growth_leaves_the_table_alone) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 5 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_ht(u32, u32) ht = SP_NULLPTR;

  sp_err_t err = SP_OK;
  u32 inserted = 0;
  while (!err) {
    err = sp_ht_insert(mem, ht, inserted, inserted * 7);
    if (!err) inserted++;
  }

  EXPECT_EQ(err, SP_ERR_SYS_BUSY);
  EXPECT_EQ(sp_ht_size(ht), inserted);
  EXPECT_TRUE(sp_ht_getp(ht, inserted) == SP_NULLPTR);
  sp_for(it, inserted) {
    u32* value = sp_ht_getp(ht, (u32)it);
    ASSERT_TRUE(value != SP_NULLPTR);
    EXPECT_EQ(*value, it * 7);
  }

  ASSERT_EQ(sp_ht_insert(mem, ht, inserted, inserted * 7), SP_OK);
  EXPECT_EQ(sp_ht_size(ht), inserted + 1);
  sp_for(it, inserted + 1) {
    EXPECT_EQ(*sp_ht_getp(ht, (u32)it), it * 7);
  }

  sp_ht_free(mem, ht);
}
