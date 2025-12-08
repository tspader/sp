#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  s32 id;
  s32 type;
} compound_key_t;

sp_hash_t sp_test_string_hash(void* key, u32 size) {
  (void)size;
  sp_str_t* str = (sp_str_t*)key;
  return sp_hash_bytes(str->data, str->len, SP_HT_HASH_SEED);
}

bool sp_test_string_compare(void* ka, void* kb, u32 size) {
  (void)size;
  sp_str_t* a = (sp_str_t*)ka;
  sp_str_t* b = (sp_str_t*)kb;
  return sp_str_equal(*a, *b);
}

UTEST(ht, basic_operations) {
  sp_ht(int, float) ht = SP_NULLPTR;

  EXPECT_EQ(sp_ht_size(ht), 0);
  EXPECT_TRUE(sp_ht_empty(ht));
  EXPECT_FALSE(sp_ht_exists(ht, 42));

  sp_ht_insert(ht, 42, 3.14f);
  EXPECT_EQ(sp_ht_size(ht), 1);
  EXPECT_FALSE(sp_ht_empty(ht));
  EXPECT_TRUE(sp_ht_exists(ht, 42));
  EXPECT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ht, 10, 1.5f);
  sp_ht_insert(ht, 20, 2.5f);
  sp_ht_insert(ht, 30, 3.5f);
  EXPECT_EQ(sp_ht_size(ht), 4);

  EXPECT_EQ(*sp_ht_getp(ht, 10), 1.5f);
  EXPECT_EQ(*sp_ht_getp(ht, 20), 2.5f);
  EXPECT_EQ(*sp_ht_getp(ht, 30), 3.5f);
  EXPECT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ht, 42, 6.28f);
  EXPECT_EQ(*sp_ht_getp(ht, 42), 6.28f);
  EXPECT_EQ(sp_ht_size(ht), 4);

  sp_ht_erase(ht, 20);
  EXPECT_FALSE(sp_ht_exists(ht, 20));
  EXPECT_EQ(sp_ht_size(ht), 3);

  sp_ht_clear(ht);
  EXPECT_EQ(sp_ht_size(ht), 0);
  EXPECT_TRUE(sp_ht_empty(ht));

  sp_ht_free(ht);
}

UTEST(ht, pointer_retrieval) {
  sp_ht(u32, double) ht = SP_NULLPTR;

  sp_ht_insert(ht, 100, 123.456);
  sp_ht_insert(ht, 200, 789.012);

  double* ptr1 = sp_ht_getp(ht, 100);
  EXPECT_NE(ptr1, SP_NULLPTR);
  EXPECT_EQ(*ptr1, 123.456);

  *ptr1 = 999.999;
  EXPECT_EQ(*sp_ht_getp(ht, 100), 999.999);

  double* ptr2 = sp_ht_getp(ht, 999);
  EXPECT_EQ(ptr2, SP_NULLPTR);

  sp_ht_free(ht);
}

UTEST(ht, struct_values) {
  sp_ht(int, vec3_t) ht = SP_NULLPTR;

  vec3_t v1 = {1.0f, 2.0f, 3.0f};
  vec3_t v2 = {4.0f, 5.0f, 6.0f};
  vec3_t v3 = {7.0f, 8.0f, 9.0f};

  sp_ht_insert(ht, 1, v1);
  sp_ht_insert(ht, 2, v2);
  sp_ht_insert(ht, 3, v3);

  vec3_t retrieved = *sp_ht_getp(ht, 2);
  EXPECT_EQ(retrieved.x, 4.0f);
  EXPECT_EQ(retrieved.y, 5.0f);
  EXPECT_EQ(retrieved.z, 6.0f);

  sp_ht_free(ht);
}

UTEST(ht, struct_keys) {
  sp_ht(compound_key_t, const char*) ht = SP_NULLPTR;

  compound_key_t k1 = {100, 1};
  compound_key_t k2 = {200, 2};
  compound_key_t k3 = {300, 3};

  sp_ht_insert(ht, k1, "First");
  sp_ht_insert(ht, k2, "Second");
  sp_ht_insert(ht, k3, "Third");

  EXPECT_EQ(sp_ht_size(ht), 3);

  compound_key_t lookup = {200, 2};
  EXPECT_TRUE(sp_ht_exists(ht, lookup));
  const char* value = *sp_ht_getp(ht, lookup);
  EXPECT_STREQ(value, "Second");

  compound_key_t missing = {200, 3};
  EXPECT_FALSE(sp_ht_exists(ht, missing));

  sp_ht_free(ht);
}

UTEST(ht, string_keys) {
  sp_ht(u64, int) ht = SP_NULLPTR;

  const char* s1 = "apple";
  const char* s2 = "banana";
  const char* s3 = "cherry";

  u64 k1 = sp_hash_cstr(s1);
  u64 k2 = sp_hash_cstr(s2);
  u64 k3 = sp_hash_cstr(s3);

  sp_ht_insert(ht, k1, 10);
  sp_ht_insert(ht, k2, 20);
  sp_ht_insert(ht, k3, 30);

  u64 lookup = sp_hash_cstr("banana");
  EXPECT_TRUE(sp_ht_exists(ht, lookup));
  EXPECT_EQ(*sp_ht_getp(ht, lookup), 20);

  lookup = sp_hash_cstr("dragonfruit");
  EXPECT_FALSE(sp_ht_exists(ht, lookup));

  sp_ht_free(ht);
}

UTEST(ht, collision_handling) {
  sp_ht(int, int) ht = SP_NULLPTR;

  for (s32 i = 0; i < 100; i++) {
      sp_ht_insert(ht, i, i * 100);
  }

  EXPECT_EQ(sp_ht_size(ht), 100);

  for (s32 i = 0; i < 100; i++) {
      EXPECT_TRUE(sp_ht_exists(ht, i));
      EXPECT_EQ(*sp_ht_getp(ht, i), i * 100);
  }

  for (s32 i = 0; i < 100; i += 3) {
      sp_ht_erase(ht, i);
  }

  for (s32 i = 0; i < 100; i++) {
      if (i % 3 == 0) {
          EXPECT_FALSE(sp_ht_exists(ht, i));
      } else {
          EXPECT_TRUE(sp_ht_exists(ht, i));
          EXPECT_EQ(*sp_ht_getp(ht, i), i * 100);
      }
  }

  sp_ht_free(ht);
}

UTEST(ht, iteration) {
  sp_ht(s32, s32) ht = SP_NULLPTR;

  for (s32 i = 0; i < 10; i++) {
    sp_ht_insert(ht, i * 10, i);
  }

  s32 count = 0;
  s32 sum = 0;

  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    float val = *sp_ht_it_getp(ht, it);

    count++;
    sum += val;
  }

  EXPECT_EQ(count, 10);
  EXPECT_EQ(sum, 45);

  sp_ht_free(ht);
}

UTEST(ht, edge_cases) {
  sp_ht(int, int) ht1 = SP_NULLPTR;
  EXPECT_EQ(sp_ht_size(ht1), 0);
  EXPECT_TRUE(sp_ht_empty(ht1));
  EXPECT_FALSE(sp_ht_exists(ht1, 42));

  sp_ht_clear(ht1);
  sp_ht_free(ht1);

  sp_ht(int, int) ht2 = SP_NULLPTR;
  sp_ht_insert(ht2, 1, 100);
  sp_ht_erase(ht2, 1);
  EXPECT_EQ(sp_ht_size(ht2), 0);
  sp_ht_free(ht2);

  sp_ht(int, int) ht3 = SP_NULLPTR;
  sp_ht_insert(ht3, 1, 100);
  sp_ht_erase(ht3, 999);
  EXPECT_EQ(sp_ht_size(ht3), 1);
  sp_ht_free(ht3);
}

UTEST(ht, pathological_all_same_hash) {
  sp_ht(u32, u32) ht = sp_ht_new(u32, u32);

  u32 cap = sp_ht_capacity(ht);
  if (cap < 2) {
    sp_ht_insert(ht, 0, 0);
    sp_ht_insert(ht, 1, 0);
    cap = sp_ht_capacity(ht);
  }

  for (u32 i = 0; i < cap; i++) {
    sp_ht_insert(ht, i, i * 100);
  }

  for (u32 i = 0; i < cap; i++) {
    EXPECT_TRUE(sp_ht_exists(ht, i));
    EXPECT_EQ(i * 100, *sp_ht_getp(ht, i));
  }

  sp_ht_free(ht);
}

UTEST(ht, duplicate_key_insert_size_bug) {
  sp_ht(u32, u32) table = SP_NULLPTR;

  sp_ht_insert(table, 42, 100);
  EXPECT_EQ(sp_ht_size(table), 1);
  EXPECT_EQ(*sp_ht_getp(table, 42), 100);

  sp_ht_insert(table, 42, 200);
  EXPECT_EQ(sp_ht_size(table), 1);
  EXPECT_EQ(*sp_ht_getp(table, 42), 200);

  sp_ht_insert(table, 99, 300);
  EXPECT_EQ(sp_ht_size(table), 2);
  EXPECT_EQ(*sp_ht_getp(table, 42), 200);
  EXPECT_EQ(*sp_ht_getp(table, 99), 300);

  sp_ht_free(table);
}

UTEST(ht, iterator_yields_inactive_entry_at_slot_zero) {
  sp_ht(u64, u64) ht = sp_ht_new(u64, u64);

  sp_ht_insert(ht, 0, 999);
  sp_ht_erase(ht, 0);

  EXPECT_EQ(sp_ht_size(ht), 0);

  u64 num_entries = 0;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    num_entries++;
  }

  EXPECT_EQ(num_entries, 0);

  sp_ht_free(ht);
}

UTEST(ht, collision) {
  sp_ht(s32, s32) ht = SP_NULLPTR;

  for (u32 i = 0; i < 8; i++) {
    sp_ht_insert(ht, i, i);
  }

  u32 capacity = sp_ht_capacity(ht);
  s32 keys [3] = { -1, -1, -1 };
  s32 num_found = 0;

  for (u32 candidate = 0; candidate < 1000; candidate++) {
    sp_hash_t hash = sp_hash_bytes(&candidate, sizeof(candidate), SP_HT_HASH_SEED);

    u32 bucket = hash % capacity;
    if (bucket == 0) {
      keys[num_found++] = candidate;
    }

    if (num_found == SP_CARR_LEN(keys)) {
      break;
    }
  }

  EXPECT_EQ(num_found, 3);

  sp_ht_clear(ht);

  sp_ht_insert(ht, keys[0], 0);
  sp_ht_insert(ht, keys[1], 1);
  sp_ht_insert(ht, keys[2], 2);

  EXPECT_TRUE(sp_ht_exists(ht, keys[0]));
  EXPECT_TRUE(sp_ht_exists(ht, keys[1]));
  EXPECT_TRUE(sp_ht_exists(ht, keys[2]));

  EXPECT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  EXPECT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  EXPECT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_erase(ht, keys[0]);

  EXPECT_FALSE(sp_ht_exists(ht, keys[0]));
  EXPECT_TRUE(sp_ht_exists(ht, keys[1]));
  EXPECT_TRUE(sp_ht_exists(ht, keys[2]));

  EXPECT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  EXPECT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_insert(ht, keys[0], 0);

  EXPECT_EQ(sp_ht_size(ht), 3);
  EXPECT_TRUE(sp_ht_exists(ht, keys[0]));
  EXPECT_EQ(*sp_ht_getp(ht, keys[0]), 0);

  sp_ht_erase(ht, keys[2]);

  EXPECT_TRUE(sp_ht_exists(ht, keys[0]));
  EXPECT_TRUE(sp_ht_exists(ht, keys[1]));
  EXPECT_FALSE(sp_ht_exists(ht, keys[2]));

  EXPECT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  EXPECT_EQ(*sp_ht_getp(ht, keys[1]), 1);

  sp_ht_insert(ht, keys[2], 2);
  EXPECT_TRUE(sp_ht_exists(ht, keys[2]));
  EXPECT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_free(ht);
}

UTEST(ht, iterator_returns_zero_entries_for_populated_table) {
  sp_ht(u64, u64) ht = sp_ht_new(u64, u64);

  sp_ht_insert(ht, 1, 100);
  sp_ht_insert(ht, 2, 200);

  u64 count = 0;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    count++;
  }

  EXPECT_EQ(count, 2);

  sp_ht_free(ht);
}

UTEST(ht, null_safety) {
  sp_ht(s32, s32) null_ht = NULL;

  EXPECT_EQ(sp_ht_size(null_ht), 0);
  EXPECT_EQ(sp_ht_capacity(null_ht), 0);
  EXPECT_TRUE(sp_ht_empty(null_ht));
  EXPECT_FALSE(sp_ht_exists(null_ht, 42));

  s32* default_val = sp_ht_getp(null_ht, 42);
  EXPECT_EQ(default_val, SP_NULLPTR);

  sp_ht_it it = sp_ht_it_init(null_ht);
  EXPECT_EQ(it, 0);
  EXPECT_FALSE(sp_ht_it_valid(null_ht, it));

  sp_ht_it_advance(null_ht, it);

  s32* iter_key = sp_ht_it_getkp(null_ht, it);
  s32* iter_val = sp_ht_it_getp(null_ht, it);
  EXPECT_EQ(iter_key, SP_NULLPTR);
  EXPECT_EQ(iter_val, SP_NULLPTR);

  sp_ht_clear(null_ht);
  sp_ht_erase(null_ht, 42);
  sp_ht_free(null_ht);

  sp_ht_insert(null_ht, 42, 100);
  EXPECT_NE(null_ht, SP_NULLPTR);
  EXPECT_EQ(sp_ht_size(null_ht), 1);
  EXPECT_EQ(*sp_ht_getp(null_ht, 42), 100);

  sp_ht_free(null_ht);
}

UTEST(ht, string_key_custom_hash) {
  sp_ht(sp_str_t, int) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_str_t ka = sp_str_copy(sp_str_lit("hello"));
  sp_str_t kb = sp_str_copy(sp_str_lit("world"));
  sp_str_t kc = sp_str_copy(sp_str_lit("test"));

  sp_ht_insert(ht, ka, 100);
  sp_ht_insert(ht, kb, 200);
  sp_ht_insert(ht, kc, 300);

  EXPECT_TRUE(sp_ht_exists(ht, ka));
  EXPECT_TRUE(sp_ht_exists(ht, kb));
  EXPECT_TRUE(sp_ht_exists(ht, kc));

  EXPECT_EQ(*sp_ht_getp(ht, ka), 100);
  EXPECT_EQ(*sp_ht_getp(ht, kb), 200);
  EXPECT_EQ(*sp_ht_getp(ht, kc), 300);

  sp_str_t ka_copy = sp_str_copy(SP_LIT("hello"));
  sp_str_t kb_copy = sp_str_copy(SP_LIT("world"));

  EXPECT_TRUE(sp_ht_exists(ht, ka_copy));
  EXPECT_TRUE(sp_ht_exists(ht, kb_copy));

  EXPECT_EQ(*sp_ht_getp(ht, ka_copy), 100);
  EXPECT_EQ(*sp_ht_getp(ht, kb_copy), 200);

  sp_str_t kd = sp_str_copy(SP_LIT("missing"));
  EXPECT_FALSE(sp_ht_exists(ht, kd));
  EXPECT_EQ(sp_ht_getp(ht, kd), SP_NULLPTR);

  sp_ht_free(ht);
}

UTEST(siphash, consistency) {
  const char* data = "Hello, World!";
  u64 seed = 0x12345678;

  u64 hash1 = sp_hash_bytes((void*)data, strlen(data), seed);
  u64 hash2 = sp_hash_bytes((void*)data, strlen(data), seed);

  EXPECT_EQ(hash1, hash2);

  u64 hash3 = sp_hash_bytes((void*)data, strlen(data), seed + 1);
  EXPECT_NE(hash1, hash3);
}

UTEST(siphash, different_lengths) {
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

UTEST(siphash, collision_resistance) {
  u64 seed = 0x31415926;

  const s32 count = 1000;
  u64* hashes = (u64*)sp_alloc(sizeof(u64) * count);

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

  sp_free(hashes);
}

UTEST(combined, hash_table_with_dyn_array_values) {
    typedef int* int_array;
    sp_ht(int, int_array) ht = SP_NULLPTR;

    for (s32 i = 0; i < 5; i++) {
        sp_dyn_array(int) arr = SP_NULLPTR;

        for (s32 j = 0; j < 10; j++) {
            sp_dyn_array_push(arr, i * 100 + j);
        }

        sp_ht_insert(ht, i, arr);
    }

    for (s32 i = 0; i < 5; i++) {
        EXPECT_TRUE(sp_ht_exists(ht, i));

        int_array arr = *sp_ht_getp(ht, i);
        EXPECT_EQ(sp_dyn_array_size(arr), 10);

        for (s32 j = 0; j < 10; j++) {
            EXPECT_EQ(arr[j], i * 100 + j);
        }
    }

    for (s32 i = 0; i < 5; i++) {
        int_array arr = *sp_ht_getp(ht, i);
        sp_dyn_array_free(arr);
    }

    sp_ht_free(ht);
}

UTEST(combined, multiple_arrays_in_hash_table) {
  sp_ht(int, void*) ht = SP_NULLPTR;

  for (s32 key = 0; key < 5; key++) {
    sp_dyn_array(int) arr = SP_NULLPTR;

    for (s32 j = 0; j < 20; j++) {
        sp_dyn_array_push(arr, key * 1000 + j);
    }

    sp_ht_insert(ht, key, (void*)arr);
  }

  for (s32 key = 0; key < 5; key++) {
    EXPECT_TRUE(sp_ht_exists(ht, key));

    int* arr = (int*)*sp_ht_getp(ht, key);
    EXPECT_EQ(sp_dyn_array_size(arr), 20);

    for (s32 j = 0; j < 20; j++) {
        EXPECT_EQ(arr[j], key * 1000 + j);
    }
  }

  for (s32 key = 0; key < 5; key++) {
    int* arr = (int*)*sp_ht_getp(ht, key);
    sp_dyn_array_free(arr);
  }

  sp_ht_free(ht);
}

UTEST(ht_front, null_table) {
  sp_ht(int, int) ht = NULL;
  EXPECT_EQ(sp_ht_front(ht), NULL);
}

UTEST(ht_front, empty_table) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ht);
  EXPECT_EQ(sp_ht_front(ht), NULL);
  sp_ht_free(ht);
}

UTEST(ht_front, single_item) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_insert(ht, 42, 100);

  int* front = sp_ht_front(ht);
  EXPECT_NE(front, NULL);
  EXPECT_EQ(*front, 100);

  sp_ht_free(ht);
}

UTEST(ht_front, multiple_items) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_insert(ht, 1, 10);
  sp_ht_insert(ht, 2, 20);
  sp_ht_insert(ht, 3, 30);

  int* front = sp_ht_front(ht);
  EXPECT_NE(front, NULL);

  bool found = false;
  sp_ht_for(ht, it) {
    if (sp_ht_it_getp(ht, it) == front) {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);

  sp_ht_free(ht);
}

UTEST(ht_front, after_erase) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_insert(ht, 1, 10);
  sp_ht_insert(ht, 2, 20);

  int* first_front = sp_ht_front(ht);
  EXPECT_NE(first_front, NULL);

  int first_val = *first_front;

  if (first_val == 10) {
    sp_ht_erase(ht, 1);
    int* new_front = sp_ht_front(ht);
    EXPECT_NE(new_front, NULL);
    EXPECT_EQ(*new_front, 20);
  } else {
    sp_ht_erase(ht, 2);
    int* new_front = sp_ht_front(ht);
    EXPECT_NE(new_front, NULL);
    EXPECT_EQ(*new_front, 10);
  }

  sp_ht_free(ht);
}

UTEST(ht, for_kv_iteration) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht(s32, bool) visited = SP_NULLPTR;

  sp_for(it, 10) {
    sp_ht_insert(ht, it * 10, it);
  }

  s32 count = 0;
  s32 key_sum = 0;
  s32 val_sum = 0;

  sp_ht_for_kv(ht, it) {
    sp_ht_insert(visited, *it.key, true);
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

  sp_ht_free(ht);
}

UTEST(ht, for_kv_string_keys) {
  sp_ht(sp_str_t, s32) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_ht(sp_str_t, bool) visited = SP_NULLPTR;
  sp_ht_set_fns(visited, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_ht_insert(ht, sp_str_lit("one"), 1);
  sp_ht_insert(ht, sp_str_lit("two"), 2);
  sp_ht_insert(ht, sp_str_lit("three"), 3);

  s32 count = 0;
  s32 val_sum = 0;

  sp_ht_for_kv(ht, it) {
    EXPECT_FALSE(sp_str_empty(*it.key));
    sp_ht_insert(visited, *it.key, true);
    val_sum += *it.val;
    count++;
  }

  EXPECT_EQ(count, 3);
  EXPECT_EQ(val_sum, 6);
  EXPECT_NE(sp_ht_getp(visited, sp_str_lit("one")), SP_NULLPTR);
  EXPECT_NE(sp_ht_getp(visited, sp_str_lit("two")), SP_NULLPTR);
  EXPECT_NE(sp_ht_getp(visited, sp_str_lit("three")), SP_NULLPTR);

  sp_ht_free(ht);
}

UTEST(ht, for_kv_empty_table) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_init(ht);

  s32 count = 0;
  sp_ht_for_kv(ht, it) {
    (void)it;
    count++;
  }

  EXPECT_EQ(count, 0);

  sp_ht_free(ht);
}

UTEST(ht, for_kv_null_table) {
  sp_ht(s32, s32) ht = SP_NULLPTR;

  s32 count = 0;
  sp_ht_for_kv(ht, it) {
    (void)it;
    count++;
  }

  EXPECT_EQ(count, 0);
}

// Issue tests: reproduce bugs documented in doc/hash-table.md

static sp_hash_t sp_test_constant_hash(void* key, u32 size) {
  (void)key;
  (void)size;
  return 42;
}

static bool sp_test_s32_compare(void* ka, void* kb, u32 size) {
  (void)size;
  return *(s32*)ka == *(s32*)kb;
}

// Hash collision: insert uses hash equality instead of key equality for probing.
// Constant hash forces all keys to same bucket; different keys overwrite each other.
UTEST(ht_issue, hash_collision_overwrites_different_keys) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_test_constant_hash, sp_test_s32_compare);

  sp_ht_insert(ht, 100, 1);
  sp_ht_insert(ht, 200, 2);
  sp_ht_insert(ht, 300, 3);

  EXPECT_EQ(sp_ht_size(ht), 3);
  EXPECT_TRUE(sp_ht_exists(ht, 100));
  EXPECT_TRUE(sp_ht_exists(ht, 200));
  EXPECT_TRUE(sp_ht_exists(ht, 300));

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

  sp_ht_free(ht);
}

static sp_hash_t sp_test_identity_hash(void* key, u32 size) {
  (void)size;
  return (sp_hash_t)(*(s32*)key);
}

// Rehash: resize extends array but doesn't rehash existing entries.
// Key 2 at cap=2 goes to slot 2%2=0. After resize to cap=4, it should be at 2%4=2.
// Directly check that entry is at correct slot after resize.
UTEST(ht_issue, rehash_breaks_lookups_after_resize) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_test_identity_hash, sp_test_s32_compare);
  u32 ka = sp_ht_capacity(ht);
  u32 slot_before_resize = ka % sp_ht_capacity(ht);
  u32 initial_capacity = sp_ht_capacity(ht);

  sp_ht_insert(ht, ka, 2000);
  EXPECT_EQ(ht->data[slot_before_resize].key, ka);

  // force a resize
  sp_ht_insert(ht, 1, 0);
  EXPECT_GE(sp_ht_capacity(ht), initial_capacity);

  // sanity check that the key should actually be in a different slot
  u32 slot_after_resize = ka % sp_ht_capacity(ht);
  EXPECT_NE(slot_before_resize, slot_after_resize);

  // if we're rehashing correctly, the old slot is clean and the new slot has the data
  EXPECT_NE(ht->data[slot_before_resize].key, ka);
  EXPECT_EQ(ht->data[slot_after_resize].key, ka);
  EXPECT_EQ(ht->data[slot_after_resize].state, SP_HT_ENTRY_ACTIVE);

  sp_ht_free(ht);
}

static u32 g_hash_call_count = 0;

static sp_hash_t sp_test_counting_hash(void* key, u32 size) {
  g_hash_call_count++;
  return sp_hash_bytes(key, size, SP_HT_HASH_SEED);
}

static bool sp_test_counting_compare(void* ka, void* kb, u32 size) {
  return sp_mem_is_equal(ka, kb, size);
}

// Linear scan: lookup probes entire capacity instead of stopping at INACTIVE.
// With 1 element in cap=64 table, missing key lookup should be O(1), not O(N).
UTEST(ht_issue, linear_scan_calls_hash_too_many_times) {
  sp_ht(s32, s32) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_test_counting_hash, sp_test_counting_compare);

  for (s32 i = 0; i < 32; i++) {
    sp_ht_insert(ht, i, i);
  }
  u32 capacity = sp_ht_capacity(ht);
  sp_ht_clear(ht);

  g_hash_call_count = 0;
  sp_ht_insert(ht, 999, 42);

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
  sp_ht_free(ht);
}

SP_TEST_MAIN()
