#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"


SP_TEST_MAIN()

UTEST(sp_os_is_root, various_roots) {
  ASSERT_TRUE(sp_fs_is_root(SP_LIT("")));
  ASSERT_TRUE(sp_fs_is_root(SP_LIT("/")));

#ifdef SP_WIN32
  ASSERT_TRUE(sp_fs_is_root(SP_LIT("C:")));
  ASSERT_TRUE(sp_fs_is_root(SP_LIT("C:/")));
  ASSERT_TRUE(sp_fs_is_root(SP_LIT("C:\\")));
  ASSERT_FALSE(sp_fs_is_root(SP_LIT("C:/foo")));
  ASSERT_FALSE(sp_fs_is_root(SP_LIT("C:\\foo")));
#endif

  ASSERT_FALSE(sp_fs_is_root(SP_LIT("/home")));
  ASSERT_FALSE(sp_fs_is_root(SP_LIT("/home/user")));
  ASSERT_FALSE(sp_fs_is_root(SP_LIT("relative/path")));
}


////////////////////////////
// sp_os_create_directory //
////////////////////////////
typedef struct sp_os_create_directory_fixture {
  s32 foo;
} sp_os_create_directory_fixture;

UTEST_F_SETUP(sp_os_create_directory_fixture) {
}
UTEST_F_TEARDOWN(sp_os_create_directory_fixture) {
}

UTEST_F(sp_os_create_directory_fixture, path_exists_as_directory) {
  sp_str_t dir = SP_LIT("jerry");
  ASSERT_FALSE(sp_fs_exists(dir));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(dir);
}

UTEST_F(sp_os_create_directory_fixture, path_exists_as_file) {
  sp_str_t file = SP_LIT("test_file_conflict.txt");
  ASSERT_FALSE(sp_fs_exists(file));

  sp_fs_create_file(file);
  ASSERT_TRUE(sp_fs_exists(file));
  ASSERT_TRUE(sp_fs_is_file(file));

  sp_fs_create_dir(file);
  ASSERT_TRUE(sp_fs_is_file(file));
  ASSERT_FALSE(sp_fs_is_dir(file));

  sp_fs_remove_file(file);
}

UTEST_F(sp_os_create_directory_fixture, path_doesnt_exist_no_nesting) {
  sp_str_t dir = SP_LIT("test_simple_dir");
  ASSERT_FALSE(sp_fs_exists(dir));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(dir);
}

UTEST_F(sp_os_create_directory_fixture, path_doesnt_exist_requires_nesting) {
  sp_str_t root = SP_LIT("test_nested_root");
  sp_str_t dir = SP_LIT("test_nested_root/level1/level2/level3");
  ASSERT_FALSE(sp_fs_exists(dir));

  sp_fs_create_dir(dir);

  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));
  ASSERT_TRUE(sp_fs_exists(SP_LIT("test_nested_root/level1")));
  ASSERT_TRUE(sp_fs_exists(SP_LIT("test_nested_root/level1/level2")));

  sp_fs_remove_dir(root);
}



UTEST_F(sp_os_create_directory_fixture, malformed_paths) {
  sp_str_t root = SP_LIT("test_malformed");
  ASSERT_FALSE(sp_fs_exists(root));

  struct {
    sp_str_t malformed;
    sp_str_t formed;
  } dirs [] = {
    { .malformed = SP_LIT("test_malformed//double//slash"), .formed = SP_LIT("test_malformed/double/slash") },
    { .malformed = SP_LIT("test_malformed/trailing/slash/"), .formed = SP_LIT("test_malformed/trailing/slash") },
    { .malformed = SP_LIT("test_malformed///both///kinds/"), .formed = SP_LIT("test_malformed/both/kinds") }
  };

  SP_CARR_FOR(dirs, i) {
    sp_fs_create_dir(dirs[i].malformed);
    ASSERT_TRUE(sp_fs_exists(dirs[i].formed));
  }

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, deep_nesting) {
  sp_str_t root = SP_LIT("test_deep");
  sp_str_t dir = SP_LIT("test_deep/a/b/c/d/e/f/g/h/i/j");
  ASSERT_FALSE(sp_fs_exists(dir));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, empty_path) {
  sp_fs_create_dir(SP_LIT(""));
}

UTEST_F(sp_os_create_directory_fixture, partially_existing_path) {
  sp_str_t root = SP_LIT("test_partial");
  sp_str_t partial = SP_LIT("test_partial/exists");
  sp_str_t full = SP_LIT("test_partial/exists/deep/nested");
  ASSERT_FALSE(sp_fs_exists(root));

  sp_fs_create_dir(partial);
  ASSERT_TRUE(sp_fs_exists(partial));

  sp_fs_create_dir(full);
  ASSERT_TRUE(sp_fs_exists(full));
  ASSERT_TRUE(sp_fs_is_dir(full));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, path_with_spaces) {
  sp_str_t root = SP_LIT("test_spaces");
  sp_str_t dir = SP_LIT("test_spaces/dir with spaces");
  ASSERT_FALSE(sp_fs_exists(root));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, hidden_directory) {
  sp_str_t root = SP_LIT("test_hidden");
  sp_str_t dir = SP_LIT("test_hidden/.hidden");
  ASSERT_FALSE(sp_fs_exists(root));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, unicode_characters) {
  sp_str_t root = SP_LIT("test_unicode");
  sp_str_t dir = SP_LIT("test_unicode/ñame_with_üñíçødé");
  ASSERT_FALSE(sp_fs_exists(root));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, special_symbols) {
  sp_str_t root = SP_LIT("test_symbols");
  sp_str_t dir = SP_LIT("test_symbols/dir-with_dashes.and.dots");
  ASSERT_FALSE(sp_fs_exists(root));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, trailing_slashes_variations) {
  sp_str_t root = SP_LIT("test_trailing");
  ASSERT_FALSE(sp_fs_exists(root));

  sp_fs_create_dir(SP_LIT("test_trailing/dir1/"));
  ASSERT_TRUE(sp_fs_exists(SP_LIT("test_trailing/dir1")));
  ASSERT_TRUE(sp_fs_is_dir(SP_LIT("test_trailing/dir1")));

  sp_fs_create_dir(SP_LIT("test_trailing/dir2//"));
  ASSERT_TRUE(sp_fs_exists(SP_LIT("test_trailing/dir2")));
  ASSERT_TRUE(sp_fs_is_dir(SP_LIT("test_trailing/dir2")));

  sp_fs_create_dir(SP_LIT("test_trailing/dir3///"));
  ASSERT_TRUE(sp_fs_exists(SP_LIT("test_trailing/dir3")));
  ASSERT_TRUE(sp_fs_is_dir(SP_LIT("test_trailing/dir3")));

  sp_fs_remove_dir(root);
}

UTEST_F(sp_os_create_directory_fixture, leading_slashes) {
  sp_str_t dir = SP_LIT("/tmp/sp_test_absolute");
  ASSERT_FALSE(sp_fs_exists(dir));

  sp_fs_create_dir(dir);
  ASSERT_TRUE(sp_fs_exists(dir));
  ASSERT_TRUE(sp_fs_is_dir(dir));

  sp_fs_remove_dir(dir);
}

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
  c16 c16_val;
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



sp_str_t sp_test_build_scan_directory() {
  sp_str_t directory = SP_LIT("build/test/sp_fs_collect");
  if (sp_fs_exists(directory)) {
    sp_fs_remove_dir(directory);
  }
  sp_fs_create_dir(SP_LIT("build/test"));
  sp_fs_create_dir(directory);
  return directory;
}

UTEST(sp_fs_collect, basic_scan) {

  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t file1 = sp_fs_join_path(base, SP_LIT("file1.txt"));
  sp_str_t file2 = sp_fs_join_path(base, SP_LIT("file2.log"));
  sp_str_t dir1 = sp_fs_join_path(base, SP_LIT("subdir1"));
  sp_str_t dir2 = sp_fs_join_path(base, SP_LIT("subdir2"));

  sp_fs_create_file(file1);
  sp_fs_create_file(file2);
  sp_fs_create_dir(dir1);
  sp_fs_create_dir(dir2);

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(base);

  ASSERT_EQ(sp_dyn_array_size(entries), 4);

  u32 file_count = 0;
  u32 dir_count = 0;

  sp_dyn_array_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];
    if (entry->kind == SP_FS_KIND_FILE) {
      file_count++;
    }
    if (entry->kind == SP_FS_KIND_DIR) {
      dir_count++;
    }
  }

  ASSERT_EQ(file_count, 2);
  ASSERT_EQ(dir_count, 2);

  sp_test_build_scan_directory();
}

UTEST(sp_fs_collect, file_names_validation) {

  sp_str_t base = sp_test_build_scan_directory();

  const c8* expected_names[] = {
    "alpha.txt",
    "beta.log",
    "gamma.c",
    "delta"
  };

  for (u32 i = 0; i < 4; i++) {
    sp_str_t file_path = sp_fs_join_path(base, SP_CSTR(expected_names[i]));
    sp_fs_create_file(file_path);
  }

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(base);

  ASSERT_EQ(sp_dyn_array_size(entries), 4);

  bool found[4] = {false, false, false, false};

  sp_dyn_array_for(entries, i) {
    for (u32 j = 0; j < 4; j++) {
      if (sp_str_equal_cstr(entries[i].name, expected_names[j])) {
        found[j] = true;
        break;
      }
    }
  }

  for (u32 i = 0; i < 4; i++) {
    ASSERT_TRUE(found[i]);
  }

  sp_test_build_scan_directory();
}

UTEST(sp_fs_collect, file_attributes) {

  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t test_file = sp_fs_join_path(base, SP_LIT("test.txt"));
  sp_str_t test_dir = sp_fs_join_path(base, SP_LIT("testdir"));

  sp_fs_create_file(test_file);
  sp_fs_create_dir(test_dir);

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(base);

  ASSERT_EQ(sp_dyn_array_size(entries), 2);

  bool found_file = false;
  bool found_dir = false;

  sp_dyn_array_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];

    if (sp_str_equal_cstr(entry->name, "test.txt")) {
      ASSERT_TRUE(entry->kind == SP_FS_KIND_FILE);
      ASSERT_FALSE(entry->kind == SP_FS_KIND_DIR);
      found_file = true;
    }

    if (sp_str_equal_cstr(entry->name, "testdir")) {
      ASSERT_TRUE(entry->kind == SP_FS_KIND_DIR);
      ASSERT_FALSE(entry->kind == SP_FS_KIND_FILE);
      found_dir = true;
    }
  }

  ASSERT_TRUE(found_file);
  ASSERT_TRUE(found_dir);

  sp_test_build_scan_directory();
}

UTEST(sp_fs_collect, empty_directory) {

  sp_str_t base = sp_test_build_scan_directory();

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(base);

  ASSERT_EQ(sp_dyn_array_size(entries), 0);
  ASSERT_TRUE(entries == SP_NULLPTR || sp_dyn_array_size(entries) == 0);

  sp_test_build_scan_directory();
}

UTEST(sp_fs_collect, non_existent_directory) {

  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t non_existent = sp_fs_join_path(base, SP_LIT("some_bullshit"));

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(non_existent);

  ASSERT_EQ(sp_dyn_array_size(entries), 0);
}

UTEST(sp_fs_collect, file_path_correctness) {

  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t file1 = sp_fs_join_path(base, SP_LIT("test1.txt"));
  sp_str_t dir1 = sp_fs_join_path(base, SP_LIT("subdir"));

  sp_fs_create_file(file1);
  sp_fs_create_dir(dir1);

  sp_da(sp_fs_entry_t) entries = sp_fs_collect(base);

  ASSERT_EQ(sp_dyn_array_size(entries), 2);

  sp_dyn_array_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];

    ASSERT_TRUE(sp_str_starts_with(entry->path, base));

    ASSERT_TRUE(sp_fs_exists(entry->path));

    if (sp_str_equal_cstr(entry->name, "test1.txt")) {
      ASSERT_TRUE(sp_str_ends_with(entry->path, SP_LIT("test1.txt")));
    }
    if (sp_str_equal_cstr(entry->name, "subdir")) {
      ASSERT_TRUE(sp_str_ends_with(entry->path, SP_LIT("subdir")));
    }
  }

  sp_test_build_scan_directory();
}

UTEST(sp_fs_get_kind_ex, basic_functionality) {
  sp_str_t base = sp_test_build_scan_directory();

  // test regular file - verify ONLY file flag is set
  sp_str_t file1 = sp_fs_join_path(base, SP_LIT("test_file.txt"));
  sp_fs_create_file(file1);
  sp_fs_kind_t file_attr = sp_fs_get_kind(file1);
  ASSERT_EQ(file_attr, SP_FS_KIND_FILE);

  // test directory - verify ONLY directory flag is set
  sp_str_t dir1 = sp_fs_join_path(base, SP_LIT("test_dir"));
  sp_fs_create_dir(dir1);
  sp_fs_kind_t dir_attr = sp_fs_get_kind(dir1);
  ASSERT_EQ(dir_attr, SP_FS_KIND_DIR);

  // test non-existent path
  sp_str_t non_existent = sp_fs_join_path(base, SP_LIT("does_not_exist"));
  sp_fs_kind_t none_attr = sp_fs_get_kind(non_existent);
  ASSERT_EQ(none_attr, SP_FS_KIND_NONE);

  // verify consistency with sp_fs_collect
  sp_da(sp_fs_entry_t) entries = sp_fs_collect(base);
  ASSERT_EQ(sp_dyn_array_size(entries), 2);

  sp_dyn_array_for(entries, i) {
    sp_fs_entry_t* entry = &entries[i];
    sp_fs_kind_t direct_attr = sp_fs_get_kind(entry->path);
    ASSERT_EQ(entry->kind, direct_attr);
  }

  sp_test_build_scan_directory();
}

UTEST(sp_fs_get_kind_ex, path_edge_cases) {

  sp_str_t base = sp_test_build_scan_directory();

  // empty path
  sp_fs_kind_t empty_attr = sp_fs_get_kind(SP_LIT(""));
  ASSERT_EQ(empty_attr, SP_FS_KIND_NONE);

  // null-like sp_str_t
  sp_str_t null_str = SP_ZERO_STRUCT(sp_str_t);
  null_str.data = SP_NULLPTR;
  null_str.len = 0;
  sp_fs_kind_t null_attr = sp_fs_get_kind(null_str);
  ASSERT_EQ(null_attr, SP_FS_KIND_NONE);

  // current directory
  sp_fs_kind_t dot_attr = sp_fs_get_kind(SP_LIT("."));
  ASSERT_EQ(dot_attr, SP_FS_KIND_DIR);

  // parent directory
  sp_fs_kind_t dotdot_attr = sp_fs_get_kind(SP_LIT(".."));
  ASSERT_EQ(dotdot_attr, SP_FS_KIND_DIR);

  // path with trailing slash (directory)
  sp_str_t dir_trail = sp_fs_join_path(base, SP_LIT("testdir"));
  sp_fs_create_dir(dir_trail);
  sp_str_builder_t slash_builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&slash_builder, dir_trail);
  sp_str_builder_append(&slash_builder, SP_LIT("/"));
  sp_str_t dir_with_slash = sp_str_builder_to_str(&slash_builder);
  sp_fs_kind_t trail_attr = sp_fs_get_kind(dir_with_slash);
  ASSERT_EQ(trail_attr, SP_FS_KIND_DIR);

  // path with double slashes
  sp_str_builder_t double_builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&double_builder, base);
  sp_str_builder_append(&double_builder, SP_LIT("//testdir"));
  sp_str_t double_slash = sp_str_builder_to_str(&double_builder);
  sp_fs_kind_t double_attr = sp_fs_get_kind(double_slash);
  ASSERT_EQ(double_attr, SP_FS_KIND_DIR);

  // test very long file name
  // sp_str_builder_t long_name = SP_ZERO_INITIALIZE();
  // sp_str_builder_append(&long_name, base);
  // sp_str_builder_append(&long_name, SP_LIT("/"));
  // for (u32 i = 0; i < 50; i++) {
  //   sp_str_builder_append(&long_name, SP_LIT("long"));
  // }
  // sp_str_builder_append(&long_name, SP_LIT(".txt"));
  // sp_str_t long_path = sp_str_builder_to_str(&long_name);
  // sp_fs_create_file(long_path);
  // sp_fs_kind_t long_attr = sp_fs_get_kind(long_path);
  // ASSERT_EQ(long_attr, SP_FS_KIND_FILE);

  sp_test_build_scan_directory();
}

UTEST(sp_fs_get_kind_ex, special_names_and_nesting) {

  sp_str_t base = sp_test_build_scan_directory();

  // test with spaces in names
  sp_str_t space_file = sp_fs_join_path(base, SP_LIT("file with spaces.txt"));
  sp_fs_create_file(space_file);
  sp_fs_kind_t space_attr = sp_fs_get_kind(space_file);
  ASSERT_EQ(space_attr, SP_FS_KIND_FILE);

  sp_str_t space_dir = sp_fs_join_path(base, SP_LIT("dir with spaces"));
  sp_fs_create_dir(space_dir);
  sp_fs_kind_t space_dir_attr = sp_fs_get_kind(space_dir);
  ASSERT_EQ(space_dir_attr, SP_FS_KIND_DIR);

  // test deeply nested paths
  sp_str_t level1 = sp_fs_join_path(base, SP_LIT("level1"));
  sp_fs_create_dir(level1);
  sp_str_t level2 = sp_fs_join_path(level1, SP_LIT("level2"));
  sp_fs_create_dir(level2);
  sp_str_t level3 = sp_fs_join_path(level2, SP_LIT("level3"));
  sp_fs_create_dir(level3);
  sp_str_t deep_file = sp_fs_join_path(level3, SP_LIT("deep.txt"));
  sp_fs_create_file(deep_file);

  ASSERT_EQ(sp_fs_get_kind(level1), SP_FS_KIND_DIR);
  ASSERT_EQ(sp_fs_get_kind(level2), SP_FS_KIND_DIR);
  ASSERT_EQ(sp_fs_get_kind(level3), SP_FS_KIND_DIR);
  ASSERT_EQ(sp_fs_get_kind(deep_file), SP_FS_KIND_FILE);

  // test file deleted after creation (race condition)
  sp_str_t temp_file = sp_fs_join_path(base, SP_LIT("temp.txt"));
  sp_fs_create_file(temp_file);
  sp_fs_remove_file(temp_file);
  sp_fs_kind_t deleted_attr = sp_fs_get_kind(temp_file);
  ASSERT_EQ(deleted_attr, SP_FS_KIND_NONE);

  sp_test_build_scan_directory();
}

UTEST(sp_os_join_path, empty_path_handling) {


  // Left side empty
  sp_str_t left_empty = sp_fs_join_path(SP_LIT(""), SP_LIT("file.txt"));
  SP_EXPECT_STR_EQ(left_empty, SP_LIT("file.txt"));

  sp_str_t left_empty_dir = sp_fs_join_path(SP_LIT(""), SP_LIT("dir/"));
  SP_EXPECT_STR_EQ(left_empty_dir, SP_LIT("dir"));

  // Right side empty
  sp_str_t right_empty = sp_fs_join_path(SP_LIT("path/to"), SP_LIT(""));
  SP_EXPECT_STR_EQ(right_empty, SP_LIT("path/to"));

  sp_str_t right_empty_slash = sp_fs_join_path(SP_LIT("path/to/"), SP_LIT(""));
  SP_EXPECT_STR_EQ(right_empty_slash, SP_LIT("path/to"));

  // Both sides empty
  sp_str_t both_empty = sp_fs_join_path(SP_LIT(""), SP_LIT(""));
  SP_EXPECT_STR_EQ(both_empty, SP_LIT(""));

  // Single character paths
  sp_str_t single_char = sp_fs_join_path(SP_LIT("a"), SP_LIT("b"));
  SP_EXPECT_STR_EQ(single_char, SP_LIT("a/b"));
}

UTEST(path_functions, normalized_join_and_parent) {


  // Test join path removes trailing slash
  sp_str_t joined = sp_fs_join_path(SP_LIT("path/to/dir/"), SP_LIT("file.txt"));
  SP_EXPECT_STR_EQ(joined, SP_LIT("path/to/dir/file.txt"));

  // Test parent path removes trailing slash
  sp_str_t parent = sp_fs_parent_path(SP_LIT("path/to/file.txt/"));
  SP_EXPECT_STR_EQ(parent, SP_LIT("path/to"));

  // Test joining with empty paths
  sp_str_t empty_left = sp_fs_join_path(SP_LIT(""), SP_LIT("file.txt"));
  SP_EXPECT_STR_EQ(empty_left, SP_LIT("file.txt"));

  sp_str_t empty_right = sp_fs_join_path(SP_LIT("path/to"), SP_LIT(""));
  SP_EXPECT_STR_EQ(empty_right, SP_LIT("path/to"));

  sp_str_t both_empty = sp_fs_join_path(SP_LIT(""), SP_LIT(""));
  SP_EXPECT_STR_EQ(both_empty, SP_LIT(""));

  // Verify consistency - no trailing slashes in results
  for (u32 i = 0; i < joined.len; i++) {
    if (i == joined.len - 1) {
      ASSERT_NE(joined.data[i], '/');
      ASSERT_NE(joined.data[i], '\\');
    }
  }

  for (u32 i = 0; i < parent.len; i++) {
    if (i == parent.len - 1) {
      ASSERT_NE(parent.data[i], '/');
      ASSERT_NE(parent.data[i], '\\');
    }
  }
}



typedef struct {
  bool context_valid;
  bool allocator_valid;
  bool alloc_succeeded;
  sp_str_t allocated_string;
} sp_thread_context_test_data_t;

s32 sp_thread_context_test_fn(void* userdata) {
  sp_thread_context_test_data_t* data = (sp_thread_context_test_data_t*)userdata;

  sp_context_t* ctx = sp_context_get();
  data->context_valid = (ctx != SP_NULLPTR);
  data->allocator_valid = (ctx != SP_NULLPTR && ctx->allocator.on_alloc != SP_NULLPTR);

  sp_str_t test_str = sp_str_from_cstr("thread allocation test");
  data->allocated_string = test_str;
  data->alloc_succeeded = test_str.data != SP_NULLPTR && test_str.len > 0;

  return 0;
}

UTEST(threading, context_in_child_thread) {
#if defined(SP_FREESTANDING)
  UTEST_SKIP("threads not available in freestanding");
#endif
  sp_thread_context_test_data_t data = SP_ZERO_INITIALIZE();

  sp_thread_t thread;
  sp_thread_init(&thread, sp_thread_context_test_fn, &data);
  sp_thread_join(&thread);

  ASSERT_TRUE(data.context_valid);
  ASSERT_TRUE(data.allocator_valid);
  ASSERT_TRUE(data.alloc_succeeded);
  ASSERT_GT(data.allocated_string.len, 0);
  SP_EXPECT_STR_EQ_CSTR(data.allocated_string, "thread allocation test");
}


UTEST(sp_spin_lock, basic_lock_unlock) {
  sp_spin_lock_t lock = 0;

  sp_spin_lock(&lock);
  ASSERT_EQ(lock, 1);

  sp_spin_unlock(&lock);
  ASSERT_EQ(lock, 0);
}

UTEST(sp_spin_lock, try_lock_success) {
  sp_spin_lock_t lock = 0;

  bool acquired = sp_spin_try_lock(&lock);
  ASSERT_TRUE(acquired);
  ASSERT_EQ(lock, 1);

  sp_spin_unlock(&lock);
  ASSERT_EQ(lock, 0);
}

UTEST(sp_spin_lock, try_lock_fails_when_locked) {
  sp_spin_lock_t lock = 0;

  sp_spin_lock(&lock);
  ASSERT_EQ(lock, 1);

  bool second_acquire = sp_spin_try_lock(&lock);
  ASSERT_FALSE(second_acquire);

  sp_spin_unlock(&lock);
}

UTEST(sp_spin_lock, multiple_lock_unlock_cycles) {
  sp_spin_lock_t lock = 0;

  for (s32 i = 0; i < 1000; i++) {
    sp_spin_lock(&lock);
    ASSERT_EQ(lock, 1);
    sp_spin_unlock(&lock);
    ASSERT_EQ(lock, 0);
  }
}

typedef struct {
  sp_spin_lock_t* lock;
  s32* shared_counter;
  s32 iterations;
  s32 thread_id;
} sp_spin_lock_thread_data_t;

s32 sp_spin_lock_increment_thread(void* userdata) {
  sp_spin_lock_thread_data_t* data = (sp_spin_lock_thread_data_t*)userdata;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_spin_lock(data->lock);
    (*data->shared_counter)++;
    sp_spin_unlock(data->lock);
  }

  return 0;
}

UTEST(sp_spin_lock, mutual_exclusion_two_threads) {
#if defined(SP_FREESTANDING)
  UTEST_SKIP("threads not available in freestanding");
#endif
  sp_spin_lock_t lock = 0;
  s32 shared_counter = 0;
  const s32 iterations_per_thread = 10000;

  sp_spin_lock_thread_data_t data1 = SP_ZERO_INITIALIZE();
  data1.lock = &lock;
  data1.shared_counter = &shared_counter;
  data1.iterations = iterations_per_thread;
  data1.thread_id = 1;

  sp_spin_lock_thread_data_t data2 = SP_ZERO_INITIALIZE();
  data2.lock = &lock;
  data2.shared_counter = &shared_counter;
  data2.iterations = iterations_per_thread;
  data2.thread_id = 2;

  sp_thread_t thread1, thread2;
  sp_thread_init(&thread1, sp_spin_lock_increment_thread, &data1);
  sp_thread_init(&thread2, sp_spin_lock_increment_thread, &data2);

  sp_thread_join(&thread1);
  sp_thread_join(&thread2);

  ASSERT_EQ(shared_counter, iterations_per_thread * 2);
  ASSERT_EQ(lock, 0);
}



UTEST(sp_atomic_s32, basic_operations) {
  sp_atomic_s32_t value = 0;

  s32 old = sp_atomic_s32_set(&value, 42);
  ASSERT_EQ(old, 0);
  ASSERT_EQ(sp_atomic_s32_get(&value), 42);

  old = sp_atomic_s32_add(&value, 10);
  ASSERT_EQ(old, 42);
  ASSERT_EQ(sp_atomic_s32_get(&value), 52);

  old = sp_atomic_s32_add(&value, -2);
  ASSERT_EQ(old, 52);
  ASSERT_EQ(sp_atomic_s32_get(&value), 50);
}

UTEST(sp_atomic_s32, cmp_and_swap_success) {
  sp_atomic_s32_t value = 100;

  bool result = sp_atomic_s32_cas(&value, 100, 200);
  ASSERT_TRUE(result);
  ASSERT_EQ(sp_atomic_s32_get(&value), 200);
}

UTEST(sp_atomic_s32, cmp_and_swap_fails) {
  sp_atomic_s32_t value = 100;

  bool result = sp_atomic_s32_cas(&value, 50, 200);
  ASSERT_FALSE(result);
  ASSERT_EQ(sp_atomic_s32_get(&value), 100);
}

UTEST(sp_atomic_s32, add_returns_old_value) {
  sp_atomic_s32_t value = 0;

  for (s32 i = 0; i < 100; i++) {
    s32 old = sp_atomic_s32_add(&value, 1);
    ASSERT_EQ(old, i);
  }

  ASSERT_EQ(sp_atomic_s32_get(&value), 100);
}

typedef struct {
  sp_atomic_s32_t* counter;
  s32 iterations;
} sp_atomic_s32_thread_data_t;

s32 sp_atomic_s32_add_thread(void* userdata) {
  sp_atomic_s32_thread_data_t* data = (sp_atomic_s32_thread_data_t*)userdata;

  for (s32 i = 0; i < data->iterations; i++) {
    sp_atomic_s32_add(data->counter, 1);
  }

  return 0;
}

UTEST(sp_atomic_s32, concurrent_adds) {
#if defined(SP_FREESTANDING)
  UTEST_SKIP("threads not available in freestanding");
#endif
  sp_atomic_s32_t counter = 0;
  const s32 iterations = 5000;

  sp_atomic_s32_thread_data_t data1 = {.counter = &counter, .iterations = iterations};
  sp_atomic_s32_thread_data_t data2 = {.counter = &counter, .iterations = iterations};

  sp_thread_t thread1, thread2;
  sp_thread_init(&thread1, sp_atomic_s32_add_thread, &data1);
  sp_thread_init(&thread2, sp_atomic_s32_add_thread, &data2);

  sp_thread_join(&thread1);
  sp_thread_join(&thread2);

  ASSERT_EQ(sp_atomic_s32_get(&counter), iterations * 2);
}

UTEST(sp_os, is_glob_with_wildcard) {
  ASSERT_TRUE(sp_fs_is_glob(SP_LIT("src/*.c")));
  ASSERT_TRUE(sp_fs_is_glob(SP_LIT("*")));
  ASSERT_TRUE(sp_fs_is_glob(SP_LIT("file*.txt")));
}

UTEST(sp_os, is_glob_without_wildcard) {
  ASSERT_FALSE(sp_fs_is_glob(SP_LIT("src/file.c")));
  ASSERT_FALSE(sp_fs_is_glob(SP_LIT("README.md")));
  ASSERT_FALSE(sp_fs_is_glob(SP_LIT("build/test")));
}

struct sp_os_copy_tests {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(sp_os_copy_tests) {
  sp_test_file_manager_init(&ut.file_manager);
}

UTEST_F_TEARDOWN(sp_os_copy_tests) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(sp_os_copy_tests, copy_single_file) {
  sp_str_t src = sp_test_file_path(&ut.file_manager, SP_LIT("source.txt"));
  sp_str_t dst = sp_test_file_path(&ut.file_manager, SP_LIT("dest.txt"));

  sp_str_t content = SP_LIT("test content");
  sp_test_file_create_ex((sp_test_file_config_t){.path = src, .content = content});

  sp_fs_copy_file(src, dst);

  ASSERT_TRUE(sp_fs_exists(dst));

  sp_io_reader_t reader = SP_ZERO_INITIALIZE();
  sp_io_reader_from_file(&reader, dst);
  u8 buffer[256];
  u64 bytes_read = 0;
  sp_io_read(&reader, buffer, sizeof(buffer), &bytes_read);
  sp_io_reader_close(&reader);

  sp_str_t read_content = sp_str_from_cstr_sized((c8*)buffer, (u32)bytes_read);
  SP_EXPECT_STR_EQ(read_content, content);
}

UTEST_F(sp_os_copy_tests, copy_file_to_directory) {
  sp_str_t src = sp_test_file_path(&ut.file_manager, SP_LIT("file.txt"));
  sp_str_t dst_dir = sp_test_file_path(&ut.file_manager, SP_LIT("dest"));
  sp_fs_create_dir(dst_dir);

  sp_str_t content = SP_LIT("data");
  sp_test_file_create_ex((sp_test_file_config_t){.path = src, .content = content});

  sp_fs_copy_file(src, dst_dir);

  sp_str_t expected_dst = sp_fs_join_path(dst_dir, SP_LIT("file.txt"));
  ASSERT_TRUE(sp_fs_exists(expected_dst));
}

UTEST_F(sp_os_copy_tests, copy_glob_files) {
  sp_str_t src_dir = sp_test_file_path(&ut.file_manager, SP_LIT("src"));
  sp_str_t dst_dir = sp_test_file_path(&ut.file_manager, SP_LIT("dst"));
  sp_fs_create_dir(src_dir);
  sp_fs_create_dir(dst_dir);

  sp_fs_create_file(sp_fs_join_path(src_dir, SP_LIT("a.txt")));
  sp_fs_create_file(sp_fs_join_path(src_dir, SP_LIT("b.txt")));
  sp_fs_create_file(sp_fs_join_path(src_dir, SP_LIT("c.log")));

  sp_fs_copy_glob(src_dir, SP_LIT("*"), dst_dir);

  ASSERT_TRUE(sp_fs_exists(sp_fs_join_path(dst_dir, SP_LIT("a.txt"))));
  ASSERT_TRUE(sp_fs_exists(sp_fs_join_path(dst_dir, SP_LIT("b.txt"))));
  ASSERT_TRUE(sp_fs_exists(sp_fs_join_path(dst_dir, SP_LIT("c.log"))));
}

UTEST_F(sp_os_copy_tests, copy_directory) {
  sp_str_t src_dir = sp_test_file_path(&ut.file_manager, SP_LIT("source"));
  sp_str_t dst_dir = sp_test_file_path(&ut.file_manager, SP_LIT("dest"));
  sp_fs_create_dir(src_dir);
  sp_fs_create_dir(dst_dir);

  sp_fs_create_file(sp_fs_join_path(src_dir, SP_LIT("file1.txt")));
  sp_fs_create_file(sp_fs_join_path(src_dir, SP_LIT("file2.txt")));

  sp_fs_copy_dir(src_dir, dst_dir);

  sp_str_t copied_dir = sp_fs_join_path(dst_dir, SP_LIT("source"));
  ASSERT_TRUE(sp_fs_exists(sp_fs_join_path(copied_dir, SP_LIT("file1.txt"))));
  ASSERT_TRUE(sp_fs_exists(sp_fs_join_path(copied_dir, SP_LIT("file2.txt"))));
}

UTEST(sp_os_get_cwd, smoke_test) {
  sp_str_t cwd = sp_fs_get_cwd();
  ASSERT_TRUE(sp_str_valid(cwd));
  ASSERT_TRUE(sp_fs_is_dir(cwd));
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
