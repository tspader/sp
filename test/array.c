#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"


SP_TEST_MAIN()

struct dyn_array {
  sp_mem_tracking_t tracking;
  sp_mem_t mem;
};

UTEST_F_SETUP(dyn_array) {
  sp_mem_tracking_init(&ut.tracking);
  ut.mem = sp_mem_tracking_as_allocator(&ut.tracking);
}

UTEST_F_TEARDOWN(dyn_array) {
  EXPECT_TRUE(sp_mem_tracking_ok(&ut.tracking));
  sp_mem_tracking_deinit(&ut.tracking);
}

UTEST_F(dyn_array, basic_operations) {
    sp_da(int) arr = SP_NULLPTR;

    ASSERT_EQ(sp_da_size(arr), 0);
    ASSERT_GE(sp_da_capacity(arr), 0);
    ASSERT_TRUE(sp_da_empty(arr));

    sp_da_push(sp_mem_get_scratch(), arr, 42);
    ASSERT_EQ(sp_da_size(arr), 1);
    ASSERT_GE(sp_da_capacity(arr), 1);
    ASSERT_FALSE(sp_da_empty(arr));
    ASSERT_EQ(arr[0], 42);

    for (s32 i = 1; i < 10; i++) {
        sp_da_push(sp_mem_get_scratch(), arr, i * 10);
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

    sp_da_free(sp_mem_get_scratch(), arr);
}

UTEST_F(dyn_array, reserve_capacity) {
    sp_da(float) arr = SP_NULLPTR;

    sp_da_reserve(ut.mem, arr, 100);
    ASSERT_GE(sp_da_capacity(arr), 100);
    ASSERT_EQ(sp_da_size(arr), 0);

    for (s32 i = 0; i < 50; i++) {
        sp_da_push(ut.mem, arr, (float)i * 0.5f);
    }
    ASSERT_GE(sp_da_capacity(arr), 100);
    ASSERT_EQ(sp_da_size(arr), 50);

    sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, growth_pattern) {
    sp_da(u32) arr = SP_NULLPTR;

    u64 prev_capacity = 0;

    for (u32 i = 0; i < 100; i++) {
        sp_da_push(ut.mem, arr, i);

        u64 current_capacity = sp_da_capacity(arr);
        if (current_capacity != prev_capacity) {
            if (prev_capacity > 0) {
                ASSERT_EQ(current_capacity, prev_capacity * 2);
            }
            prev_capacity = current_capacity;
        }
    }

    ASSERT_EQ(sp_da_size(arr), 100);

    sp_da_free(ut.mem, arr);
}

typedef struct test_struct {
    s32 id;
    float value;
    char name[32];
} test_struct;

UTEST_F(dyn_array, struct_type) {
    sp_da(test_struct) arr = SP_NULLPTR;

    for (s32 i = 0; i < 10; i++) {
        test_struct s = sp_zero;
        s.id = i;
        s.value = (float)i * 1.5f;
        sp_io_mem_writer_t io = sp_zero;
        sp_io_mem_writer_from_buffer(&io, s.name, sizeof(s.name));
        sp_io_write_str(&io.base, sp_fmt(sp_mem_get_scratch(), "Item_{}", sp_fmt_int(s.id)).value, SP_NULLPTR);
        sp_io_pad(&io.base, 1, SP_NULLPTR);
        sp_da_push(ut.mem, arr, s);
    }

    ASSERT_EQ(sp_da_size(arr), 10);

    for (s32 i = 0; i < 10; i++) {
        ASSERT_EQ(arr[i].id, i);
        ASSERT_EQ(arr[i].value, (float)i * 1.5f);

        char expected[32];
        sp_io_mem_writer_t io = sp_zero;
        sp_io_mem_writer_from_buffer(&io, expected, sizeof(expected));
        sp_io_write_str(&io.base, sp_fmt(sp_mem_get_scratch(), "Item_{}", sp_fmt_int(i)).value, SP_NULLPTR);
        sp_io_pad(&io.base, 1, SP_NULLPTR);

        ASSERT_STREQ(arr[i].name, expected);
    }

    sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, pointer_type) {
    sp_da(char*) arr = SP_NULLPTR;

    const char* strings[] = {"Hello", "World", "Dynamic", "Array", "Test"};

    for (s32 i = 0; i < 5; i++) {
        c8* str = sp_cstr_copy(ut.mem, strings[i]);
        sp_da_push(ut.mem, arr, str);
    }

    ASSERT_EQ(sp_da_size(arr), 5);

    for (u32 i = 0; i < 5; i++) {
        ASSERT_STREQ(arr[i], strings[i]);
    }

    for (u64 i = 0; i < sp_da_size(arr); i++) {
        sp_free(ut.mem, arr[i], sp_cstr_len(arr[i]) + 1);
    }

    sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, edge_cases) {
    sp_da(int) arr2 = SP_NULLPTR;
    sp_da_pop(arr2);
    ASSERT_EQ(sp_da_size(arr2), 0);

    sp_da_clear(arr2);

    sp_da_push(ut.mem, arr2, 42);
    sp_da_free(ut.mem, arr2);

    sp_da(int) arr3 = SP_NULLPTR;
    sp_da_reserve(ut.mem, arr3, 0);
    ASSERT_GE(sp_da_capacity(arr3), 0);
    sp_da_free(ut.mem, arr3);
}

UTEST_F(dyn_array, basic_int_push) {
    int* arr = SP_NULLPTR;

    ASSERT_EQ(sp_da_size(arr), 0);
    ASSERT_GE(sp_da_capacity(arr), 0);

    int val1 = 42;
    sp_da_push_ex(ut.mem, (void**)&arr, &val1, sizeof(int));

    ASSERT_NE(arr, SP_NULLPTR);
    ASSERT_EQ(sp_da_size(arr), 1);
    ASSERT_GE(sp_da_capacity(arr), 1);
    ASSERT_EQ(arr[0], 42);

    int val2 = 69;
    sp_da_push_ex(ut.mem, (void**)&arr, &val2, sizeof(int));
    ASSERT_EQ(sp_da_size(arr), 2);
    ASSERT_EQ(arr[1], 69);

    int val3 = 420;
    sp_da_push_ex(ut.mem, (void**)&arr, &val3, sizeof(int));
    ASSERT_EQ(sp_da_size(arr), 3);
    ASSERT_EQ(arr[2], 420);

    ASSERT_EQ(arr[0], 42);
    ASSERT_EQ(arr[1], 69);
    ASSERT_EQ(arr[2], 420);

    sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, different_types) {
    {
        u8* arr = SP_NULLPTR;
        for (u8 i = 0; i < 10; i++) {
            sp_da_push_ex(ut.mem, (void**)&arr, &i, sizeof(u8));
        }
        ASSERT_EQ(sp_da_size(arr), 10);
        for (u8 i = 0; i < 10; i++) {
            ASSERT_EQ(arr[i], i);
        }
        sp_da_free(ut.mem, arr);
    }

    {
        u16* arr = SP_NULLPTR;
        u16 vals[] = {100, 200, 300, 400, 500};
        for (int i = 0; i < 5; i++) {
            sp_da_push_ex(ut.mem, (void**)&arr, &vals[i], sizeof(u16));
        }
        ASSERT_EQ(sp_da_size(arr), 5);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(arr[i], vals[i]);
        }
        sp_da_free(ut.mem, arr);
    }

    {
        u64* arr = SP_NULLPTR;
        u64 val = 0xDEADBEEFCAFEBABE;
        sp_da_push_ex(ut.mem, (void**)&arr, &val, sizeof(u64));
        ASSERT_EQ(sp_da_size(arr), 1);
        ASSERT_EQ(arr[0], 0xDEADBEEFCAFEBABE);
        sp_da_free(ut.mem, arr);
    }

    {
        float* arr = SP_NULLPTR;
        float vals[] = {3.14f, 2.71f, 1.41f};
        for (int i = 0; i < 3; i++) {
            sp_da_push_ex(ut.mem, (void**)&arr, &vals[i], sizeof(float));
        }
        ASSERT_EQ(sp_da_size(arr), 3);
        ASSERT_NEAR(arr[0], 3.14f, 0.001f);
        ASSERT_NEAR(arr[1], 2.71f, 0.001f);
        ASSERT_NEAR(arr[2], 1.41f, 0.001f);
        sp_da_free(ut.mem, arr);
    }
}

UTEST_F(dyn_array, push_struct) {
    typedef struct {
        int id;
        float value;
        u8 flags;
    } test_struct_t;

    test_struct_t* arr = SP_NULLPTR;

    test_struct_t item1 = {.id = 1, .value = 3.14f, .flags = 0xFF};
    sp_da_push_ex(ut.mem, (void**)&arr, &item1, sizeof(test_struct_t));

    test_struct_t item2 = {.id = 2, .value = 2.71f, .flags = 0x42};
    sp_da_push_ex(ut.mem, (void**)&arr, &item2, sizeof(test_struct_t));

    test_struct_t item3 = {.id = 3, .value = 1.41f, .flags = 0x69};
    sp_da_push_ex(ut.mem, (void**)&arr, &item3, sizeof(test_struct_t));

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

    sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, growth_behavior) {
    int* arr = SP_NULLPTR;

    for (int i = 0; i < 100; i++) {
        sp_da_push_ex(ut.mem, (void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_da_size(arr), 100);
    ASSERT_GE(sp_da_capacity(arr), 100);

    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(arr[i], i);
    }

    u64 old_capacity = sp_da_capacity(arr);
    int val = 1000;
    while (sp_da_size(arr) < sp_da_capacity(arr)) {
        sp_da_push_ex(ut.mem, (void**)&arr, &val, sizeof(int));
    }

    sp_da_push_ex(ut.mem, (void**)&arr, &val, sizeof(int));
    ASSERT_GT(sp_da_capacity(arr), old_capacity);

    sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, alignment_test) {
  typedef struct {
    u8 a;
    u64 b;
    u8 c;
  } aligned_struct_t;

  aligned_struct_t* arr = SP_NULLPTR;

  for (int i = 0; i < 10; i++) {
    aligned_struct_t item = {.a = (u8)i, .b = (u64)(i * 1000), .c = (u8)(255 - i)};
    sp_da_push_ex(ut.mem, (void**)&arr, &item, sizeof(aligned_struct_t));
  }

  ASSERT_EQ(sp_da_size(arr), 10);

  for (int i = 0; i < 10; i++) {
    ASSERT_EQ(arr[i].a, i);
    ASSERT_EQ(arr[i].b, i * 1000);
    ASSERT_EQ(arr[i].c, 255 - i);
  }

  sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, zero_initialization) {
  typedef struct {
    int values[10];
  } big_struct_t;

  big_struct_t* arr = SP_NULLPTR;
  big_struct_t zero_struct = {0};

  for (int i = 0; i < 5; i++) {
    sp_da_push_ex(ut.mem, (void**)&arr, &zero_struct, sizeof(big_struct_t));
  }

  ASSERT_EQ(sp_da_size(arr), 5);

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 10; j++) {
      ASSERT_EQ(arr[i].values[j], 0);
    }
  }

  sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, mixed_with_macros) {
  int* arr = SP_NULLPTR;

  int val1 = 10;
  sp_da_push_ex(ut.mem, (void**)&arr, &val1, sizeof(int));

  sp_da_push(ut.mem, arr, 20);

  int val3 = 30;
  sp_da_push_ex(ut.mem, (void**)&arr, &val3, sizeof(int));

  sp_da_push(ut.mem, arr, 40);

  ASSERT_EQ(sp_da_size(arr), 4);
  ASSERT_EQ(arr[0], 10);
  ASSERT_EQ(arr[1], 20);
  ASSERT_EQ(arr[2], 30);
  ASSERT_EQ(arr[3], 40);

  sp_da_free(ut.mem, arr);
}


UTEST_F(dyn_array, push_edge_cases) {
  {
    u8* arr = SP_NULLPTR;
    u8 single_byte = 0xFF;
    sp_da_push_ex(ut.mem, (void**)&arr, &single_byte, sizeof(c8));
    ASSERT_EQ(sp_da_size(arr), 1);
    ASSERT_EQ(arr[0], 0xFFu);
    sp_da_free(ut.mem, arr);
  }

  {
    int* arr = SP_NULLPTR;
    sp_da_reserve(ut.mem, arr, 50);

    for (int i = 0; i < 25; i++) {
      sp_da_push_ex(ut.mem, (void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_da_size(arr), 25);
    ASSERT_GE(sp_da_capacity(arr), 50);

    for (int i = 0; i < 25; i++) {
      ASSERT_EQ(arr[i], i);
    }

    sp_da_free(ut.mem, arr);
  }

}

UTEST_F(dyn_array, allocator_realloc_into_scratch_clobber) {
  struct {
    u8 array;
    u8 fill;
  } pattern = {
    .array = 0xFF,
    .fill = 0xAA,
  };

  sp_mem_arena_t* arena = sp_mem_arena_new(sp_mem_os_new());
  sp_mem_t mem = sp_mem_arena_as_allocator(arena);

  sp_da(u8) arr = SP_NULLPTR;

  sp_da_reserve(mem, arr, 4);
  sp_for(it, 4) {
    sp_da_push(mem, arr, pattern.array);
  }
  ASSERT_EQ(sp_da_size(arr), 4);

  u64 num_bytes = 0;
  u64 len = 0;

  {
    sp_mem_arena_marker_t s = sp_mem_begin_scratch();
    u8* ptr = arr;
    while (ptr == arr) {
      sp_da_push(mem, arr, pattern.array);
    }

    num_bytes = sp_mem_arena_bytes_used(arena);
    len = sp_da_size(arr);

    sp_da_for(arr, it) {
      ASSERT_EQ(arr[it], pattern.array);
    }

    sp_mem_end_scratch(s);
  }

  {
    sp_mem_arena_marker_t s = sp_mem_begin_scratch();
    u8* clobber = sp_alloc_n(s.mem, u8, num_bytes);
    sp_mem_fill_u8(clobber, num_bytes, pattern.fill);

    sp_for(it, len) {
      ASSERT_EQ(arr[it], pattern.array);
    }

    sp_mem_end_scratch(s);
  }


  sp_mem_arena_destroy(arena);
}

UTEST_F(dyn_array, null_is_empty) {
  sp_da(u32) arr = SP_NULLPTR;

  EXPECT_EQ(sp_da_size(arr), 0u);
  EXPECT_EQ(sp_da_capacity(arr), 0u);
  EXPECT_TRUE(sp_da_empty(arr));
  sp_da_clear(arr);
  sp_da_pop(arr);
  sp_da_free(ut.mem, arr);
  sp_da_for(arr, it) {
    EXPECT_TRUE(false);
  }

  EXPECT_EQ(sp_da_grow(ut.mem, arr, 0), SP_OK);
  EXPECT_EQ(arr, (u32*)SP_NULLPTR);
}

// The value is stored before the array has anywhere to put it, so it has to survive that
UTEST_F(dyn_array, push_stores_value_from_null) {
  typedef struct { u64 a; f32 b; c8 name [48]; } wide_t;

  sp_da(u8) bytes = SP_NULLPTR;
  sp_da(u64) words = SP_NULLPTR;
  sp_da(wide_t) wides = SP_NULLPTR;
  wide_t wide = { .a = 0xDEADBEEFCAFEBABEull, .b = 2.5f, .name = "wide" };

  ASSERT_EQ(sp_da_push(ut.mem, bytes, 0xAB), SP_OK);
  ASSERT_EQ(sp_da_push(ut.mem, words, 0x0123456789ABCDEFull), SP_OK);
  ASSERT_EQ(sp_da_push(ut.mem, wides, wide), SP_OK);

  EXPECT_EQ(sp_da_size(bytes), 1u);
  EXPECT_EQ(bytes[0], 0xAB);
  EXPECT_EQ(words[0], 0x0123456789ABCDEFull);
  EXPECT_EQ(wides[0].a, wide.a);
  EXPECT_EQ(wides[0].b, wide.b);
  EXPECT_STREQ(wides[0].name, "wide");

  sp_da_free(ut.mem, bytes);
  sp_da_free(ut.mem, words);
  sp_da_free(ut.mem, wides);
}

// The pushed expression may read the array it is being pushed to, whether or not the push
// allocates or moves it
UTEST_F(dyn_array, push_value_reads_the_array) {
  sp_da(u64) arr = SP_NULLPTR;

  sp_for(it, 1000) {
    ASSERT_EQ(sp_da_push(ut.mem, arr, sp_da_size(arr) * 3), SP_OK);
  }
  sp_for(it, 1000) {
    ASSERT_EQ(arr[it], it * 3);
  }

  sp_for(it, 1000) {
    ASSERT_EQ(sp_da_push(ut.mem, arr, arr[it] + arr[sp_da_size(arr) - 1]), SP_OK);
  }
  u64 last = arr[999];
  sp_for(it, 1000) {
    last = arr[it] + last;
    ASSERT_EQ(arr[1000 + it], last);
  }

  sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, reserve_holds_exactly_what_was_asked_without_moving) {
  sp_da(u32) arr = SP_NULLPTR;

  ASSERT_EQ(sp_da_reserve(ut.mem, arr, 100), SP_OK);
  EXPECT_EQ(sp_da_capacity(arr), 100u);
  EXPECT_EQ(sp_da_size(arr), 0u);

  u32* before = arr;
  sp_for(it, 100) {
    ASSERT_EQ(sp_da_push(ut.mem, arr, it), SP_OK);
  }
  EXPECT_EQ(arr, before);
  EXPECT_TRUE(sp_da_full(arr));

  ASSERT_EQ(sp_da_reserve(ut.mem, arr, 10), SP_OK);
  EXPECT_EQ(arr, before);
  EXPECT_EQ(sp_da_capacity(arr), 100u);

  sp_da_free(ut.mem, arr);
}

// Whatever the allocator said is what the caller hears
UTEST_F(dyn_array, push_returns_the_allocators_error) {
  sp_err_t errs [] = { SP_ERR_SYS_NO_MEMORY, SP_ERR_SYS_BUSY, SP_ERR_IO_EOF };

  sp_carr_for(errs, it) {
    sp_mem_failing_t failing = { .backing = ut.mem, .err = errs[it], .at = 0 };
    sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
    sp_da(u32) arr = SP_NULLPTR;

    EXPECT_EQ(sp_da_push(mem, arr, 69), errs[it]);
    EXPECT_EQ(arr, (u32*)SP_NULLPTR);
    EXPECT_EQ(sp_da_reserve(mem, arr, 10), SP_OK);
    sp_da_free(mem, arr);
  }
}

UTEST_F(dyn_array, failed_first_push_leaves_null) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 0 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_da(u64) arr = SP_NULLPTR;

  EXPECT_EQ(sp_da_push(mem, arr, 1), SP_ERR_SYS_BUSY);
  EXPECT_EQ(arr, (u64*)SP_NULLPTR);
  EXPECT_EQ(sp_da_size(arr), 0u);

  EXPECT_EQ(sp_da_push(mem, arr, 2), SP_OK);
  EXPECT_EQ(sp_da_size(arr), 1u);
  EXPECT_EQ(arr[0], 2u);

  sp_da_free(mem, arr);
}

static u32 da_eval_counting(u32* count) {
  (*count)++;
  return 7;
}

static u32 da_eval_seeing(void* arr, void** seen) {
  *seen = arr;
  return 7;
}

UTEST_F(dyn_array, failed_first_push_does_not_evaluate_the_value) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 0 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_da(u32) arr = SP_NULLPTR;
  u32 count = 0;

  EXPECT_EQ(sp_da_push(mem, arr, da_eval_counting(&count)), SP_ERR_SYS_BUSY);
  EXPECT_EQ(count, 0u);
  EXPECT_EQ(arr, (u32*)SP_NULLPTR);

  ASSERT_EQ(sp_da_push(mem, arr, da_eval_counting(&count)), SP_OK);
  EXPECT_EQ(count, 1u);
  EXPECT_EQ(arr[0], 7u);

  sp_da_free(mem, arr);
}

UTEST_F(dyn_array, first_push_evaluates_against_the_real_array) {
  sp_da(u32) arr = SP_NULLPTR;
  void* seen = SP_NULLPTR;

  ASSERT_EQ(sp_da_push(ut.mem, arr, da_eval_seeing(arr, &seen)), SP_OK);
  EXPECT_TRUE(seen != SP_NULLPTR);
  EXPECT_TRUE(seen == (void*)arr);
  EXPECT_EQ(sp_da_size(arr), 1u);
  EXPECT_EQ(arr[0], 7u);

  sp_da_free(ut.mem, arr);
}

UTEST_F(dyn_array, failed_growth_leaves_the_array_alone) {
  // Allocation 0 creates the array; 1 and 2 grow it; 3 fails
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 3 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_da(u32) arr = SP_NULLPTR;

  sp_err_t err = SP_OK;
  u32 pushed = 0;
  while (!err) {
    err = sp_da_push(mem, arr, pushed * 7);
    if (!err) pushed++;
  }

  EXPECT_EQ(err, SP_ERR_SYS_BUSY);
  EXPECT_EQ(sp_da_size(arr), pushed);
  EXPECT_EQ(sp_da_capacity(arr), pushed);
  sp_for(it, pushed) {
    EXPECT_EQ(arr[it], it * 7);
  }

  u32* before = arr;
  EXPECT_EQ(sp_da_reserve(ut.mem, arr, pushed), SP_OK);
  EXPECT_EQ(arr, before);

  // Nothing was lost; the same push works once there is memory
  ASSERT_EQ(sp_da_push(mem, arr, pushed * 7), SP_OK);
  EXPECT_EQ(sp_da_size(arr), pushed + 1);
  sp_for(it, pushed + 1) {
    EXPECT_EQ(arr[it], it * 7);
  }

  sp_da_free(mem, arr);
}

UTEST_F(dyn_array, failed_reserve_and_push_ex) {
  sp_mem_failing_t failing = { .backing = ut.mem, .err = SP_ERR_SYS_BUSY, .at = 1 };
  sp_mem_t mem = sp_mem_failing_as_allocator(&failing);
  sp_da(u32) arr = SP_NULLPTR;

  ASSERT_EQ(sp_da_reserve(mem, arr, 4), SP_OK);
  sp_for(it, 4) {
    ASSERT_EQ(sp_da_push_ex(mem, sp_da_vp(arr), &it, sizeof(u32)), SP_OK);
  }

  u32 value = 4;
  EXPECT_EQ(sp_da_reserve(mem, arr, 64), SP_ERR_SYS_BUSY);
  EXPECT_EQ(sp_da_capacity(arr), 4u);
  EXPECT_EQ(sp_da_size(arr), 4u);

  failing.at = failing.count;
  EXPECT_EQ(sp_da_push_ex(mem, sp_da_vp(arr), &value, sizeof(u32)), SP_ERR_SYS_BUSY);
  EXPECT_EQ(sp_da_size(arr), 4u);
  sp_for(it, 4) {
    EXPECT_EQ(arr[it], it);
  }

  sp_da_free(mem, arr);
}

// A fixed allocator sized for exactly N elements is a fixed capacity array
UTEST_F(dyn_array, fixed_capacity) {
  SP_ALIGNED u8 buffer [sp_da_alloc_size(8, sizeof(u32))];
  sp_mem_fixed_t fixed = sp_mem_fixed(buffer, sizeof(buffer));
  sp_mem_t mem = sp_mem_fixed_as_allocator(&fixed);
  sp_da(u32) arr = SP_NULLPTR;

  ASSERT_EQ(sp_da_reserve(mem, arr, 8), SP_OK);
  sp_for(it, 8) {
    ASSERT_EQ(sp_da_push(mem, arr, it), SP_OK);
  }
  EXPECT_EQ(sp_da_push(mem, arr, 8), SP_ERR_SYS_NO_MEMORY);
  EXPECT_EQ(sp_da_size(arr), 8u);
  sp_for(it, 8) {
    EXPECT_EQ(arr[it], it);
  }
}
