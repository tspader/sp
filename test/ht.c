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

UTEST(hash_table, basic_operations) {
  sp_ht(int, float) ht = SP_NULLPTR;

  ASSERT_EQ(sp_ht_size(ht), 0);
  ASSERT_TRUE(sp_ht_empty(ht));
  ASSERT_FALSE(sp_ht_exists(ht, 42));

  sp_ht_insert(ht, 42, 3.14f);
  ASSERT_EQ(sp_ht_size(ht), 1);
  ASSERT_FALSE(sp_ht_empty(ht));
  ASSERT_TRUE(sp_ht_exists(ht, 42));
  ASSERT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ht, 10, 1.5f);
  sp_ht_insert(ht, 20, 2.5f);
  sp_ht_insert(ht, 30, 3.5f);
  ASSERT_EQ(sp_ht_size(ht), 4);

  ASSERT_EQ(*sp_ht_getp(ht, 10), 1.5f);
  ASSERT_EQ(*sp_ht_getp(ht, 20), 2.5f);
  ASSERT_EQ(*sp_ht_getp(ht, 30), 3.5f);
  ASSERT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ht, 42, 6.28f);
  ASSERT_EQ(*sp_ht_getp(ht, 42), 6.28f);
  ASSERT_EQ(sp_ht_size(ht), 4);

  sp_ht_erase(ht, 20);
  ASSERT_FALSE(sp_ht_exists(ht, 20));
  ASSERT_EQ(sp_ht_size(ht), 3);

  sp_ht_clear(ht);
  ASSERT_EQ(sp_ht_size(ht), 0);
  ASSERT_TRUE(sp_ht_empty(ht));

  sp_ht_free(ht);
}

UTEST(hash_table, pointer_retrieval) {
  sp_ht(u32, double) ht = SP_NULLPTR;

  sp_ht_insert(ht, 100, 123.456);
  sp_ht_insert(ht, 200, 789.012);

  double* ptr1 = sp_ht_getp(ht, 100);
  ASSERT_NE(ptr1, SP_NULLPTR);
  ASSERT_EQ(*ptr1, 123.456);

  *ptr1 = 999.999;
  ASSERT_EQ(*sp_ht_getp(ht, 100), 999.999);

  double* ptr2 = sp_ht_getp(ht, 999);
  ASSERT_EQ(ptr2, SP_NULLPTR);

  sp_ht_free(ht);
}

UTEST(hash_table, struct_values) {
  sp_ht(int, vec3_t) ht = SP_NULLPTR;

  vec3_t v1 = {1.0f, 2.0f, 3.0f};
  vec3_t v2 = {4.0f, 5.0f, 6.0f};
  vec3_t v3 = {7.0f, 8.0f, 9.0f};

  sp_ht_insert(ht, 1, v1);
  sp_ht_insert(ht, 2, v2);
  sp_ht_insert(ht, 3, v3);

  vec3_t retrieved = *sp_ht_getp(ht, 2);
  ASSERT_EQ(retrieved.x, 4.0f);
  ASSERT_EQ(retrieved.y, 5.0f);
  ASSERT_EQ(retrieved.z, 6.0f);

  sp_ht_free(ht);
}

UTEST(hash_table, struct_keys) {
  sp_ht(compound_key_t, const char*) ht = SP_NULLPTR;

  compound_key_t k1 = {100, 1};
  compound_key_t k2 = {200, 2};
  compound_key_t k3 = {300, 3};

  sp_ht_insert(ht, k1, "First");
  sp_ht_insert(ht, k2, "Second");
  sp_ht_insert(ht, k3, "Third");

  ASSERT_EQ(sp_ht_size(ht), 3);

  compound_key_t lookup = {200, 2};
  ASSERT_TRUE(sp_ht_exists(ht, lookup));
  const char* value = *sp_ht_getp(ht, lookup);
  ASSERT_STREQ(value, "Second");

  compound_key_t missing = {200, 3};
  ASSERT_FALSE(sp_ht_exists(ht, missing));

  sp_ht_free(ht);
}

UTEST(hash_table, string_keys) {
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
  ASSERT_TRUE(sp_ht_exists(ht, lookup));
  ASSERT_EQ(*sp_ht_getp(ht, lookup), 20);

  lookup = sp_hash_cstr("dragonfruit");
  ASSERT_FALSE(sp_ht_exists(ht, lookup));

  sp_ht_free(ht);
}

UTEST(hash_table, collision_handling) {
  sp_ht(int, int) ht = SP_NULLPTR;

  for (s32 i = 0; i < 100; i++) {
      sp_ht_insert(ht, i, i * 100);
  }

  ASSERT_EQ(sp_ht_size(ht), 100);

  for (s32 i = 0; i < 100; i++) {
      ASSERT_TRUE(sp_ht_exists(ht, i));
      ASSERT_EQ(*sp_ht_getp(ht, i), i * 100);
  }

  for (s32 i = 0; i < 100; i += 3) {
      sp_ht_erase(ht, i);
  }

  for (s32 i = 0; i < 100; i++) {
      if (i % 3 == 0) {
          ASSERT_FALSE(sp_ht_exists(ht, i));
      } else {
          ASSERT_TRUE(sp_ht_exists(ht, i));
          ASSERT_EQ(*sp_ht_getp(ht, i), i * 100);
      }
  }

  sp_ht_free(ht);
}

UTEST(hash_table, iteration) {
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

  ASSERT_EQ(count, 10);
  ASSERT_EQ(sum, 45);

  sp_ht_free(ht);
}

UTEST(hash_table, edge_cases) {
  sp_ht(int, int) ht1 = SP_NULLPTR;
  ASSERT_EQ(sp_ht_size(ht1), 0);
  ASSERT_TRUE(sp_ht_empty(ht1));
  ASSERT_FALSE(sp_ht_exists(ht1, 42));

  sp_ht_clear(ht1);
  sp_ht_free(ht1);

  sp_ht(int, int) ht2 = SP_NULLPTR;
  sp_ht_insert(ht2, 1, 100);
  sp_ht_erase(ht2, 1);
  ASSERT_EQ(sp_ht_size(ht2), 0);
  sp_ht_free(ht2);

  sp_ht(int, int) ht3 = SP_NULLPTR;
  sp_ht_insert(ht3, 1, 100);
  sp_ht_erase(ht3, 999);
  ASSERT_EQ(sp_ht_size(ht3), 1);
  sp_ht_free(ht3);
}

UTEST(hash_table, pathological_all_same_hash) {
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
    ASSERT_TRUE(sp_ht_exists(ht, i));
    ASSERT_EQ(i * 100, *sp_ht_getp(ht, i));
  }

  sp_ht_free(ht);
}

UTEST(hash_table, duplicate_key_insert_size_bug) {
  sp_ht(u32, u32) table = SP_NULLPTR;

  sp_ht_insert(table, 42, 100);
  ASSERT_EQ(sp_ht_size(table), 1);
  ASSERT_EQ(*sp_ht_getp(table, 42), 100);

  sp_ht_insert(table, 42, 200);
  ASSERT_EQ(sp_ht_size(table), 1);
  ASSERT_EQ(*sp_ht_getp(table, 42), 200);

  sp_ht_insert(table, 99, 300);
  ASSERT_EQ(sp_ht_size(table), 2);
  ASSERT_EQ(*sp_ht_getp(table, 42), 200);
  ASSERT_EQ(*sp_ht_getp(table, 99), 300);

  sp_ht_free(table);
}

UTEST(sp_ht, iterator_yields_inactive_entry_at_slot_zero) {
  sp_ht(u64, u64) ht = sp_ht_new(u64, u64);

  sp_ht_insert(ht, 0, 999);
  sp_ht_erase(ht, 0);

  ASSERT_EQ(sp_ht_size(ht), 0);

  u64 num_entries = 0;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    num_entries++;
  }

  ASSERT_EQ(num_entries, 0);

  sp_ht_free(ht);
}

UTEST(hash_table, collision) {
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

  ASSERT_EQ(num_found, 3);

  sp_ht_clear(ht);

  sp_ht_insert(ht, keys[0], 0);
  sp_ht_insert(ht, keys[1], 1);
  sp_ht_insert(ht, keys[2], 2);

  ASSERT_TRUE(sp_ht_exists(ht, keys[0]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[1]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[2]));

  ASSERT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  ASSERT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  ASSERT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_erase(ht, keys[0]);

  ASSERT_FALSE(sp_ht_exists(ht, keys[0]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[1]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[2]));

  ASSERT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  ASSERT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_insert(ht, keys[0], 0);

  ASSERT_EQ(sp_ht_size(ht), 3);
  ASSERT_TRUE(sp_ht_exists(ht, keys[0]));
  ASSERT_EQ(*sp_ht_getp(ht, keys[0]), 0);

  sp_ht_erase(ht, keys[2]);

  ASSERT_TRUE(sp_ht_exists(ht, keys[0]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[1]));
  ASSERT_FALSE(sp_ht_exists(ht, keys[2]));

  ASSERT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  ASSERT_EQ(*sp_ht_getp(ht, keys[1]), 1);

  sp_ht_insert(ht, keys[2], 2);
  ASSERT_TRUE(sp_ht_exists(ht, keys[2]));
  ASSERT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  sp_ht_free(ht);
}

UTEST(hash_table, iterator_returns_zero_entries_for_populated_table) {
  sp_ht(u64, u64) ht = sp_ht_new(u64, u64);

  sp_ht_insert(ht, 1, 100);
  sp_ht_insert(ht, 2, 200);

  u64 count = 0;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    count++;
  }

  ASSERT_EQ(count, 2);

  sp_ht_free(ht);
}

UTEST(hash_table, null_safety) {
  sp_ht(s32, s32) null_ht = NULL;

  ASSERT_EQ(sp_ht_size(null_ht), 0);
  ASSERT_EQ(sp_ht_capacity(null_ht), 0);
  ASSERT_TRUE(sp_ht_empty(null_ht));
  ASSERT_FALSE(sp_ht_exists(null_ht, 42));

  s32* default_val = sp_ht_getp(null_ht, 42);
  ASSERT_EQ(default_val, SP_NULLPTR);

  sp_ht_it it = sp_ht_it_init(null_ht);
  ASSERT_EQ(it, 0);
  ASSERT_FALSE(sp_ht_it_valid(null_ht, it));

  sp_ht_it_advance(null_ht, it);

  s32* iter_key = sp_ht_it_getkp(null_ht, it);
  s32* iter_val = sp_ht_it_getp(null_ht, it);
  ASSERT_EQ(iter_key, SP_NULLPTR);
  ASSERT_EQ(iter_val, SP_NULLPTR);

  sp_ht_clear(null_ht);
  sp_ht_erase(null_ht, 42);
  sp_ht_free(null_ht);

  sp_ht_insert(null_ht, 42, 100);
  ASSERT_NE(null_ht, SP_NULLPTR);
  ASSERT_EQ(sp_ht_size(null_ht), 1);
  ASSERT_EQ(*sp_ht_getp(null_ht, 42), 100);

  sp_ht_free(null_ht);
}

UTEST(hash_table, string_key_custom_hash) {
  sp_ht(sp_str_t, int) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_str_t ka = sp_str_copy(sp_str_lit("hello"));
  sp_str_t kb = sp_str_copy(sp_str_lit("world"));
  sp_str_t kc = sp_str_copy(sp_str_lit("test"));

  sp_ht_insert(ht, ka, 100);
  sp_ht_insert(ht, kb, 200);
  sp_ht_insert(ht, kc, 300);

  ASSERT_TRUE(sp_ht_exists(ht, ka));
  ASSERT_TRUE(sp_ht_exists(ht, kb));
  ASSERT_TRUE(sp_ht_exists(ht, kc));

  ASSERT_EQ(*sp_ht_getp(ht, ka), 100);
  ASSERT_EQ(*sp_ht_getp(ht, kb), 200);
  ASSERT_EQ(*sp_ht_getp(ht, kc), 300);

  sp_str_t ka_copy = sp_str_copy(SP_LIT("hello"));
  sp_str_t kb_copy = sp_str_copy(SP_LIT("world"));

  ASSERT_TRUE(sp_ht_exists(ht, ka_copy));
  ASSERT_TRUE(sp_ht_exists(ht, kb_copy));

  ASSERT_EQ(*sp_ht_getp(ht, ka_copy), 100);
  ASSERT_EQ(*sp_ht_getp(ht, kb_copy), 200);

  sp_str_t kd = sp_str_copy(SP_LIT("missing"));
  ASSERT_FALSE(sp_ht_exists(ht, kd));
  ASSERT_EQ(sp_ht_getp(ht, kd), SP_NULLPTR);

  sp_ht_free(ht);
}

UTEST(siphash, consistency) {
  const char* data = "Hello, World!";
  u64 seed = 0x12345678;

  u64 hash1 = sp_hash_bytes((void*)data, strlen(data), seed);
  u64 hash2 = sp_hash_bytes((void*)data, strlen(data), seed);

  ASSERT_EQ(hash1, hash2);

  u64 hash3 = sp_hash_bytes((void*)data, strlen(data), seed + 1);
  ASSERT_NE(hash1, hash3);
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

  ASSERT_NE(h1, h2);
  ASSERT_NE(h2, h3);
  ASSERT_NE(h3, h4);
  ASSERT_NE(h4, h5);
  ASSERT_NE(h5, h6);
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

  ASSERT_EQ(collisions, 0);

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
        ASSERT_TRUE(sp_ht_exists(ht, i));

        int_array arr = *sp_ht_getp(ht, i);
        ASSERT_EQ(sp_dyn_array_size(arr), 10);

        for (s32 j = 0; j < 10; j++) {
            ASSERT_EQ(arr[j], i * 100 + j);
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
    ASSERT_TRUE(sp_ht_exists(ht, key));

    int* arr = (int*)*sp_ht_getp(ht, key);
    ASSERT_EQ(sp_dyn_array_size(arr), 20);

    for (s32 j = 0; j < 20; j++) {
        ASSERT_EQ(arr[j], key * 1000 + j);
    }
  }

  for (s32 key = 0; key < 5; key++) {
    int* arr = (int*)*sp_ht_getp(ht, key);
    sp_dyn_array_free(arr);
  }

  sp_ht_free(ht);
}

UTEST(sp_ht_front, null_table) {
  sp_ht(int, int) ht = NULL;
  ASSERT_EQ(sp_ht_front(ht), NULL);
}

UTEST(sp_ht_front, empty_table) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_init(ht);
  ASSERT_EQ(sp_ht_front(ht), NULL);
  sp_ht_free(ht);
}

UTEST(sp_ht_front, single_item) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_insert(ht, 42, 100);

  int* front = sp_ht_front(ht);
  ASSERT_NE(front, NULL);
  ASSERT_EQ(*front, 100);

  sp_ht_free(ht);
}

UTEST(sp_ht_front, multiple_items) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_insert(ht, 1, 10);
  sp_ht_insert(ht, 2, 20);
  sp_ht_insert(ht, 3, 30);

  int* front = sp_ht_front(ht);
  ASSERT_NE(front, NULL);

  bool found = false;
  sp_ht_for(ht, it) {
    if (sp_ht_it_getp(ht, it) == front) {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);

  sp_ht_free(ht);
}

UTEST(sp_ht_front, after_erase) {
  sp_ht(int, int) ht = SP_NULLPTR;
  sp_ht_insert(ht, 1, 10);
  sp_ht_insert(ht, 2, 20);

  int* first_front = sp_ht_front(ht);
  ASSERT_NE(first_front, NULL);

  int first_val = *first_front;

  if (first_val == 10) {
    sp_ht_erase(ht, 1);
    int* new_front = sp_ht_front(ht);
    ASSERT_NE(new_front, NULL);
    ASSERT_EQ(*new_front, 20);
  } else {
    sp_ht_erase(ht, 2);
    int* new_front = sp_ht_front(ht);
    ASSERT_NE(new_front, NULL);
    ASSERT_EQ(*new_front, 10);
  }

  sp_ht_free(ht);
}

UTEST_MAIN()
