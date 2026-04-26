#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"


SP_TEST_MAIN()

UTEST(dyn_array, basic_operations) {
    sp_da(int) arr = SP_NULLPTR;

    ASSERT_EQ(sp_da_size(arr), 0);
    ASSERT_EQ(sp_da_capacity(arr), 0);
    ASSERT_TRUE(sp_da_empty(arr));

    sp_da_push(arr, 42);
    ASSERT_EQ(sp_da_size(arr), 1);
    ASSERT_GE(sp_da_capacity(arr), 1);
    ASSERT_FALSE(sp_da_empty(arr));
    ASSERT_EQ(arr[0], 42);

    for (s32 i = 1; i < 10; i++) {
        sp_da_push(arr, i * 10);
    }
    ASSERT_EQ(sp_da_size(arr), 10);

    ASSERT_EQ(arr[0], 42);
    for (s32 i = 1; i < 10; i++) {
        ASSERT_EQ(arr[i], i * 10);
    }

    sp_da_pop(arr);
    ASSERT_EQ(sp_da_size(arr), 9);

    ASSERT_EQ(*sp_da_back(arr), 80);

    sp_da_clear(arr);
    ASSERT_EQ(sp_da_size(arr), 0);
    ASSERT_TRUE(sp_da_empty(arr));

    sp_da_free(arr);
}

UTEST(dyn_array, reserve_capacity) {


    sp_da(float) arr = SP_NULLPTR;

    sp_da_reserve(arr, 100);
    ASSERT_GE(sp_da_capacity(arr), 100);
    ASSERT_EQ(sp_da_size(arr), 0);

    for (s32 i = 0; i < 50; i++) {
        sp_da_push(arr, (float)i * 0.5f);
    }
    ASSERT_GE(sp_da_capacity(arr), 100);
    ASSERT_EQ(sp_da_size(arr), 50);

    sp_da_free(arr);
}

UTEST(dyn_array, growth_pattern) {


    sp_da(u32) arr = SP_NULLPTR;

    u32 prev_capacity = 0;

    for (u32 i = 0; i < 100; i++) {
        sp_da_push(arr, i);

        u32 current_capacity = sp_da_capacity(arr);
        if (current_capacity != prev_capacity) {
            if (prev_capacity > 0) {
                ASSERT_EQ(current_capacity, prev_capacity * 2);
            }
            prev_capacity = current_capacity;
        }
    }

    ASSERT_EQ(sp_da_size(arr), 100);

    sp_da_free(arr);
}

typedef struct test_struct {
    s32 id;
    float value;
    char name[32];
} test_struct;

UTEST(dyn_array, struct_type) {
    sp_da(test_struct) arr = SP_NULLPTR;

    for (s32 i = 0; i < 10; i++) {
        test_struct s = SP_ZERO_INITIALIZE();
        s.id = i;
        s.value = (float)i * 1.5f;
        sp_io_writer_t io = sp_zero();
        sp_io_writer_from_mem(&io, s.name, sizeof(s.name));
        sp_io_write_str(&io, sp_fmt("Item_{}", sp_fmt_int(s.id)), SP_NULLPTR);
        sp_io_pad(&io, 1, SP_NULLPTR);
        //sp_io_write_cstr(&io, "\0", SP_NULLPTR);
        //snprintf(s.name, sizeof(s.name), "Item_%d", i);
        sp_da_push(arr, s);
    }

    ASSERT_EQ(sp_da_size(arr), 10);

    for (s32 i = 0; i < 10; i++) {
        ASSERT_EQ(arr[i].id, i);
        ASSERT_EQ(arr[i].value, (float)i * 1.5f);

        char expected[32];
        sp_io_writer_t io = sp_zero();
        sp_io_writer_from_mem(&io, expected, sizeof(expected));
        sp_io_write_str(&io, sp_fmt("Item_{}", sp_fmt_int(i)), SP_NULLPTR);
        sp_io_pad(&io, 1, SP_NULLPTR);
        //sp_io_write_cstr(&io, "\0", SP_NULLPTR);

        //snprintf(expected, sizeof(expected), "Item_%d", i);
        ASSERT_STREQ(arr[i].name, expected);
    }

    sp_da_free(arr);
}

UTEST(dyn_array, pointer_type) {


    sp_da(char*) arr = SP_NULLPTR;

    const char* strings[] = {"Hello", "World", "Dynamic", "Array", "Test"};

    for (s32 i = 0; i < 5; i++) {
        // c8* str = (c8*)sp_alloc(strlen(strings[i]) + 1);
        // strcpy(str, strings[i]);
        c8* str = sp_cstr_copy(strings[i]);
        sp_da_push(arr, str);
    }

    ASSERT_EQ(sp_da_size(arr), 5);

    for (u32 i = 0; i < 5; i++) {
        ASSERT_STREQ(arr[i], strings[i]);
    }

    for (u64 i = 0; i < sp_da_size(arr); i++) {
        sp_free(arr[i]);
    }

    sp_da_free(arr);
}

UTEST(dyn_array, edge_cases) {


    sp_da(int) arr1 = SP_NULLPTR;
    sp_da_free(arr1);
    sp_da_free(arr1);

    sp_da(int) arr2 = SP_NULLPTR;
    sp_da_pop(arr2);
    ASSERT_EQ(sp_da_size(arr2), 0);

    sp_da_clear(arr2);

    sp_da_push(arr2, 42);
    sp_da_free(arr2);

    sp_da(int) arr3 = SP_NULLPTR;
    sp_da_reserve(arr3, 0);
    ASSERT_GE(sp_da_capacity(arr3), 0);
    sp_da_free(arr3);
}

UTEST(dyn_array, basic_int_push) {


    int* arr = SP_NULLPTR;

    ASSERT_EQ(sp_da_size(arr), 0);
    ASSERT_EQ(sp_da_capacity(arr), 0);

    int val1 = 42;
    sp_da_push_ex((void**)&arr, &val1, sizeof(int));

    ASSERT_NE(arr, SP_NULLPTR);
    ASSERT_EQ(sp_da_size(arr), 1);
    ASSERT_GE(sp_da_capacity(arr), 1);
    ASSERT_EQ(arr[0], 42);

    int val2 = 69;
    sp_da_push_ex((void**)&arr, &val2, sizeof(int));
    ASSERT_EQ(sp_da_size(arr), 2);
    ASSERT_EQ(arr[1], 69);

    int val3 = 420;
    sp_da_push_ex((void**)&arr, &val3, sizeof(int));
    ASSERT_EQ(sp_da_size(arr), 3);
    ASSERT_EQ(arr[2], 420);

    ASSERT_EQ(arr[0], 42);
    ASSERT_EQ(arr[1], 69);
    ASSERT_EQ(arr[2], 420);

    sp_da_free(arr);
}

UTEST(dyn_array, different_types) {


    {
        u8* arr = SP_NULLPTR;
        for (u8 i = 0; i < 10; i++) {
            sp_da_push_ex((void**)&arr, &i, sizeof(u8));
        }
        ASSERT_EQ(sp_da_size(arr), 10);
        for (u8 i = 0; i < 10; i++) {
            ASSERT_EQ(arr[i], i);
        }
        sp_da_free(arr);
    }

    {
        u16* arr = SP_NULLPTR;
        u16 vals[] = {100, 200, 300, 400, 500};
        for (int i = 0; i < 5; i++) {
            sp_da_push_ex((void**)&arr, &vals[i], sizeof(u16));
        }
        ASSERT_EQ(sp_da_size(arr), 5);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(arr[i], vals[i]);
        }
        sp_da_free(arr);
    }

    {
        u64* arr = SP_NULLPTR;
        u64 val = 0xDEADBEEFCAFEBABE;
        sp_da_push_ex((void**)&arr, &val, sizeof(u64));
        ASSERT_EQ(sp_da_size(arr), 1);
        ASSERT_EQ(arr[0], 0xDEADBEEFCAFEBABE);
        sp_da_free(arr);
    }

    {
        float* arr = SP_NULLPTR;
        float vals[] = {3.14f, 2.71f, 1.41f};
        for (int i = 0; i < 3; i++) {
            sp_da_push_ex((void**)&arr, &vals[i], sizeof(float));
        }
        ASSERT_EQ(sp_da_size(arr), 3);
        ASSERT_NEAR(arr[0], 3.14f, 0.001f);
        ASSERT_NEAR(arr[1], 2.71f, 0.001f);
        ASSERT_NEAR(arr[2], 1.41f, 0.001f);
        sp_da_free(arr);
    }
}

UTEST(dyn_array, push_struct) {
    typedef struct {
        int id;
        float value;
        u8 flags;
    } test_struct_t;

    test_struct_t* arr = SP_NULLPTR;

    test_struct_t item1 = {.id = 1, .value = 3.14f, .flags = 0xFF};
    sp_da_push_ex((void**)&arr, &item1, sizeof(test_struct_t));

    test_struct_t item2 = {.id = 2, .value = 2.71f, .flags = 0x42};
    sp_da_push_ex((void**)&arr, &item2, sizeof(test_struct_t));

    test_struct_t item3 = {.id = 3, .value = 1.41f, .flags = 0x69};
    sp_da_push_ex((void**)&arr, &item3, sizeof(test_struct_t));

    ASSERT_EQ(sp_da_size(arr), 3);

    ASSERT_EQ(arr[0].id, 1);
    ASSERT_NEAR(arr[0].value, 3.14f, 0.001f);
    ASSERT_EQ(arr[0].flags, 0xFF);

    ASSERT_EQ(arr[1].id, 2);
    ASSERT_NEAR(arr[1].value, 2.71f, 0.001f);
    ASSERT_EQ(arr[1].flags, 0x42);

    ASSERT_EQ(arr[2].id, 3);
    ASSERT_NEAR(arr[2].value, 1.41f, 0.001f);
    ASSERT_EQ(arr[2].flags, 0x69);

    sp_da_free(arr);
}

UTEST(dyn_array, growth_behavior) {


    int* arr = SP_NULLPTR;

    for (int i = 0; i < 100; i++) {
        sp_da_push_ex((void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_da_size(arr), 100);
    ASSERT_GE(sp_da_capacity(arr), 100);

    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(arr[i], i);
    }

    u64 old_capacity = sp_da_capacity(arr);
    int val = 1000;
    while (sp_da_size(arr) < sp_da_capacity(arr)) {
        sp_da_push_ex((void**)&arr, &val, sizeof(int));
    }

    sp_da_push_ex((void**)&arr, &val, sizeof(int));
    ASSERT_GT(sp_da_capacity(arr), old_capacity);

    sp_da_free(arr);
}

UTEST(dyn_array, alignment_test) {
  typedef struct {
    u8 a;
    u64 b;
    u8 c;
  } aligned_struct_t;

  aligned_struct_t* arr = SP_NULLPTR;

  for (int i = 0; i < 10; i++) {
    aligned_struct_t item = {.a = (u8)i, .b = (u64)(i * 1000), .c = (u8)(255 - i)};
    sp_da_push_ex((void**)&arr, &item, sizeof(aligned_struct_t));
  }

  ASSERT_EQ(sp_da_size(arr), 10);

  for (int i = 0; i < 10; i++) {
    ASSERT_EQ(arr[i].a, i);
    ASSERT_EQ(arr[i].b, i * 1000);
    ASSERT_EQ(arr[i].c, 255 - i);
  }

  sp_da_free(arr);
}

UTEST(dyn_array, zero_initialization) {
  typedef struct {
    int values[10];
  } big_struct_t;

  big_struct_t* arr = SP_NULLPTR;
  big_struct_t zero_struct = {0};

  for (int i = 0; i < 5; i++) {
    sp_da_push_ex((void**)&arr, &zero_struct, sizeof(big_struct_t));
  }

  ASSERT_EQ(sp_da_size(arr), 5);

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 10; j++) {
      ASSERT_EQ(arr[i].values[j], 0);
    }
  }

  sp_da_free(arr);
}

UTEST(dyn_array, mixed_with_macros) {
  int* arr = SP_NULLPTR;

  int val1 = 10;
  sp_da_push_ex((void**)&arr, &val1, sizeof(int));

  sp_da_push(arr, 20);

  int val3 = 30;
  sp_da_push_ex((void**)&arr, &val3, sizeof(int));

  sp_da_push(arr, 40);

  ASSERT_EQ(sp_da_size(arr), 4);
  ASSERT_EQ(arr[0], 10);
  ASSERT_EQ(arr[1], 20);
  ASSERT_EQ(arr[2], 30);
  ASSERT_EQ(arr[3], 40);

  sp_da_free(arr);
}


UTEST(dyn_array, push_edge_cases) {
  {
    u8* arr = SP_NULLPTR;
    u8 single_byte = 0xFF;
    sp_da_push_ex((void**)&arr, &single_byte, sizeof(c8));
    ASSERT_EQ(sp_da_size(arr), 1);
    ASSERT_EQ(arr[0], 0xFFu);
    sp_da_free(arr);
  }

  {
    int* arr = SP_NULLPTR;
    sp_da_reserve(arr, 50);

    for (int i = 0; i < 25; i++) {
      sp_da_push_ex((void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_da_size(arr), 25);
    ASSERT_GE(sp_da_capacity(arr), 50);

    for (int i = 0; i < 25; i++) {
      ASSERT_EQ(arr[i], i);
    }

    sp_da_free(arr);
  }

}

UTEST(dyn_array, allocator_realloc_into_scratch_clobber) {
  struct {
    u8 array;
    u8 fill;
  } pattern = {
    .array = 0xFF,
    .fill = 0xAA,
  };

  sp_mem_arena_t* arena = sp_mem_arena_new();
  sp_context_push_arena(arena);
  sp_allocator_t a = sp_mem_arena_as_allocator(arena);

  sp_da(u8) arr = sp_zero();
  sp_da_init(&a, arr);

  sp_da_reserve(arr, 4);
  sp_for(it, 4) {
    sp_da_push(arr, pattern.array);
  }
  ASSERT_EQ(sp_da_size(arr), 4);

  u64 num_bytes = 0;
  u32 len = 0;

  {
    sp_mem_scratch_t s = sp_mem_begin_scratch();
    u8* ptr = arr;
    while (ptr == arr) {
      sp_da_push(arr, pattern.array);
    }

    num_bytes = sp_mem_arena_bytes_used(arena);
    len = sp_da_size(arr);

    sp_da_for(arr, it) {
      ASSERT_EQ(arr[it], pattern.array);
    }

    sp_mem_end_scratch(s);

  }

  {
    sp_mem_scratch_t s = sp_mem_begin_scratch();
    u8* clobber = sp_alloc_n(u8, num_bytes);
    sp_mem_fill_u8(clobber, num_bytes, pattern.fill);

    sp_for(it, len) {
      ASSERT_EQ(arr[it], pattern.array);
    }

    sp_mem_end_scratch(s);
  }


  sp_context_pop();
  sp_mem_arena_destroy(arena);
}
