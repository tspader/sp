#include "sp.h"

typedef struct {
  u32 name_offset;
  u32 data_offset;
  u32 data_size;
  u32 _pad;
} embed_header_t;

static void print(const c8* s) {
  u64 len = 0;
  while (s[len]) len++;
  sp_sys_write(2, s, len);
}

static void print_str(sp_str_t s) {
  sp_sys_write(2, s.data, s.len);
}

static void print_u32(u32 n) {
  c8 buf[16];
  s32 i = 0;
  if (n == 0) {
    buf[i++] = '0';
  } else {
    while (n > 0) {
      buf[i++] = '0' + (n % 10);
      n /= 10;
    }
  }
  while (i-- > 0) {
    sp_sys_write(2, &buf[i], 1);
  }
}

static sp_str_t read_file(sp_str_t path) {
  const c8* cpath = sp_str_to_cstr(path);

  sp_stat_t st;
  if (sp_stat(cpath, &st) < 0) {
    print("error: cannot stat ");
    print_str(path);
    print("\n");
    return SP_ZERO_STRUCT(sp_str_t);
  }

  u64 size = st.st_size;
  s32 fd = sp_open(cpath, SP_O_RDONLY, 0);
  if (fd < 0) {
    print("error: cannot open ");
    print_str(path);
    print("\n");
    return SP_ZERO_STRUCT(sp_str_t);
  }

  u8* data = sp_alloc(size);
  s64 n = sp_read(fd, data, size);
  sp_close(fd);

  if (n != (s64)size) {
    print("error: short read\n");
    return SP_ZERO_STRUCT(sp_str_t);
  }

  return (sp_str_t){ .data = (const c8*)data, .len = size };
}

static sp_str_t get_basename(sp_str_t path) {
  s64 last_slash = -1;
  sp_for(i, path.len) {
    if (path.data[i] == '/') {
      last_slash = i;
    }
  }
  if (last_slash >= 0) {
    return (sp_str_t){
      .data = path.data + last_slash + 1,
      .len = path.len - last_slash - 1
    };
  }
  return path;
}

static sp_str_t make_symbol_name(sp_str_t basename) {
  u64 len = 6 + basename.len;
  c8* buf = sp_alloc(len + 1);
  sp_memcpy(buf, "embed_", 6);
  sp_memcpy(buf + 6, basename.data, basename.len);
  buf[len] = 0;
  return (sp_str_t){ .data = buf, .len = len };
}

s32 embed_main(s32 argc, c8** argv) {
  if (argc < 3) {
    print("usage: embed <output.o> <file1> [file2 ...]\n");
    return 1;
  }

  sp_str_t output_path = sp_str_view(argv[1]);
  u32 num_files = argc - 2;

  print("embedding ");
  print_u32(num_files);
  print(" files into ");
  print_str(output_path);
  print("\n");

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
    sp_str_t basename = get_basename(path);
    sp_str_t content = read_file(path);

    if (sp_str_empty(content)) {
      return 1;
    }

    print("  ");
    print_str(basename);
    print(" (");
    print_u32((u32)content.len);
    print(" bytes)\n");

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
  if (err != SP_ERR_OK) {
    print("error: failed to write output\n");
    return 1;
  }

  print("wrote ");
  print_str(output_path);
  print("\n");

  return 0;
}

SP_ENTRY(embed_main)

