#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"


SP_TEST_MAIN()

UTEST(dyn_array, basic_operations) {
    sp_dyn_array(int) arr = SP_NULLPTR;

    ASSERT_EQ(sp_dyn_array_size(arr), 0);
    ASSERT_EQ(sp_dyn_array_capacity(arr), 0);
    ASSERT_TRUE(sp_dyn_array_empty(arr));

    sp_dyn_array_push(arr, 42);
    ASSERT_EQ(sp_dyn_array_size(arr), 1);
    ASSERT_GE(sp_dyn_array_capacity(arr), 1);
    ASSERT_FALSE(sp_dyn_array_empty(arr));
    ASSERT_EQ(arr[0], 42);

    for (s32 i = 1; i < 10; i++) {
        sp_dyn_array_push(arr, i * 10);
    }
    ASSERT_EQ(sp_dyn_array_size(arr), 10);

    ASSERT_EQ(arr[0], 42);
    for (s32 i = 1; i < 10; i++) {
        ASSERT_EQ(arr[i], i * 10);
    }

    sp_dyn_array_pop(arr);
    ASSERT_EQ(sp_dyn_array_size(arr), 9);

    ASSERT_EQ(*sp_dyn_array_back(arr), 80);

    sp_dyn_array_clear(arr);
    ASSERT_EQ(sp_dyn_array_size(arr), 0);
    ASSERT_TRUE(sp_dyn_array_empty(arr));

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, reserve_capacity) {


    sp_dyn_array(float) arr = SP_NULLPTR;

    sp_dyn_array_reserve(arr, 100);
    ASSERT_GE(sp_dyn_array_capacity(arr), 100);
    ASSERT_EQ(sp_dyn_array_size(arr), 0);

    for (s32 i = 0; i < 50; i++) {
        sp_dyn_array_push(arr, (float)i * 0.5f);
    }
    ASSERT_GE(sp_dyn_array_capacity(arr), 100);
    ASSERT_EQ(sp_dyn_array_size(arr), 50);

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, growth_pattern) {


    sp_dyn_array(u32) arr = SP_NULLPTR;

    u32 prev_capacity = 0;

    for (u32 i = 0; i < 100; i++) {
        sp_dyn_array_push(arr, i);

        u32 current_capacity = sp_dyn_array_capacity(arr);
        if (current_capacity != prev_capacity) {
            if (prev_capacity > 0) {
                ASSERT_EQ(current_capacity, prev_capacity * 2);
            }
            prev_capacity = current_capacity;
        }
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 100);

    sp_dyn_array_free(arr);
}

typedef struct test_struct {
    s32 id;
    float value;
    char name[32];
} test_struct;

UTEST(dyn_array, struct_type) {


    sp_dyn_array(test_struct) arr = SP_NULLPTR;

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
        sp_dyn_array_push(arr, s);
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 10);

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

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, pointer_type) {


    sp_dyn_array(char*) arr = SP_NULLPTR;

    const char* strings[] = {"Hello", "World", "Dynamic", "Array", "Test"};

    for (s32 i = 0; i < 5; i++) {
        // c8* str = (c8*)sp_alloc(strlen(strings[i]) + 1);
        // strcpy(str, strings[i]);
        c8* str = sp_cstr_copy(strings[i]);
        sp_dyn_array_push(arr, str);
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 5);

    for (u32 i = 0; i < 5; i++) {
        ASSERT_STREQ(arr[i], strings[i]);
    }

    for (u64 i = 0; i < sp_da_size(arr); i++) {
        sp_free(arr[i]);
    }

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, edge_cases) {


    sp_dyn_array(int) arr1 = SP_NULLPTR;
    sp_dyn_array_free(arr1);
    sp_dyn_array_free(arr1);

    sp_dyn_array(int) arr2 = SP_NULLPTR;
    sp_dyn_array_pop(arr2);
    ASSERT_EQ(sp_dyn_array_size(arr2), 0);

    sp_dyn_array_clear(arr2);

    sp_dyn_array_push(arr2, 42);
    sp_dyn_array_free(arr2);

    sp_dyn_array(int) arr3 = SP_NULLPTR;
    sp_dyn_array_reserve(arr3, 0);
    ASSERT_GE(sp_dyn_array_capacity(arr3), 0);
    sp_dyn_array_free(arr3);
}

UTEST(sp_dyn_array_push_f, basic_int_push) {


    int* arr = SP_NULLPTR;

    ASSERT_EQ(sp_dyn_array_size(arr), 0);
    ASSERT_EQ(sp_dyn_array_capacity(arr), 0);

    int val1 = 42;
    sp_dyn_array_push_f((void**)&arr, &val1, sizeof(int));

    ASSERT_NE(arr, SP_NULLPTR);
    ASSERT_EQ(sp_dyn_array_size(arr), 1);
    ASSERT_GE(sp_dyn_array_capacity(arr), 1);
    ASSERT_EQ(arr[0], 42);

    int val2 = 69;
    sp_dyn_array_push_f((void**)&arr, &val2, sizeof(int));
    ASSERT_EQ(sp_dyn_array_size(arr), 2);
    ASSERT_EQ(arr[1], 69);

    int val3 = 420;
    sp_dyn_array_push_f((void**)&arr, &val3, sizeof(int));
    ASSERT_EQ(sp_dyn_array_size(arr), 3);
    ASSERT_EQ(arr[2], 420);

    ASSERT_EQ(arr[0], 42);
    ASSERT_EQ(arr[1], 69);
    ASSERT_EQ(arr[2], 420);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, different_types) {


    {
        u8* arr = SP_NULLPTR;
        for (u8 i = 0; i < 10; i++) {
            sp_dyn_array_push_f((void**)&arr, &i, sizeof(u8));
        }
        ASSERT_EQ(sp_dyn_array_size(arr), 10);
        for (u8 i = 0; i < 10; i++) {
            ASSERT_EQ(arr[i], i);
        }
        sp_dyn_array_free(arr);
    }

    {
        u16* arr = SP_NULLPTR;
        u16 vals[] = {100, 200, 300, 400, 500};
        for (int i = 0; i < 5; i++) {
            sp_dyn_array_push_f((void**)&arr, &vals[i], sizeof(u16));
        }
        ASSERT_EQ(sp_dyn_array_size(arr), 5);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(arr[i], vals[i]);
        }
        sp_dyn_array_free(arr);
    }

    {
        u64* arr = SP_NULLPTR;
        u64 val = 0xDEADBEEFCAFEBABE;
        sp_dyn_array_push_f((void**)&arr, &val, sizeof(u64));
        ASSERT_EQ(sp_dyn_array_size(arr), 1);
        ASSERT_EQ(arr[0], 0xDEADBEEFCAFEBABE);
        sp_dyn_array_free(arr);
    }

    {
        float* arr = SP_NULLPTR;
        float vals[] = {3.14f, 2.71f, 1.41f};
        for (int i = 0; i < 3; i++) {
            sp_dyn_array_push_f((void**)&arr, &vals[i], sizeof(float));
        }
        ASSERT_EQ(sp_dyn_array_size(arr), 3);
        ASSERT_NEAR(arr[0], 3.14f, 0.001f);
        ASSERT_NEAR(arr[1], 2.71f, 0.001f);
        ASSERT_NEAR(arr[2], 1.41f, 0.001f);
        sp_dyn_array_free(arr);
    }
}

UTEST(sp_dyn_array_push_f, struct_type) {


    typedef struct {
        int id;
        float value;
        u8 flags;
    } test_struct_t;

    test_struct_t* arr = SP_NULLPTR;

    test_struct_t item1 = {.id = 1, .value = 3.14f, .flags = 0xFF};
    sp_dyn_array_push_f((void**)&arr, &item1, sizeof(test_struct_t));

    test_struct_t item2 = {.id = 2, .value = 2.71f, .flags = 0x42};
    sp_dyn_array_push_f((void**)&arr, &item2, sizeof(test_struct_t));

    test_struct_t item3 = {.id = 3, .value = 1.41f, .flags = 0x69};
    sp_dyn_array_push_f((void**)&arr, &item3, sizeof(test_struct_t));

    ASSERT_EQ(sp_dyn_array_size(arr), 3);

    ASSERT_EQ(arr[0].id, 1);
    ASSERT_NEAR(arr[0].value, 3.14f, 0.001f);
    ASSERT_EQ(arr[0].flags, 0xFF);

    ASSERT_EQ(arr[1].id, 2);
    ASSERT_NEAR(arr[1].value, 2.71f, 0.001f);
    ASSERT_EQ(arr[1].flags, 0x42);

    ASSERT_EQ(arr[2].id, 3);
    ASSERT_NEAR(arr[2].value, 1.41f, 0.001f);
    ASSERT_EQ(arr[2].flags, 0x69);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, growth_behavior) {


    int* arr = SP_NULLPTR;

    for (int i = 0; i < 100; i++) {
        sp_dyn_array_push_f((void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 100);
    ASSERT_GE(sp_dyn_array_capacity(arr), 100);

    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(arr[i], i);
    }

    u64 old_capacity = sp_dyn_array_capacity(arr);
    int val = 1000;
    // Fill up to capacity - 1 (because pushing when size == capacity - 1 will trigger growth)
    while (sp_dyn_array_size(arr) < sp_dyn_array_capacity(arr) - 1) {
        sp_dyn_array_push_f((void**)&arr, &val, sizeof(int));
    }

    // Now we're at capacity - 1, next push should trigger growth
    sp_dyn_array_push_f((void**)&arr, &val, sizeof(int));
    ASSERT_GT(sp_dyn_array_capacity(arr), old_capacity);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, alignment_test) {
  typedef struct {
    u8 a;
    u64 b;
    u8 c;
  } aligned_struct_t;

  aligned_struct_t* arr = SP_NULLPTR;

  for (int i = 0; i < 10; i++) {
    aligned_struct_t item = {.a = (u8)i, .b = (u64)(i * 1000), .c = (u8)(255 - i)};
    sp_dyn_array_push_f((void**)&arr, &item, sizeof(aligned_struct_t));
  }

  ASSERT_EQ(sp_dyn_array_size(arr), 10);

  for (int i = 0; i < 10; i++) {
    ASSERT_EQ(arr[i].a, i);
    ASSERT_EQ(arr[i].b, i * 1000);
    ASSERT_EQ(arr[i].c, 255 - i);
  }

  sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, zero_initialization) {
  typedef struct {
    int values[10];
  } big_struct_t;

  big_struct_t* arr = SP_NULLPTR;
  big_struct_t zero_struct = {0};

  for (int i = 0; i < 5; i++) {
    sp_dyn_array_push_f((void**)&arr, &zero_struct, sizeof(big_struct_t));
  }

  ASSERT_EQ(sp_dyn_array_size(arr), 5);

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 10; j++) {
      ASSERT_EQ(arr[i].values[j], 0);
    }
  }

  sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, mixed_with_macros) {
  int* arr = SP_NULLPTR;

  int val1 = 10;
  sp_dyn_array_push_f((void**)&arr, &val1, sizeof(int));

  sp_dyn_array_push(arr, 20);

  int val3 = 30;
  sp_dyn_array_push_f((void**)&arr, &val3, sizeof(int));

  sp_dyn_array_push(arr, 40);

  ASSERT_EQ(sp_dyn_array_size(arr), 4);
  ASSERT_EQ(arr[0], 10);
  ASSERT_EQ(arr[1], 20);
  ASSERT_EQ(arr[2], 30);
  ASSERT_EQ(arr[3], 40);

  sp_dyn_array_free(arr);
}


UTEST(sp_dyn_array_push_f, edge_cases) {
  {
    u8* arr = SP_NULLPTR;
    u8 single_byte = 0xFF;
    sp_dyn_array_push_f((void**)&arr, &single_byte, sizeof(c8));
    ASSERT_EQ(sp_dyn_array_size(arr), 1);
    ASSERT_EQ(arr[0], 0xFFu);
    sp_dyn_array_free(arr);
  }

  {
    int* arr = SP_NULLPTR;
    sp_dyn_array_reserve(arr, 50);

    for (int i = 0; i < 25; i++) {
      sp_dyn_array_push_f((void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 25);
    ASSERT_GE(sp_dyn_array_capacity(arr), 50);

    for (int i = 0; i < 25; i++) {
      ASSERT_EQ(arr[i], i);
    }

    sp_dyn_array_free(arr);
  }

}

/////////////////////
// PARSER TESTS    //
/////////////////////

UTEST(sp_parse, unsigned_integers) {


  // sp_parse_u8
  ASSERT_EQ(sp_parse_u8(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u8(SP_LIT("255")), 255);
  ASSERT_EQ(sp_parse_u8(SP_LIT("128")), 128);
  ASSERT_EQ(sp_parse_u8(SP_LIT("42")), 42);
  // Would assert: "256", "-1", "abc", ""

  // sp_parse_u16
  ASSERT_EQ(sp_parse_u16(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u16(SP_LIT("65535")), 65535);
  ASSERT_EQ(sp_parse_u16(SP_LIT("32768")), 32768);
  ASSERT_EQ(sp_parse_u16(SP_LIT("1234")), 1234);
  // Would assert: "65536", "-1", "text"

  // sp_parse_u32
  ASSERT_EQ(sp_parse_u32(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u32(SP_LIT("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u32(SP_LIT("2147483648")), 2147483648U);
  ASSERT_EQ(sp_parse_u32(SP_LIT("123456789")), 123456789U);
  // Would assert: "4294967296", "-1", "not_a_number"

  // sp_parse_u64
  ASSERT_EQ(sp_parse_u64(SP_LIT("0")), 0ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("18446744073709551615")), 18446744073709551615ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("9223372036854775808")), 9223372036854775808ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("1234567890123")), 1234567890123ULL);
  // Would assert: "18446744073709551616", "-1", "invalid"
}

UTEST(sp_parse, signed_integers) {
  // sp_parse_s8
  ASSERT_EQ(sp_parse_s8(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s8(SP_LIT("127")), 127);
  ASSERT_EQ(sp_parse_s8(SP_LIT("-128")), -128);
  ASSERT_EQ(sp_parse_s8(SP_LIT("-42")), -42);
  ASSERT_EQ(sp_parse_s8(SP_LIT("42")), 42);
  // Would assert: "128", "-129", "text"

  // sp_parse_s16
  ASSERT_EQ(sp_parse_s16(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s16(SP_LIT("32767")), 32767);
  ASSERT_EQ(sp_parse_s16(SP_LIT("-32768")), -32768);
  ASSERT_EQ(sp_parse_s16(SP_LIT("-1234")), -1234);
  ASSERT_EQ(sp_parse_s16(SP_LIT("1234")), 1234);
  // Would assert: "32768", "-32769", "invalid"

  // sp_parse_s32
  ASSERT_EQ(sp_parse_s32(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s32(SP_LIT("2147483647")), 2147483647);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-2147483648")), INT32_MIN);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-123456789")), -123456789);
  ASSERT_EQ(sp_parse_s32(SP_LIT("123456789")), 123456789);
  // Would assert: "2147483648", "-2147483649", "not_number"

  // sp_parse_s64
  ASSERT_EQ(sp_parse_s64(SP_LIT("0")), 0LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("9223372036854775807")), 9223372036854775807LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("-9223372036854775808")), INT64_MIN);
  ASSERT_EQ(sp_parse_s64(SP_LIT("-1234567890123")), -1234567890123LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("1234567890123")), 1234567890123LL);
  // Would assert: "9223372036854775808", "-9223372036854775809", "abc"
}

UTEST(sp_parse, floating_point) {


  // sp_parse_f32
  ASSERT_NEAR(sp_parse_f32(SP_LIT("0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("0.0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("3.14159")), 3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-3.14159")), -3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("1.23e2")), 123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("1.23e-2")), 0.0123f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-1.23e2")), -123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("42")), 42.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-42")), -42.0f, 1e-5f);
  // Would assert: "nan", "inf", "text", ""

  // sp_parse_f64 - NOT IMPLEMENTED (SP_BROKEN)
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("0.0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("3.141592653589793")), 3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-3.141592653589793")), -3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("1.23e10")), 1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("1.23e-10")), 1.23e-10, 1e-20);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-1.23e10")), -1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("42.0")), 42.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-42.0")), -42.0, 1e-10);
  // Would assert: "nan", "inf", "invalid", ""
}

UTEST(sp_parse, hex_and_hash) {


  // sp_parse_hex
  ASSERT_EQ(sp_parse_hex(SP_LIT("0")), 0ULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("F")), 0xFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("f")), 0xfULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("FF")), 0xFFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("ff")), 0xffULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("deadbeef")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("123ABC")), 0x123ABCULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("FFFFFFFFFFFFFFFF")), 0xFFFFFFFFFFFFFFFFULL);
  // Would assert: "G", "xyz", "-F", "", "0x" prefix, "0123" octal notation

  // sp_parse_hash
  ASSERT_EQ(sp_parse_hash(SP_LIT("0")), 0U);
  ASSERT_EQ(sp_parse_hash(SP_LIT("FFFFFFFF")), 0xFFFFFFFFU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("12345678")), 0x12345678U);
  ASSERT_EQ(sp_parse_hash(SP_LIT("DEADBEEF")), 0xDEADBEEFU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("deadbeef")), 0xdeadbeefU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("ABCD")), 0xABCDU);
  // Would assert: "G", "12345678901", "-1", ""
}

UTEST(sp_parse, boolean) {


  // sp_parse_bool
  ASSERT_EQ(sp_parse_bool(SP_LIT("true")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("false")), false);
  ASSERT_EQ(sp_parse_bool(SP_LIT("1")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("0")), false);
  // yes/no, on/off not supported - only true/false and 1/0
  // Would assert: "maybe", "2", "TRUE", "", "yes", "no", "on", "off"
}

UTEST(sp_parse, characters) {


  // sp_parse_c8 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c8(SP_LIT("'A'")), 'A');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'z'")), 'z');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'0'")), '0');
  ASSERT_EQ(sp_parse_c8(SP_LIT("' '")), ' ');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'!'")), '!');
  // Would assert: "AB", "", "abc", "A" (no quotes)

  // sp_parse_c16 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c16(SP_LIT("'A'")), L'A');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'z'")), L'z');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'0'")), L'0');
  ASSERT_EQ(sp_parse_c16(SP_LIT("' '")), L' ');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'!'")), L'!');
  // Would assert: "AB", "", "abc", "A" (no quotes)
}

UTEST(sp_parse, extended_versions) {


  // sp_parse_u32_ex
  u32 u32_val;
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("42"), &u32_val));
  ASSERT_EQ(u32_val, 42U);
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("0"), &u32_val));
  ASSERT_EQ(u32_val, 0U);
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("4294967295"), &u32_val));
  ASSERT_EQ(u32_val, 4294967295U);
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("4294967296"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("-1"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("abc"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT(""), &u32_val));

  // sp_parse_s32_ex
  s32 s32_val;
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("42"), &s32_val));
  ASSERT_EQ(s32_val, 42);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("-42"), &s32_val));
  ASSERT_EQ(s32_val, -42);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("0"), &s32_val));
  ASSERT_EQ(s32_val, 0);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("2147483647"), &s32_val));
  ASSERT_EQ(s32_val, 2147483647);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("-2147483648"), &s32_val));
  ASSERT_EQ(s32_val, INT32_MIN);
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("2147483648"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("-2147483649"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("text"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT(""), &s32_val));

  // sp_parse_f32_ex
  f32 f32_val;
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("3.14"), &f32_val));
  ASSERT_NEAR(f32_val, 3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("-3.14"), &f32_val));
  ASSERT_NEAR(f32_val, -3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("0"), &f32_val));
  ASSERT_NEAR(f32_val, 0.0f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("1.23e2"), &f32_val));
  ASSERT_NEAR(f32_val, 123.0f, 1e-5f);
  ASSERT_FALSE(sp_parse_f32_ex(SP_LIT("abc"), &f32_val));
  ASSERT_FALSE(sp_parse_f32_ex(SP_LIT(""), &f32_val));

  // sp_parse_f64_ex - NOT IMPLEMENTED (SP_BROKEN)
  // f64 f64_val;
  // ASSERT_TRUE(sp_parse_f64_ex(SP_LIT("3.14"), &f64_val));

  // sp_parse_bool_ex
  bool bool_val;
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("true"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("false"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("1"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("0"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_FALSE(sp_parse_bool_ex(SP_LIT("maybe"), &bool_val));
  ASSERT_FALSE(sp_parse_bool_ex(SP_LIT(""), &bool_val));

  // sp_parse_hex_ex
  u64 hex_val;
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("DEADBEEF"), &hex_val));
  ASSERT_EQ(hex_val, 0xDEADBEEFULL);
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("0"), &hex_val));
  ASSERT_EQ(hex_val, 0ULL);
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("FF"), &hex_val));
  ASSERT_EQ(hex_val, 0xFFULL);
  ASSERT_FALSE(sp_parse_hex_ex(SP_LIT("XYZ"), &hex_val));
  ASSERT_FALSE(sp_parse_hex_ex(SP_LIT(""), &hex_val));

  // sp_parse_hash_ex
  sp_hash_t hash_val;
  ASSERT_TRUE(sp_parse_hash_ex(SP_LIT("DEADBEEF"), &hash_val));
  ASSERT_EQ(hash_val, 0xDEADBEEF);
  ASSERT_TRUE(sp_parse_hash_ex(SP_LIT("0"), &hash_val));
  ASSERT_EQ(hash_val, 0);
  ASSERT_FALSE(sp_parse_hash_ex(SP_LIT("GHIJKLMN"), &hash_val));
  ASSERT_FALSE(sp_parse_hash_ex(SP_LIT(""), &hash_val));

  // sp_parse_c8_ex
  c8 c8_val;
  ASSERT_TRUE(sp_parse_c8_ex(SP_LIT("'A'"), &c8_val));
  ASSERT_EQ(c8_val, 'A');
  ASSERT_TRUE(sp_parse_c8_ex(SP_LIT("' '"), &c8_val));
  ASSERT_EQ(c8_val, ' ');
  ASSERT_FALSE(sp_parse_c8_ex(SP_LIT("AB"), &c8_val));
  ASSERT_FALSE(sp_parse_c8_ex(SP_LIT(""), &c8_val));

  // sp_parse_c16_ex
  u16 c16_val;
  ASSERT_TRUE(sp_parse_c16_ex(SP_LIT("'Z'"), &c16_val));
  ASSERT_EQ(c16_val, L'Z');
  ASSERT_TRUE(sp_parse_c16_ex(SP_LIT("'!'"), &c16_val));
  ASSERT_EQ(c16_val, L'!');
  ASSERT_FALSE(sp_parse_c16_ex(SP_LIT("XY"), &c16_val));
  ASSERT_FALSE(sp_parse_c16_ex(SP_LIT(""), &c16_val));

  // Additional extended tests for completeness
  u8 u8_val;
  ASSERT_TRUE(sp_parse_u8_ex(SP_LIT("255"), &u8_val));
  ASSERT_EQ(u8_val, 255);
  ASSERT_FALSE(sp_parse_u8_ex(SP_LIT("256"), &u8_val));

  u16 u16_val;
  ASSERT_TRUE(sp_parse_u16_ex(SP_LIT("65535"), &u16_val));
  ASSERT_EQ(u16_val, 65535);
  ASSERT_FALSE(sp_parse_u16_ex(SP_LIT("65536"), &u16_val));

  u64 u64_val;
  ASSERT_TRUE(sp_parse_u64_ex(SP_LIT("18446744073709551615"), &u64_val));
  ASSERT_EQ(u64_val, 18446744073709551615ULL);
  ASSERT_FALSE(sp_parse_u64_ex(SP_LIT("not_a_number"), &u64_val));

  s8 s8_val;
  ASSERT_TRUE(sp_parse_s8_ex(SP_LIT("-128"), &s8_val));
  ASSERT_EQ(s8_val, -128);
  ASSERT_FALSE(sp_parse_s8_ex(SP_LIT("-129"), &s8_val));

  s16 s16_val;
  ASSERT_TRUE(sp_parse_s16_ex(SP_LIT("32767"), &s16_val));
  ASSERT_EQ(s16_val, 32767);
  ASSERT_FALSE(sp_parse_s16_ex(SP_LIT("32768"), &s16_val));

  s64 s64_val;
  ASSERT_TRUE(sp_parse_s64_ex(SP_LIT("9223372036854775807"), &s64_val));
  ASSERT_EQ(s64_val, 9223372036854775807LL);
  ASSERT_FALSE(sp_parse_s64_ex(SP_LIT("invalid"), &s64_val));
}

UTEST(sp_parse, edge_cases) {


  // Leading/trailing whitespace - parsers DON'T handle whitespace
  // These would all fail/assert:
  // ASSERT_EQ(sp_parse_u32(SP_LIT("  42  ")), 42U);
  // ASSERT_EQ(sp_parse_s32(SP_LIT("  -42  ")), -42);
  // ASSERT_NEAR(sp_parse_f32(SP_LIT("  3.14  ")), 3.14f, 1e-5f);

  // Leading zeros
  ASSERT_EQ(sp_parse_u32(SP_LIT("00042")), 42U);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-00042")), -42);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("003.14")), 3.14f, 1e-5f);

  // Plus sign
  ASSERT_EQ(sp_parse_s32(SP_LIT("+42")), 42);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("+3.14")), 3.14f, 1e-5f);

  // Case sensitivity for hex
  ASSERT_EQ(sp_parse_hex(SP_LIT("DeAdBeEf")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("deadbeef")), 0xdeadbeefULL);

  // Maximum values - testing overflow detection
  ASSERT_EQ(sp_parse_u8(SP_LIT("255")), 255);
  ASSERT_EQ(sp_parse_u16(SP_LIT("65535")), 65535);
  ASSERT_EQ(sp_parse_u32(SP_LIT("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u64(SP_LIT("18446744073709551615")), 18446744073709551615ULL);
  // Would assert on overflow: "256" for u8, "65536" for u16, etc.
}

// ██████╗  ██████╗ ███████╗██╗██╗  ██╗
// ██╔══██╗██╔═══██╗██╔════╝██║╚██╗██╔╝
// ██████╔╝██║   ██║███████╗██║ ╚███╔╝
// ██╔═══╝ ██║   ██║╚════██║██║ ██╔██╗
// ██║     ╚██████╔╝███████║██║██╔╝ ██╗
// ╚═╝      ╚═════╝ ╚══════╝╚═╝╚═╝  ╚═╝
#ifdef SP_POSIX
UTEST(posix, smoke) {
  sp_mutex_t mutex;
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);
  sp_mutex_lock(&mutex);
  sp_mutex_unlock(&mutex);
  sp_mutex_destroy(&mutex);

  sp_semaphore_t sem;
  sp_semaphore_init(&sem);
  sp_semaphore_signal(&sem);
  sp_semaphore_wait(&sem);
  sp_semaphore_destroy(&sem);
}
#endif


#define SP_TEST_ENUM(X) \
  X(SP_ENUM_FOO) \
  X(SP_ENUM_BAR) \
  X(SP_ENUM_BAZ) \
  X(SP_ENUM_QUX)

typedef enum {
  SP_TEST_ENUM(SP_X_ENUM_DEFINE)
} sp_test_enum_t;

sp_str_t sp_test_enum_to_str(sp_test_enum_t e) {
  switch (e) {
    SP_TEST_ENUM(SP_X_ENUM_CASE_TO_STRING)
    default: SP_UNREACHABLE_RETURN(sp_str_lit(""));
  }
}

const c8* sp_test_enum_to_cstr(sp_test_enum_t e) {
  switch (e) {
    SP_TEST_ENUM(SP_X_ENUM_CASE_TO_CSTR)
    default: SP_UNREACHABLE_RETURN("");
  }
}

UTEST(sp_enum_macros, name_generation) {
  ASSERT_STREQ(sp_test_enum_to_cstr(SP_ENUM_BAZ), "SP_ENUM_BAZ");
  SP_EXPECT_STR_EQ_CSTR(sp_test_enum_to_str(SP_ENUM_QUX), "SP_ENUM_QUX");
}



// ███████╗███╗   ██╗██╗   ██╗
// ██╔════╝████╗  ██║██║   ██║
// █████╗  ██╔██╗ ██║██║   ██║
// ██╔══╝  ██║╚██╗██║╚██╗ ██╔╝
// ███████╗██║ ╚████║ ╚████╔╝
// ╚══════╝╚═╝  ╚═══╝  ╚═══╝

typedef struct sp_env {
  u8 placeholder;
} sp_env_fixture;

UTEST_F_SETUP(sp_env) {
  (void)utest_fixture;
}

UTEST_F_TEARDOWN(sp_env) {
  (void)utest_fixture;
}

UTEST_F(sp_env, init_empty) {
  sp_env_t env;
  sp_env_init(&env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  EXPECT_FALSE(sp_env_contains(&env, SP_LIT("PATH")));
  sp_env_destroy(&env);
}

UTEST_F(sp_env, insert_and_get) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("FOO"), SP_LIT("bar"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("FOO")), "bar");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, insert_overwrites) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("KEY"), SP_LIT("first"));
  sp_env_insert(&env, SP_LIT("KEY"), SP_LIT("second"));

  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("KEY")), "second");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, get_missing_returns_empty) {
  sp_env_t env;
  sp_env_init(&env);

  sp_str_t val = sp_env_get(&env, SP_LIT("DOES_NOT_EXIST"));
  EXPECT_TRUE(sp_str_empty(val));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, contains) {
  sp_env_t env;
  sp_env_init(&env);

  EXPECT_FALSE(sp_env_contains(&env, SP_LIT("X")));
  sp_env_insert(&env, SP_LIT("X"), SP_LIT("y"));
  EXPECT_TRUE(sp_env_contains(&env, SP_LIT("X")));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, erase) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_insert(&env, SP_LIT("B"), SP_LIT("2"));
  EXPECT_EQ(sp_env_count(&env), (u32)2);

  sp_env_erase(&env, SP_LIT("A"));
  EXPECT_EQ(sp_env_count(&env), (u32)1);
  EXPECT_FALSE(sp_env_contains(&env, SP_LIT("A")));
  EXPECT_TRUE(sp_env_contains(&env, SP_LIT("B")));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, erase_nonexistent) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_erase(&env, SP_LIT("NOPE"));
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, multiple_entries) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_insert(&env, SP_LIT("B"), SP_LIT("2"));
  sp_env_insert(&env, SP_LIT("C"), SP_LIT("3"));

  EXPECT_EQ(sp_env_count(&env), (u32)3);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("A")), "1");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("B")), "2");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("C")), "3");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, copy_is_independent) {
  sp_env_t env;
  sp_env_init(&env);
  sp_env_insert(&env, SP_LIT("K"), SP_LIT("V"));

  sp_env_t copy = sp_env_copy(&env);
  EXPECT_EQ(sp_env_count(&copy), (u32)1);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&copy, SP_LIT("K")), "V");

  sp_env_insert(&copy, SP_LIT("NEW"), SP_LIT("val"));
  EXPECT_EQ(sp_env_count(&copy), (u32)2);
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
  sp_env_destroy(&copy);
}

UTEST_F(sp_env, empty_value) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("EMPTY"), SP_LIT(""));
  EXPECT_TRUE(sp_env_contains(&env, SP_LIT("EMPTY")));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("EMPTY")), "");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, value_with_equals) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("DSN"), SP_LIT("host=localhost;port=5432"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("DSN")), "host=localhost;port=5432");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, capture_has_path) {
  sp_env_t env = sp_env_capture();

  EXPECT_TRUE(sp_env_count(&env) > 0);

  // Windows PEB stores "Path", POSIX uses "PATH"
  #if defined(SP_WIN32)
    sp_str_t path_key = SP_LIT("Path");
  #else
    sp_str_t path_key = SP_LIT("PATH");
  #endif

  EXPECT_TRUE(sp_env_contains(&env, path_key));
  sp_str_t path = sp_env_get(&env, path_key);
  EXPECT_TRUE(path.len > 0);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, capture_is_snapshot) {
  sp_env_t env = sp_env_capture();
  u32 count = sp_env_count(&env);

  sp_env_insert(&env, SP_LIT("SP_TEST_ONLY_VAR"), SP_LIT("hello"));
  EXPECT_EQ(sp_env_count(&env), count + 1);

  sp_env_t env2 = sp_env_capture();
  EXPECT_FALSE(sp_env_contains(&env2, SP_LIT("SP_TEST_ONLY_VAR")));

  sp_env_destroy(&env);
  sp_env_destroy(&env2);
}

UTEST_F(sp_env, destroy_then_reinit) {
  sp_env_t env;
  sp_env_init(&env);
  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_destroy(&env);

  sp_env_init(&env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  sp_env_insert(&env, SP_LIT("B"), SP_LIT("2"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("B")), "2");
  sp_env_destroy(&env);
}

#if defined(SP_POSIX)
UTEST_F(sp_env, to_posix_envp) {
  sp_env_t env;
  sp_env_init(&env);
  sp_env_insert(&env, SP_LIT("AA"), SP_LIT("11"));
  sp_env_insert(&env, SP_LIT("BB"), SP_LIT("22"));

  c8** envp = sp_env_to_posix_envp(&env);

  u32 count = 0;
  while (envp[count] != SP_NULLPTR) count++;
  EXPECT_EQ(count, (u32)2);

  bool found_aa = false;
  bool found_bb = false;
  for (u32 i = 0; i < count; i++) {
    sp_str_t entry = sp_str_view(envp[i]);
    if (sp_str_equal(entry, SP_LIT("AA=11"))) found_aa = true;
    if (sp_str_equal(entry, SP_LIT("BB=22"))) found_bb = true;
  }
  EXPECT_TRUE(found_aa);
  EXPECT_TRUE(found_bb);

  sp_env_free_posix_envp(envp);
  sp_env_destroy(&env);
}

UTEST_F(sp_env, to_posix_envp_empty) {
  sp_env_t env;
  sp_env_init(&env);

  c8** envp = sp_env_to_posix_envp(&env);
  EXPECT_EQ(envp[0], (c8*)SP_NULLPTR);

  sp_env_free_posix_envp(envp);
  sp_env_destroy(&env);
}
#endif

UTEST(sp_os_env, get_path) {
  sp_str_t path = sp_os_env_get(SP_LIT("PATH"));
  EXPECT_TRUE(sp_str_valid(path));
  EXPECT_TRUE(path.len > 0);
}

UTEST(sp_os_env, get_missing) {
  sp_str_t val = sp_os_env_get(SP_LIT("SP_DEFINITELY_NOT_SET_12345"));
  EXPECT_TRUE(sp_str_empty(val));
}

UTEST(sp_os_env, iterate) {
  sp_os_env_it_t it = sp_os_env_it_begin();

  // Windows PEB stores "Path", POSIX uses "PATH"
  #if defined(SP_WIN32)
    sp_str_t path_key = SP_LIT("Path");
  #else
    sp_str_t path_key = SP_LIT("PATH");
  #endif

  u32 count = 0;
  bool found_path = false;
  while (sp_os_env_it_valid(&it)) {
    EXPECT_TRUE(it.key.len > 0);
    if (sp_str_equal(it.key, path_key)) {
      found_path = true;
    }
    count++;
    sp_os_env_it_next(&it);
  }

  EXPECT_TRUE(count > 0);
  EXPECT_TRUE(found_path);
}

UTEST(sp_os_env, iterate_matches_capture) {
  sp_env_t captured = sp_env_capture();

  sp_os_env_it_t it = sp_os_env_it_begin();
  u32 it_count = 0;
  while (sp_os_env_it_valid(&it)) {
    EXPECT_TRUE(sp_env_contains(&captured, it.key));
    it_count++;
    sp_os_env_it_next(&it);
  }

  EXPECT_EQ(it_count, sp_env_count(&captured));
  sp_env_destroy(&captured);
}
