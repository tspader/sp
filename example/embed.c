#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_elf.h"

typedef struct {
  sp_str_t path;
  sp_str_t symbol;
  u64 size;
} embed_entry_t;

sp_str_t symbol_from_path(sp_mem_t mem, sp_str_t path) {
  sp_str_t symbol = sp_str_copy(mem, path);
  symbol = sp_str_replace_c8(mem, symbol, '/', '_');
  symbol = sp_str_replace_c8(mem, symbol, '\\', '_');
  symbol = sp_str_replace_c8(mem, symbol, '.', '_');
  symbol = sp_str_replace_c8(mem, symbol, '-', '_');
  return symbol;
}

s32 embed_main(s32 argc, const c8** argv) {
  s32 rc = 0;
  sp_mem_t mem = sp_mem_os_new();

  if (argc < 4) {
    sp_log("usage: embed <src_dir> <output.o> <output.h> [extra_files...]");
    rc = 1;
    goto cleanup;
  }

  sp_str_t src_dir = sp_str_view(argv[1]);
  sp_str_t out_obj = sp_str_view(argv[2]);
  sp_str_t out_hdr = sp_str_view(argv[3]);
  sp_log("scanning {}", sp_fmt_str(src_dir));

  sp_da(sp_fs_entry_t) files = sp_fs_collect_recursive(mem, src_dir);
  if (sp_da_empty(files)) {
    sp_log("no files found in {}", sp_fmt_str(src_dir));
    rc = 1;
    goto cleanup;
  }

  sp_elf_t* elf = sp_elf_new_with_null_section(mem);
  sp_elf_symtab_new(elf);

  u32 rodata_idx = sp_elf_add_section(elf, sp_str_lit(".rodata"), SHT_PROGBITS, 8)->index;
  sp_elf_find_section_by_index(elf, rodata_idx)->flags = SHF_ALLOC | SHF_WRITE;

  sp_da(embed_entry_t) entries = sp_da_new(mem, embed_entry_t);

  sp_da_for(files, it) {
    sp_fs_entry_t ent = files[it];
    if (ent.kind == SP_FS_KIND_DIR) {
      continue;
    }

    u32 skip = src_dir.len + 1;
    sp_str_t rel_path = sp_str_sub(ent.path, skip, ent.path.len - skip);
    sp_str_t symbol = symbol_from_path(mem, rel_path);

    sp_str_t content = sp_zero;
    sp_io_read_file(mem, ent.path, &content);
    u64 size = content.len;

    sp_elf_section_t* symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));
    sp_elf_section_t* section = sp_elf_find_section_by_index(elf, rodata_idx);

    {
      u64 offset = section->buffer.size;
      u8* ptr = sp_elf_section_reserve_bytes(section, size);
      if (size) sp_sys_memcpy(ptr, content.data, size);

      sp_elf_add_symbol(
        symtab, elf,
        symbol,
        offset, size,
        STB_GLOBAL, STT_OBJECT,
        section->index
      );
    }

    {
      section = sp_elf_find_section_by_index(elf, rodata_idx);
      symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));

      u64 offset = section->buffer.size;
      u8* ptr = sp_elf_section_reserve_bytes(section, sizeof(u64));
      sp_sys_memcpy(ptr, &size, sizeof(u64));

      sp_elf_add_symbol(
        symtab, elf,
        sp_fmt(mem, "{}_size", sp_fmt_str(symbol)).value,
        offset, sizeof(u64),
        STB_GLOBAL, STT_OBJECT,
        section->index
      );
    }

    sp_da_push(entries, ((embed_entry_t) {
      .path = rel_path,
      .symbol = symbol,
      .size = size,
    }));
  }

  {
    sp_str_t file_path = sp_str_view(argv[4]);
    sp_str_t symbol = sp_str_lit("include_spn_h");
    sp_str_t path = sp_str_lit("include/spn.h");

    sp_str_t content = sp_zero;
    sp_io_read_file(mem, file_path, &content);
    u64 size = content.len;

    sp_elf_section_t* symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));
    sp_elf_section_t* section = sp_elf_find_section_by_index(elf, rodata_idx);

    {
      u64 offset = section->buffer.size;
      u8* ptr = sp_elf_section_reserve_bytes(section, size);
      if (size) sp_sys_memcpy(ptr, content.data, size);

      sp_elf_add_symbol(
        symtab, elf,
        symbol,
        offset, size,
        STB_GLOBAL, STT_OBJECT,
        section->index
      );
    }

    {
      section = sp_elf_find_section_by_index(elf, rodata_idx);
      symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));

      u64 offset = section->buffer.size;
      u8* ptr = sp_elf_section_reserve_bytes(section, sizeof(u64));
      sp_sys_memcpy(ptr, &size, sizeof(u64));

      sp_elf_add_symbol(
        symtab, elf,
        sp_fmt(mem, "{}_size", sp_fmt_str(symbol)).value,
        offset, sizeof(u64),
        STB_GLOBAL, STT_OBJECT,
        section->index
      );
    }

    sp_da_push(entries, ((embed_entry_t) {
      .path = path,
      .symbol = symbol,
      .size = size,
    }));
  }

  sp_err_t err = sp_elf_write_to_file(elf, out_obj);
  if (err != SP_OK) {
    sp_log("failed to write {}", sp_fmt_str(out_obj));
    rc = 1;
    goto cleanup;
  }

  sp_io_file_writer_t hdr = sp_zero;
  sp_io_file_writer_from_path(&hdr, out_hdr, SP_IO_WRITE_MODE_OVERWRITE);
  sp_da_for(entries, it) {
    embed_entry_t entry = entries[it];
    sp_fmt_io(&hdr.base, mem,
      "extern const u8 {} [{}];\n",
      sp_fmt_str(entry.symbol),
      sp_fmt_uint(entry.size)
    );
    sp_fmt_io(&hdr.base, mem,
      "extern const u64 {}_size;\n\n",
      sp_fmt_str(entry.symbol)
    );
  }
  sp_io_write_str(&hdr.base, sp_str_lit("typedef struct {\n  const char* path;\n  const void* data;\n  unsigned long long size;\n} spn_embed_entry_t;\n"), SP_NULLPTR);
  sp_fmt_io(&hdr.base, mem, "static const unsigned int spn_embed_count = {};\n", sp_fmt_uint(sp_da_size(entries)));
  sp_io_write_str(&hdr.base, sp_str_lit("static const spn_embed_entry_t spn_embed_manifest[] = {\n"), SP_NULLPTR);
  sp_da_for(entries, it) {
    embed_entry_t entry = entries[it];
    sp_io_write_cstr(&hdr.base, "  { ", SP_NULLPTR);
    sp_fmt_io(&hdr.base, mem, "\"{}\", {}, {}", sp_fmt_str(entry.path), sp_fmt_str(entry.symbol), sp_fmt_uint(entry.size));
    sp_io_write_cstr(&hdr.base, " },\n", SP_NULLPTR);
  }
  sp_io_write_str(&hdr.base, sp_str_lit("};\n"), SP_NULLPTR);
  sp_io_file_writer_close(&hdr);

  sp_log("embedded {} files -> {} + {}", sp_fmt_uint(sp_da_size(entries)), sp_fmt_str(out_obj), sp_fmt_str(out_hdr));

cleanup:
  return rc;
}

SP_MAIN(embed_main)
