#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_elf.h"

typedef struct {
  u32 name_offset;
  u32 data_offset;
  u32 data_size;
  u32 _pad;
} embed_header_t;

static sp_str_t make_symbol_name(sp_str_t basename) {
  return sp_str_concat(sp_str_lit("embed_"), basename);
}

s32 embed_main(s32 argc, const c8** argv) {
  if (argc < 3) {
    sp_log("usage: embed <output.o> <file1> [file2 ...]");
    return 1;
  }

  sp_str_t output_path = sp_str_view(argv[1]);
  u32 num_files = argc - 2;

  sp_log("embedding {.fg brightblack} files into {.fg cyan}", sp_fmt_uint(num_files), sp_fmt_str(output_path));

  sp_elf_t* elf = sp_elf_new_with_null_section();

  u32 data_sec_idx = sp_elf_add_section(elf, sp_str_lit(".rodata"), SHT_PROGBITS, 16)->index;
  sp_elf_find_section_by_index(elf, data_sec_idx)->flags = SHF_ALLOC;

  sp_elf_symtab_new(elf);
  sp_elf_section_t* symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));

  sp_elf_section_t* data_sec = sp_elf_find_section_by_index(elf, data_sec_idx);
  u32* count_ptr = (u32*)sp_elf_section_reserve_bytes(data_sec, sizeof(u32));
  *count_ptr = num_files;
  sp_elf_add_symbol(symtab, elf, sp_str_lit("embed_file_count"), 0, sizeof(u32), STB_GLOBAL, STT_OBJECT, (u16)data_sec_idx);

  u32 header_base = sp_elf_find_section_by_index(elf, data_sec_idx)->buffer.size;
  header_base = (header_base + 15) & ~15;

  data_sec = sp_elf_find_section_by_index(elf, data_sec_idx);
  while (data_sec->buffer.size < header_base) {
    *sp_elf_section_reserve_bytes(data_sec, 1) = 0;
    data_sec = sp_elf_find_section_by_index(elf, data_sec_idx);
  }

  u32 header_size = sizeof(embed_header_t) * num_files;
  embed_header_t* headers = sp_alloc(header_size);
  sp_elf_section_reserve_bytes(sp_elf_find_section_by_index(elf, data_sec_idx), header_size);

  symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));
  sp_elf_add_symbol(symtab, elf, sp_str_lit("embed_files"), header_base, header_size, STB_GLOBAL, STT_OBJECT, (u16)data_sec_idx);

  sp_for(i, num_files) {
    sp_str_t path = sp_str_view(argv[2 + i]);
    sp_str_t basename = sp_fs_get_name(path);
    sp_str_t content = sp_zero();
    sp_io_read_file(path, &content);

    if (sp_str_empty(content)) {
      return 1;
    }

    sp_log("{.fg brightcyan} ({} bytes)", sp_fmt_str(basename), sp_fmt_uint(content.len));

    data_sec = sp_elf_find_section_by_index(elf, data_sec_idx);
    u32 data_offset = (u32)data_sec->buffer.size;
    u8* dest = sp_elf_section_reserve_bytes(data_sec, content.len);
    sp_memcpy(dest, content.data, content.len);

    sp_str_t sym_name = make_symbol_name(basename);
    symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));
    sp_elf_add_symbol(symtab, elf, sym_name, data_offset, content.len, STB_GLOBAL, STT_OBJECT, (u16)data_sec_idx);

    headers[i].name_offset = 0;
    headers[i].data_offset = data_offset;
    headers[i].data_size = (u32)content.len;
    headers[i]._pad = 0;
  }

  data_sec = sp_elf_find_section_by_index(elf, data_sec_idx);
  sp_memcpy(data_sec->buffer.data + header_base, headers, header_size);

  sp_err_t err = sp_elf_write_to_file(elf, output_path);
  if (err != SP_OK) {
    sp_log("error: failed to write output");
    return 1;
  }

  return 0;
}

SP_MAIN(embed_main)

