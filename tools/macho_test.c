#define SP_IMPLEMENTATION
#define SP_TEST_IMPLEMENTATION
#include "../sp.h"
#include "../test/tools/test.h"
#include "utest.h"

UTEST_MAIN()

#if defined(SP_MACHO)

struct macho_test {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(macho_test) {
  sp_test_file_manager_init(&ut.file_manager);
}

UTEST_F_TEARDOWN(macho_test) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(macho_test, create_empty) {
  sp_macho_t* m = sp_macho_new();
  ASSERT_NE(m, SP_NULLPTR);
  sp_macho_free(m);
}

UTEST_F(macho_test, add_section) {
  sp_macho_t* m = sp_macho_new();

  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);
  ASSERT_EQ(sect, 1u);

  sp_macho_section_t* s = sp_macho_get_section(m, sect);
  ASSERT_NE(s, SP_NULLPTR);
  ASSERT_EQ(s->index, 1u);

  sp_macho_free(m);
}

UTEST_F(macho_test, section_push_data) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u8 bytes[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8* ptr = sp_macho_section_push(m, sect, bytes, sizeof(bytes));

  ASSERT_NE(ptr, SP_NULLPTR);
  sp_macho_section_t* s = sp_macho_get_section(m, sect);
  ASSERT_EQ(sp_da_size(s->data), 4u);
  ASSERT_EQ(s->data[0], 0xDE);
  ASSERT_EQ(s->data[3], 0xEF);

  sp_macho_free(m);
}

UTEST_F(macho_test, add_symbol) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u32 idx = sp_macho_add_symbol(m, sp_str_lit("test_data"), sect, 0, SP_MACHO_SYM_EXTERNAL);
  ASSERT_EQ(idx, 0u);

  sp_macho_free(m);
}

UTEST_F(macho_test, write_minimal) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u8 bytes[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  sp_macho_section_push(m, sect, bytes, sizeof(bytes));
  sp_macho_add_symbol(m, sp_str_lit("test_data"), sect, 0, SP_MACHO_SYM_EXTERNAL);

  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("minimal.o"));
  sp_err_t err = sp_macho_write_to_file(m, path);
  ASSERT_EQ(err, SP_ERR_OK);
  ASSERT_TRUE(sp_fs_exists(path));

  sp_macho_free(m);
}

UTEST_F(macho_test, otool_validates) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u8 bytes[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  sp_macho_section_push(m, sect, bytes, sizeof(bytes));
  sp_macho_add_symbol(m, sp_str_lit("test_data"), sect, 0, SP_MACHO_SYM_EXTERNAL);

  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("otool_test.o"));
  sp_macho_write_to_file(m, path);
  sp_macho_free(m);

  sp_ps_output_t ps = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("otool"),
    .args = {sp_str_lit("-h"), path},
  });
  ASSERT_EQ(ps.status.exit_code, 0);
}

UTEST_F(macho_test, nm_shows_symbol) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u8 bytes[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  sp_macho_section_push(m, sect, bytes, sizeof(bytes));
  sp_macho_add_symbol(m, sp_str_lit("test_data"), sect, 0, SP_MACHO_SYM_EXTERNAL);

  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("nm_test.o"));
  sp_macho_write_to_file(m, path);
  sp_macho_free(m);

  sp_ps_output_t ps = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("nm"),
    .args = {sp_str_lit("-m"), path},
  });
  ASSERT_EQ(ps.status.exit_code, 0);
  EXPECT_TRUE(sp_str_contains(ps.out, sp_str_lit("_test_data")));
}

UTEST_F(macho_test, link_and_run) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u8 bytes[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  sp_macho_section_push(m, sect, bytes, sizeof(bytes));
  sp_macho_add_symbol(m, sp_str_lit("test_data"), sect, 0, SP_MACHO_SYM_EXTERNAL);

  sp_str_t obj_path = sp_test_file_path(&ut.file_manager, sp_str_lit("integration.o"));
  sp_macho_write_to_file(m, obj_path);
  sp_macho_free(m);

  sp_str_t c_src =
    sp_str_lit("#include <stdio.h>\n"
               "extern unsigned char test_data[8];\n"
               "int main(void) {\n"
               "  if (test_data[0] == 0xDE && test_data[1] == 0xAD &&\n"
               "      test_data[2] == 0xBE && test_data[3] == 0xEF &&\n"
               "      test_data[4] == 0xCA && test_data[5] == 0xFE &&\n"
               "      test_data[6] == 0xBA && test_data[7] == 0xBE) {\n"
               "    return 0;\n"
               "  }\n"
               "  return 1;\n"
               "}\n");

  sp_str_t c_path = sp_test_file_path(&ut.file_manager, sp_str_lit("integration.c"));
  sp_io_writer_t f = sp_io_writer_from_file(c_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&f, c_src);
  sp_io_writer_close(&f);

  sp_str_t bin_path = sp_test_file_path(&ut.file_manager, sp_str_lit("integration"));
  sp_ps_output_t compile = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("cc"),
    .args = {c_path, obj_path, sp_str_lit("-o"), bin_path},
  });

  if (compile.status.exit_code != 0) {
    SP_LOG("Compile failed:\n{}", SP_FMT_STR(compile.err));
  }
  ASSERT_EQ(compile.status.exit_code, 0);
  EXPECT_TRUE(sp_fs_exists(bin_path));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){
    .command = bin_path,
  });
  ASSERT_EQ(run.status.exit_code, 0);
}

UTEST_F(macho_test, multi_symbol_roundtrip) {
  sp_macho_t* m = sp_macho_new();
  u32 sect = sp_macho_add_section(m, sp_str_lit("__data"), 3);

  u8 data_a[] = {0x11, 0x22, 0x33, 0x44};
  u64 offset_a = sp_da_size(sp_macho_get_section(m, sect)->data);
  sp_macho_section_push(m, sect, data_a, sizeof(data_a));
  sp_macho_add_symbol(m, sp_str_lit("data_a"), sect, offset_a, SP_MACHO_SYM_EXTERNAL);

  u8 data_b[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
  u64 offset_b = sp_da_size(sp_macho_get_section(m, sect)->data);
  sp_macho_section_push(m, sect, data_b, sizeof(data_b));
  sp_macho_add_symbol(m, sp_str_lit("data_b"), sect, offset_b, SP_MACHO_SYM_EXTERNAL);

  u8 data_c[] = {0xFF};
  u64 offset_c = sp_da_size(sp_macho_get_section(m, sect)->data);
  sp_macho_section_push(m, sect, data_c, sizeof(data_c));
  sp_macho_add_symbol(m, sp_str_lit("data_c"), sect, offset_c, SP_MACHO_SYM_EXTERNAL);

  u8 data_local[] = {0x42, 0x42};
  u64 offset_local = sp_da_size(sp_macho_get_section(m, sect)->data);
  sp_macho_section_push(m, sect, data_local, sizeof(data_local));
  sp_macho_add_symbol(m, sp_str_lit("local_data"), sect, offset_local, SP_MACHO_SYM_LOCAL);

  sp_str_t obj_path = sp_test_file_path(&ut.file_manager, sp_str_lit("multi.o"));
  sp_macho_write_to_file(m, obj_path);
  sp_macho_free(m);

  sp_ps_output_t nm = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("nm"),
    .args = {sp_str_lit("-m"), obj_path},
  });
  ASSERT_EQ(nm.status.exit_code, 0);
  EXPECT_TRUE(sp_str_contains(nm.out, sp_str_lit("_data_a")));
  EXPECT_TRUE(sp_str_contains(nm.out, sp_str_lit("_data_b")));
  EXPECT_TRUE(sp_str_contains(nm.out, sp_str_lit("_data_c")));
  EXPECT_TRUE(sp_str_contains(nm.out, sp_str_lit("external")));

  sp_str_t c_src =
    sp_str_lit("#include <stdio.h>\n"
               "extern unsigned char data_a[4];\n"
               "extern unsigned char data_b[16];\n"
               "extern unsigned char data_c[1];\n"
               "int main(void) {\n"
               "  int ok = 1;\n"
               "  ok = ok && (data_a[0] == 0x11 && data_a[3] == 0x44);\n"
               "  ok = ok && (data_b[0] == 0xAA && data_b[15] == 0x99);\n"
               "  ok = ok && (data_c[0] == 0xFF);\n"
               "  return ok ? 0 : 1;\n"
               "}\n");

  sp_str_t c_path = sp_test_file_path(&ut.file_manager, sp_str_lit("multi.c"));
  sp_io_writer_t f2 = sp_io_writer_from_file(c_path, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&f2, c_src);
  sp_io_writer_close(&f2);

  sp_str_t bin_path = sp_test_file_path(&ut.file_manager, sp_str_lit("multi_bin"));
  sp_ps_output_t compile = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("cc"),
    .args = {c_path, obj_path, sp_str_lit("-o"), bin_path},
  });

  if (compile.status.exit_code != 0) {
    SP_LOG("Compile failed:\n{}", SP_FMT_STR(compile.err));
  }
  ASSERT_EQ(compile.status.exit_code, 0);

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){
    .command = bin_path,
  });
  ASSERT_EQ(run.status.exit_code, 0);
}

#endif
