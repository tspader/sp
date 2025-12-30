#include "sp.h"
#include "test.h"
#include "utest.h"

SP_TEST_MAIN()

struct elf {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(elf) {
  sp_test_file_manager_init(&ut.file_manager);
}

UTEST_F_TEARDOWN(elf) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(elf, new_creates_section0) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  ASSERT_NE(elf, SP_NULLPTR);

  ASSERT_EQ(sp_elf_num_sections(elf), 1u);

  sp_elf_section_t* section0 = sp_elf_find_section_by_index(elf, 0);
  ASSERT_NE(section0, SP_NULLPTR);
  ASSERT_EQ(section0->index, 0u);
  ASSERT_EQ(section0->type, (u32)SHT_NULL);
  ASSERT_EQ(section0->name.len, 0u);
  ASSERT_EQ(section0->alignment, 0u);
}

UTEST_F(elf, section_registered) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  ASSERT_NE(text, SP_NULLPTR);
  ASSERT_EQ(text->index, 1u);

  sp_elf_section_t* found = sp_elf_find_section_by_name(elf, sp_str_lit(".text"));
  ASSERT_NE(found, SP_NULLPTR);
  ASSERT_EQ(found->index, text->index);

  ASSERT_EQ(sp_elf_num_sections(elf), 2u);
}

UTEST_F(elf, section_find_missing) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* found = sp_elf_find_section_by_name(elf, sp_str_lit(".nonexistent"));
  ASSERT_EQ(found, SP_NULLPTR);
}

UTEST_F(elf, err_section_new_null_elf) {
  sp_elf_section_t* s = sp_elf_add_section(SP_NULLPTR, sp_str_lit(".text"), SHT_PROGBITS, 16);
  ASSERT_EQ(s, SP_NULLPTR);
}

UTEST_F(elf, err_section_new_null_name) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_str_t empty = SP_ZERO_INITIALIZE();
  sp_elf_section_t* s = sp_elf_add_section(elf, empty, SHT_PROGBITS, 16);
  ASSERT_EQ(s, SP_NULLPTR);
  ASSERT_EQ(sp_elf_num_sections(elf), 1u);
}

UTEST_F(elf, section_invalid_alignment) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* s1 = sp_elf_add_section(elf, sp_str_lit(".bad1"), SHT_PROGBITS, 3);
  ASSERT_EQ(s1, SP_NULLPTR);

  sp_elf_section_t* s2 = sp_elf_add_section(elf, sp_str_lit(".bad2"), SHT_PROGBITS, 5);
  ASSERT_EQ(s2, SP_NULLPTR);

  sp_elf_section_t* s3 = sp_elf_add_section(elf, sp_str_lit(".bad3"), SHT_PROGBITS, 6);
  ASSERT_EQ(s3, SP_NULLPTR);

  ASSERT_EQ(sp_elf_num_sections(elf), 1u);
}

UTEST_F(elf, section_valid_alignment) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* good0 = sp_elf_add_section(elf, sp_str_lit(".good0"), SHT_PROGBITS, 0);
  ASSERT_NE(good0, SP_NULLPTR);

  sp_elf_section_t* good1 = sp_elf_add_section(elf, sp_str_lit(".good1"), SHT_PROGBITS, 1);
  ASSERT_NE(good1, SP_NULLPTR);

  sp_elf_section_t* good2 = sp_elf_add_section(elf, sp_str_lit(".good2"), SHT_PROGBITS, 2);
  ASSERT_NE(good2, SP_NULLPTR);

  sp_elf_section_t* good4 = sp_elf_add_section(elf, sp_str_lit(".good4"), SHT_PROGBITS, 4);
  ASSERT_NE(good4, SP_NULLPTR);

  sp_elf_section_t* good16 = sp_elf_add_section(elf, sp_str_lit(".good16"), SHT_PROGBITS, 16);
  ASSERT_NE(good16, SP_NULLPTR);

  ASSERT_EQ(sp_elf_num_sections(elf), 6u);
}

UTEST_F(elf, strtab_null_prefix) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* strtab = sp_elf_add_section(elf, sp_str_lit(".strtab"), SHT_STRTAB, 1);

  sp_elf_add_string(strtab, sp_str_lit("hello"));

  ASSERT_GE(sp_elf_strtab_size(strtab), 1u);
  ASSERT_EQ(strtab->buffer.data[0], 0);
}

UTEST_F(elf, strtab_null_terminated) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* strtab = sp_elf_add_section(elf, sp_str_lit(".strtab"), SHT_STRTAB, 1);

  sp_elf_add_string(strtab, sp_str_lit("hello"));
  sp_elf_add_string(strtab, sp_str_lit("world"));

  ASSERT_EQ(strtab->buffer.data[0], 0);
  ASSERT_EQ(strtab->buffer.data[6], 0);
  ASSERT_EQ(strtab->buffer.data[12], 0);
}

UTEST_F(elf, strtab_index0_empty) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* strtab = sp_elf_add_section(elf, sp_str_lit(".strtab"), SHT_STRTAB, 1);

  u32 idx = sp_elf_add_string(strtab, sp_str_lit(""));
  ASSERT_EQ(idx, 0u);

  ASSERT_GE(sp_elf_strtab_size(strtab), 1u);
  ASSERT_EQ(strtab->buffer.data[0], 0);
}

UTEST_F(elf, err_add_string_null_section) {
  u32 idx = sp_elf_add_string(SP_NULLPTR, sp_str_lit("test"));
  ASSERT_EQ(idx, 0u);
}

UTEST_F(elf, symtab_entry0_null) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);

  ASSERT_EQ(sp_elf_section_num_entries(symtab), 1u);

  Elf64_Sym* sym0 = sp_elf_symtab_get(symtab, 0);
  ASSERT_EQ(sym0->st_name, 0u);
  ASSERT_EQ(sym0->st_value, 0u);
  ASSERT_EQ(sym0->st_size, 0u);
  ASSERT_EQ(sym0->st_info, 0u);
  ASSERT_EQ(sym0->st_other, 0u);
  ASSERT_EQ(sym0->st_shndx, 0u);
}

UTEST_F(elf, symtab_shlink) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);

  sp_elf_section_t* strtab = sp_elf_find_section_by_name(elf, sp_str_lit(".strtab"));
  ASSERT_NE(strtab, SP_NULLPTR);
  ASSERT_EQ(symtab->link, strtab->index);
}

UTEST_F(elf, symtab_entsize) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);

  ASSERT_EQ(symtab->entry_size, sizeof(Elf64_Sym));
}

UTEST_F(elf, symtab_add_symbol) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);

  u32 idx = sp_elf_add_symbol(symtab, elf, sp_str_lit("main"), 0x1000, 64, STB_GLOBAL, STT_FUNC, 1);
  ASSERT_EQ(idx, 1u);

  ASSERT_EQ(sp_elf_section_num_entries(symtab), 2u);

  Elf64_Sym* sym = sp_elf_symtab_get(symtab, 1);
  ASSERT_EQ(sym->st_value, 0x1000u);
  ASSERT_EQ(sym->st_size, 64u);
  ASSERT_EQ(ELF64_ST_BIND(sym->st_info), (u32)STB_GLOBAL);
  ASSERT_EQ(ELF64_ST_TYPE(sym->st_info), (u32)STT_FUNC);
  ASSERT_EQ(sym->st_shndx, 1u);
}

UTEST_F(elf, err_add_symbol_null_symtab) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  u32 idx = sp_elf_add_symbol(SP_NULLPTR, elf, sp_str_lit("test"), 0, 0, STB_LOCAL, STT_NOTYPE, 0);
  ASSERT_EQ(idx, 0u);
}

UTEST_F(elf, rela_shlink) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  sp_elf_section_t* rela = sp_elf_rela_new(elf, text);

  ASSERT_EQ(rela->link, symtab->index);
}

UTEST_F(elf, rela_shinfo) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  sp_elf_section_t* rela = sp_elf_rela_new(elf, text);

  ASSERT_EQ(rela->info, text->index);
}

UTEST_F(elf, rela_info_encoding) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  sp_elf_section_t* rela = sp_elf_rela_new(elf, text);

  sp_elf_add_relocation(rela, 0x100, 5, R_X86_64_PC32, -4);

  ASSERT_EQ(sp_elf_section_num_entries(rela), 1u);
  Elf64_Rela* rel = sp_elf_rela_get(rela, 0);
  ASSERT_EQ(rel->r_offset, 0x100u);
  ASSERT_EQ(ELF64_R_SYM(rel->r_info), 5u);
  ASSERT_EQ(ELF64_R_TYPE(rel->r_info), (u32)R_X86_64_PC32);
  ASSERT_EQ(rel->r_addend, -4);
}

UTEST_F(elf, err_add_reloc_null_section) {
  sp_elf_add_relocation(SP_NULLPTR, 0, 0, 0, 0);
  ASSERT_EQ(sp_err_get(), SP_ERR_OK);
}

UTEST_F(elf, minimal_elf_format) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_elf_write(elf, &buf);
  u8* data = buf.buffer.base;
  u64 size = buf.buffer.here - buf.buffer.base;

  ASSERT_GE(size, sizeof(Elf64_Ehdr));
  ASSERT_EQ(data[0], 0x7f);
  ASSERT_EQ(data[1], 'E');
  ASSERT_EQ(data[2], 'L');
  ASSERT_EQ(data[3], 'F');
  ASSERT_EQ(data[EI_CLASS], ELFCLASS64);
  ASSERT_EQ(data[EI_DATA], ELFDATA2LSB);
  ASSERT_EQ(data[EI_VERSION], EV_CURRENT);
  for (int i = EI_PAD; i < 16; i++) {
    ASSERT_EQ(data[i], 0);
  }

  Elf64_Ehdr* ehdr = (Elf64_Ehdr*)data;
  ASSERT_EQ(ehdr->e_version, (u32)EV_CURRENT);
  ASSERT_EQ(ehdr->e_ehsize, sizeof(Elf64_Ehdr));
  ASSERT_EQ(ehdr->e_shentsize, sizeof(Elf64_Shdr));

  Elf64_Shdr* shdrs = (Elf64_Shdr*)(data + ehdr->e_shoff);
  ASSERT_EQ(shdrs[0].sh_name, 0u);
  ASSERT_EQ(shdrs[0].sh_type, (u32)SHT_NULL);
  ASSERT_EQ(shdrs[0].sh_flags, 0u);
  ASSERT_EQ(shdrs[0].sh_addr, 0u);
  ASSERT_EQ(shdrs[0].sh_offset, 0u);
  ASSERT_EQ(shdrs[0].sh_size, 0u);
  ASSERT_EQ(shdrs[0].sh_link, 0u);
  ASSERT_EQ(shdrs[0].sh_info, 0u);
  ASSERT_EQ(shdrs[0].sh_addralign, 0u);
  ASSERT_EQ(shdrs[0].sh_entsize, 0u);

  sp_io_close(&buf);
}

UTEST_F(elf, minimal_readelf_validates) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("minimal.o"));
  sp_elf_write_to_file(elf, path);

  sp_ps_output_t ps = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("readelf"),
    .args = {sp_str_lit("-a"), path},
  });
  ASSERT_EQ(ps.status.exit_code, 0);
}

UTEST_F(elf, err_write_null_elf) {
  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_err_t err = sp_elf_write(SP_NULLPTR, &buf);
  ASSERT_NE(err, SP_ERR_OK);
  sp_io_close(&buf);
}

UTEST_F(elf, symtab_local_ordering) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);

  sp_elf_add_symbol(symtab, elf, sp_str_lit("global1"), 0x100, 0, STB_GLOBAL, STT_FUNC, 1);
  sp_elf_add_symbol(symtab, elf, sp_str_lit("local1"), 0x200, 0, STB_LOCAL, STT_OBJECT, 1);
  sp_elf_add_symbol(symtab, elf, sp_str_lit("global2"), 0x300, 0, STB_GLOBAL, STT_FUNC, 1);
  sp_elf_add_symbol(symtab, elf, sp_str_lit("local2"), 0x400, 0, STB_LOCAL, STT_OBJECT, 1);

  sp_elf_symtab_sort(symtab, elf);

  ASSERT_EQ(ELF64_ST_BIND(sp_elf_symtab_get(symtab, 0)->st_info), (u32)STB_LOCAL);
  ASSERT_EQ(ELF64_ST_BIND(sp_elf_symtab_get(symtab, 1)->st_info), (u32)STB_LOCAL);
  ASSERT_EQ(ELF64_ST_BIND(sp_elf_symtab_get(symtab, 2)->st_info), (u32)STB_LOCAL);
  ASSERT_EQ(ELF64_ST_BIND(sp_elf_symtab_get(symtab, 3)->st_info), (u32)STB_GLOBAL);
  ASSERT_EQ(ELF64_ST_BIND(sp_elf_symtab_get(symtab, 4)->st_info), (u32)STB_GLOBAL);

  ASSERT_EQ(symtab->info, 3u);
}

UTEST_F(elf, symtab_sort_updates_relocs) {
  sp_elf_t* elf = sp_elf_new_with_null_section();
  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  sp_elf_section_t* rela = sp_elf_rela_new(elf, text);

  u32 global1_idx = sp_elf_add_symbol(symtab, elf, sp_str_lit("global1"), 0x100, 0, STB_GLOBAL, STT_FUNC, 1);
  u32 local1_idx = sp_elf_add_symbol(symtab, elf, sp_str_lit("local1"), 0x200, 0, STB_LOCAL, STT_OBJECT, 1);
  u32 global2_idx = sp_elf_add_symbol(symtab, elf, sp_str_lit("global2"), 0x300, 0, STB_GLOBAL, STT_FUNC, 1);

  sp_elf_add_relocation(rela, 0x10, global1_idx, R_X86_64_PC32, -4);
  sp_elf_add_relocation(rela, 0x20, local1_idx, R_X86_64_PC32, -4);
  sp_elf_add_relocation(rela, 0x30, global2_idx, R_X86_64_PC32, -4);

  ASSERT_EQ(ELF64_R_SYM(sp_elf_rela_get(rela, 0)->r_info), global1_idx);
  ASSERT_EQ(ELF64_R_SYM(sp_elf_rela_get(rela, 1)->r_info), local1_idx);
  ASSERT_EQ(ELF64_R_SYM(sp_elf_rela_get(rela, 2)->r_info), global2_idx);

  sp_elf_symtab_sort(symtab, elf);

  Elf64_Sym* sym_at_1 = sp_elf_symtab_get(symtab, 1);
  Elf64_Sym* sym_at_2 = sp_elf_symtab_get(symtab, 2);
  Elf64_Sym* sym_at_3 = sp_elf_symtab_get(symtab, 3);
  ASSERT_EQ(sym_at_1->st_value, 0x200u);
  ASSERT_EQ(sym_at_2->st_value, 0x100u);
  ASSERT_EQ(sym_at_3->st_value, 0x300u);

  ASSERT_EQ(ELF64_R_SYM(sp_elf_rela_get(rela, 0)->r_info), 2u);
  ASSERT_EQ(ELF64_R_SYM(sp_elf_rela_get(rela, 1)->r_info), 1u);
  ASSERT_EQ(ELF64_R_SYM(sp_elf_rela_get(rela, 2)->r_info), 3u);

  ASSERT_EQ(ELF64_R_TYPE(sp_elf_rela_get(rela, 0)->r_info), (u32)R_X86_64_PC32);
  ASSERT_EQ(ELF64_R_TYPE(sp_elf_rela_get(rela, 1)->r_info), (u32)R_X86_64_PC32);
  ASSERT_EQ(ELF64_R_TYPE(sp_elf_rela_get(rela, 2)->r_info), (u32)R_X86_64_PC32);
}

UTEST_F(elf, populated_readelf_validates) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  text->flags = SHF_ALLOC | SHF_EXECINSTR;
  u8 code[] = {0xb8, 0x3c, 0x00, 0x00, 0x00, 0x31, 0xff, 0x0f, 0x05};
  u8* p = sp_elf_section_reserve_bytes(text, sizeof(code));
  sp_mem_copy(code, p, sizeof(code));

  sp_elf_section_t* data = sp_elf_add_section(elf, sp_str_lit(".data"), SHT_PROGBITS, 8);
  data->flags = SHF_ALLOC | SHF_WRITE;
  u8* d = sp_elf_section_reserve_bytes(data, 8);
  sp_mem_zero(d, 8);

  sp_elf_section_t* bss = sp_elf_add_section(elf, sp_str_lit(".bss"), SHT_NOBITS, 8);
  bss->flags = SHF_ALLOC | SHF_WRITE;
  bss->buffer.size = 16;

  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_add_symbol(symtab, elf, sp_str_lit("_start"), 0, 0, STB_GLOBAL, STT_FUNC, text->index);

  sp_str_t path = sp_test_file_path(&ut.file_manager, sp_str_lit("populated.o"));
  sp_elf_write_to_file(elf, path);

  sp_ps_output_t ps = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("readelf"),
    .args = {sp_str_lit("-a"), path},
  });
  ASSERT_EQ(ps.status.exit_code, 0);
}

UTEST_F(elf, populated_section_alignment) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  sp_elf_section_reserve_bytes(text, 10);

  sp_elf_section_t* data = sp_elf_add_section(elf, sp_str_lit(".data"), SHT_PROGBITS, 8);
  sp_elf_section_reserve_bytes(data, 5);

  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_elf_write(elf, &buf);
  u8* out_data = buf.buffer.base;

  Elf64_Ehdr* ehdr = (Elf64_Ehdr*)out_data;
  Elf64_Shdr* shdrs = (Elf64_Shdr*)(out_data + ehdr->e_shoff);

  for (u32 i = 1; i < ehdr->e_shnum; i++) {
    if (shdrs[i].sh_type == SHT_NULL || shdrs[i].sh_type == SHT_NOBITS) continue;
    u64 align = shdrs[i].sh_addralign ? shdrs[i].sh_addralign : 1;
    ASSERT_EQ(shdrs[i].sh_offset % align, 0u);
  }
  sp_io_close(&buf);
}

UTEST_F(elf, populated_sections_no_overlap) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  sp_elf_section_reserve_bytes(text, 100);

  sp_elf_section_t* data = sp_elf_add_section(elf, sp_str_lit(".data"), SHT_PROGBITS, 8);
  sp_elf_section_reserve_bytes(data, 50);

  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_elf_write(elf, &buf);
  u8* out_data = buf.buffer.base;

  Elf64_Ehdr* ehdr = (Elf64_Ehdr*)out_data;
  Elf64_Shdr* shdrs = (Elf64_Shdr*)(out_data + ehdr->e_shoff);

  for (u32 i = 1; i < ehdr->e_shnum; i++) {
    if (shdrs[i].sh_type == SHT_NULL || shdrs[i].sh_type == SHT_NOBITS || shdrs[i].sh_size == 0) continue;
    u64 start_i = shdrs[i].sh_offset;
    u64 end_i = start_i + shdrs[i].sh_size;

    for (u32 j = i + 1; j < ehdr->e_shnum; j++) {
      if (shdrs[j].sh_type == SHT_NULL || shdrs[j].sh_type == SHT_NOBITS || shdrs[j].sh_size == 0) continue;
      u64 start_j = shdrs[j].sh_offset;
      u64 end_j = start_j + shdrs[j].sh_size;

      bool overlap = (start_i < end_j) && (start_j < end_i);
      ASSERT_FALSE(overlap);
    }
  }
  sp_io_close(&buf);
}

UTEST_F(elf, populated_nobits_no_file_space) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* bss = sp_elf_add_section(elf, sp_str_lit(".bss"), SHT_NOBITS, 8);
  bss->buffer.size = 1024;

  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_elf_write(elf, &buf);
  u8* out_data = buf.buffer.base;

  Elf64_Ehdr* ehdr = (Elf64_Ehdr*)out_data;
  Elf64_Shdr* shdrs = (Elf64_Shdr*)(out_data + ehdr->e_shoff);

  Elf64_Shdr* bss_shdr = SP_NULLPTR;
  for (u32 i = 0; i < ehdr->e_shnum; i++) {
    if (shdrs[i].sh_type == SHT_NOBITS) {
      bss_shdr = &shdrs[i];
      break;
    }
  }

  ASSERT_NE(bss_shdr, SP_NULLPTR);
  ASSERT_EQ(bss_shdr->sh_size, 1024u);
  ASSERT_EQ(bss_shdr->sh_offset, 0u);
  sp_io_close(&buf);
}

UTEST_F(elf, roundtrip_minimal) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_elf_write(elf, &buf);

  sp_io_seek(&buf, 0, SP_IO_SEEK_SET);
  sp_elf_t* read_elf = sp_elf_read(&buf);

  ASSERT_NE(read_elf, SP_NULLPTR);
  ASSERT_EQ(sp_elf_num_sections(read_elf), 1);

  sp_elf_section_t* sec0 = sp_elf_find_section_by_index(read_elf, 0);
  ASSERT_EQ(sec0->type, (u32)SHT_NULL);

  sp_io_close(&buf);
}

UTEST_F(elf, roundtrip_populated) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* text = sp_elf_add_section(elf, sp_str_lit(".text"), SHT_PROGBITS, 16);
  text->flags = SHF_ALLOC | SHF_EXECINSTR;
  u8 code [] = {0xb8, 0x3c, 0x00, 0x00, 0x00, 0x31, 0xff, 0x0f, 0x05};
  u8* p = sp_elf_section_reserve_bytes(text, sizeof(code));
  sp_mem_copy(code, p, sizeof(code));

  sp_elf_section_t* data = sp_elf_add_section(elf, sp_str_lit(".data"), SHT_PROGBITS, 8);
  data->flags = SHF_ALLOC | SHF_WRITE;
  u8* d = sp_elf_section_reserve_bytes(data, 8);
  sp_mem_zero(d, 8);

  sp_elf_section_t* bss = sp_elf_add_section(elf, sp_str_lit(".bss"), SHT_NOBITS, 8);
  bss->flags = SHF_ALLOC | SHF_WRITE;
  bss->buffer.size = 16;

  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_add_symbol(symtab, elf, sp_str_lit("_start"), 0, sizeof(code), STB_GLOBAL, STT_FUNC, text->index);

  sp_io_t buf = sp_io_from_dyn_mem(SP_NULLPTR, 0);
  sp_elf_write(elf, &buf);

  sp_io_seek(&buf, 0, SP_IO_SEEK_SET);
  sp_elf_t* read_elf = sp_elf_read(&buf);

  ASSERT_NE(read_elf, SP_NULLPTR);

  sp_elf_section_t* r_text = sp_elf_find_section_by_name(read_elf, sp_str_lit(".text"));
  ASSERT_NE(r_text, SP_NULLPTR);
  ASSERT_EQ(r_text->type, SHT_PROGBITS);
  ASSERT_EQ(r_text->flags, SHF_ALLOC | SHF_EXECINSTR);
  ASSERT_EQ(r_text->buffer.size, sizeof(code));
  ASSERT_TRUE(sp_mem_is_equal(r_text->buffer.data, code, sizeof(code)));

  sp_elf_section_t* r_data = sp_elf_find_section_by_name(read_elf, sp_str_lit(".data"));
  ASSERT_NE(r_data, SP_NULLPTR);
  ASSERT_EQ(r_data->type, SHT_PROGBITS);
  ASSERT_EQ(r_data->buffer.size, 8);

  sp_elf_section_t* r_bss = sp_elf_find_section_by_name(read_elf, sp_str_lit(".bss"));
  ASSERT_NE(r_bss, SP_NULLPTR);
  ASSERT_EQ(r_bss->type, SHT_NOBITS);
  ASSERT_EQ(r_bss->buffer.size, 16);

  sp_elf_section_t* r_symtab = sp_elf_find_section_by_name(read_elf, sp_str_lit(".symtab"));
  ASSERT_NE(r_symtab, SP_NULLPTR);
  ASSERT_EQ(r_symtab->type, SHT_SYMTAB);
  ASSERT_EQ(sp_elf_section_num_entries(r_symtab), 2);

  sp_elf_section_t* r_strtab = sp_elf_find_section_by_name(read_elf, sp_str_lit(".strtab"));
  ASSERT_NE(r_strtab, SP_NULLPTR);

  sp_io_close(&buf);
}

UTEST_F(elf, err_read_invalid_magic) {
  u8 bad_data[64] = {0};
  bad_data[0] = 'B';
  bad_data[1] = 'A';
  bad_data[2] = 'D';
  bad_data[3] = '!';

  sp_io_t buf = sp_io_from_mem(bad_data, sizeof(bad_data));
  sp_elf_t* read_elf = sp_elf_read(&buf);

  ASSERT_EQ(read_elf, SP_NULLPTR);
}

UTEST_F(elf, err_read_invalid_class) {
  u8 bad_data[64] = {0};
  bad_data[0] = 0x7f;
  bad_data[1] = 'E';
  bad_data[2] = 'L';
  bad_data[3] = 'F';
  bad_data[EI_CLASS] = ELFCLASS32;
  bad_data[EI_DATA] = ELFDATA2LSB;

  sp_io_t buf = sp_io_from_mem(bad_data, sizeof(bad_data));
  sp_elf_t* read_elf = sp_elf_read(&buf);

  ASSERT_EQ(read_elf, SP_NULLPTR);
}

UTEST_F(elf, read_external_object) {
  sp_str_t exe_path = sp_fs_get_exe_path();
  sp_str_t root = exe_path;
  while (!sp_str_equal(sp_fs_get_name(root), sp_str_lit("build"))) {
    root = sp_fs_parent_path(root);
  }
  root = sp_fs_parent_path(root);

  sp_str_t fixture_path = sp_fs_join_path(root, sp_str_lit("build/debug/store/lib/minimal.o"));

  EXPECT_TRUE(sp_fs_exists(fixture_path));

  sp_elf_t* read_elf = sp_elf_read_from_file(fixture_path);
  ASSERT_NE(read_elf, SP_NULLPTR);
  ASSERT_GT(sp_elf_num_sections(read_elf), 1);

  sp_elf_section_t* text = sp_elf_find_section_by_name(read_elf, sp_str_lit(".text"));
  ASSERT_NE(text, SP_NULLPTR);
  ASSERT_EQ(text->type, (u32)SHT_PROGBITS);

  sp_elf_section_t* symtab = sp_elf_find_section_by_name(read_elf, sp_str_lit(".symtab"));
  ASSERT_NE(symtab, SP_NULLPTR);
  ASSERT_EQ(symtab->type, (u32)SHT_SYMTAB);
  ASSERT_GT(sp_elf_section_num_entries(symtab), 0u);
}

UTEST_F(elf, integration_link_and_run) {
  sp_elf_t* elf = sp_elf_new_with_null_section();

  sp_elf_section_t* data = sp_elf_add_section(elf, sp_str_lit(".data"), SHT_PROGBITS, 8);
  data->flags = SHF_ALLOC | SHF_WRITE;
  u8 test_bytes[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
  u8* p = sp_elf_section_reserve_bytes(data, sizeof(test_bytes));
  sp_mem_copy(test_bytes, p, sizeof(test_bytes));

  sp_elf_section_t* symtab = sp_elf_symtab_new(elf);
  sp_elf_add_symbol(symtab, elf, sp_str_lit("test_data"), 0, sizeof(test_bytes), STB_GLOBAL, STT_OBJECT, data->index);

  sp_str_t obj_path = sp_test_file_path(&ut.file_manager, sp_str_lit("integration.o"));
  sp_err_t err = sp_elf_write_to_file(elf, obj_path);
  ASSERT_EQ(err, SP_ERR_OK);

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
  sp_io_t f = sp_io_from_file(c_path, SP_IO_MODE_WRITE);
  sp_io_write_str(&f, c_src);
  sp_io_close(&f);

  sp_str_t bin_path = sp_test_file_path(&ut.file_manager, sp_str_lit("integration"));
  sp_ps_output_t compile = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("cc"),
    .args = {c_path, obj_path, sp_str_lit("-o"), bin_path},
  });
  ASSERT_EQ(compile.status.exit_code, 0);
  EXPECT_TRUE(sp_fs_exists(bin_path));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){
    .command = bin_path,
  });
  ASSERT_EQ(run.status.exit_code, 0);
}
