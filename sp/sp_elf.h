#ifndef SP_ELF_H
#define SP_ELF_H

#include "sp.h"

#if !defined(SP_ELF_NO_VENDOR) && !defined(_ELF_H)
typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;
typedef u16 Elf64_Half;
typedef u32 Elf64_Word;
typedef s32 Elf64_Sword;
typedef u64 Elf64_Xword;
typedef s64 Elf64_Sxword;

#define EI_NIDENT 16

typedef struct {
  u8 e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  Elf64_Word sh_name;
  Elf64_Word sh_type;
  Elf64_Xword sh_flags;
  Elf64_Addr sh_addr;
  Elf64_Off sh_offset;
  Elf64_Xword sh_size;
  Elf64_Word sh_link;
  Elf64_Word sh_info;
  Elf64_Xword sh_addralign;
  Elf64_Xword sh_entsize;
} Elf64_Shdr;

typedef struct {
  Elf64_Word st_name;
  u8 st_info;
  u8 st_other;
  Elf64_Half st_shndx;
  Elf64_Addr st_value;
  Elf64_Xword st_size;
} Elf64_Sym;

typedef struct {
  Elf64_Addr r_offset;
  Elf64_Xword r_info;
  Elf64_Sxword r_addend;
} Elf64_Rela;

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_PAD 8

#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATA2LSB 1

#define EV_CURRENT 1

#define ET_REL 1

#define EM_X86_64 62

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_NOBITS 8

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4

#define SHN_UNDEF 0
#define SHN_ABS 0xfff1

#define STB_LOCAL 0
#define STB_GLOBAL 1

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2

#define R_X86_64_PC32 2

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFOSABI_NONE 0

#define ELF64_ST_BIND(i) ((i) >> 4)
#define ELF64_ST_TYPE(i) ((i) & 0xf)
#define ELF64_ST_INFO(b, t) (((b) << 4) | ((t) & 0xf))

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)
#define ELF64_R_INFO(s, t) (((Elf64_Xword)(s) << 32) | (t))
#endif // !SP_ELF_NO_VENDOR && _ELF_H

#define SP_ELF_DEFAULT_BUFFER_SIZE 64
#define SP_ELF_NULL_INDEX 0

typedef struct {
  u8* data;
  u64 size;
  u64 capacity;
} sp_elf_buffer_t;

typedef struct {
  sp_str_t name;
  u32 index;
  u32 type;
  u64 flags;
  u64 addr;
  u64 alignment;
  u64 entry_size;
  u32 link;
  u32 info;
  sp_elf_buffer_t buffer;
} sp_elf_section_t;

typedef struct {
  sp_da(sp_elf_section_t) sections;
  sp_str_ht(u32) section_map;
  sp_mem_t allocator;
} sp_elf_t;

sp_elf_t* sp_elf_new();
sp_elf_t* sp_elf_new_alloc(sp_mem_t a);
sp_elf_t* sp_elf_new_with_null_section();
void sp_elf_free(sp_elf_t* elf);
sp_elf_section_t* sp_elf_add_section(sp_elf_t* elf, sp_str_t name, u32 type, u64 addralign);
sp_elf_section_t* sp_elf_find_section_by_index(sp_elf_t* elf, u32 index);
sp_elf_section_t* sp_elf_find_section_by_name(sp_elf_t* elf, sp_str_t name);
u32 sp_elf_num_sections(sp_elf_t* elf);
u32 sp_elf_section_num_entries(sp_elf_section_t* section);
void* sp_elf_section_reserve_aligned(sp_elf_section_t* section, u64 size, u64 align);
void* sp_elf_section_reserve_entry(sp_elf_section_t* section);
u8* sp_elf_section_reserve_bytes(sp_elf_section_t* section, u64 size);
sp_elf_section_t* sp_elf_symtab_new(sp_elf_t* elf);
u32 sp_elf_add_string(sp_elf_section_t* strtab, sp_str_t str);
u32 sp_elf_add_symbol(sp_elf_section_t* symbols, sp_elf_t* elf, sp_str_t name, u64 value, u64 size, u8 bind, u8 type, u16 shndx);
Elf64_Sym* sp_elf_symtab_get(sp_elf_section_t* symtab, u32 index);
void sp_elf_symtab_sort(sp_elf_section_t* symtab, sp_elf_t* elf);
sp_elf_section_t* sp_elf_rela_new(sp_elf_t* elf, sp_elf_section_t* target);
void sp_elf_add_relocation(sp_elf_section_t* rela, u64 offset, u32 sym_idx, u32 type, s64 addend);
Elf64_Rela* sp_elf_rela_get(sp_elf_section_t* rela, u32 index);
u32 sp_elf_strtab_size(sp_elf_section_t* strtab);
sp_str_t sp_elf_strtab_get(sp_elf_section_t* strtab, u32 offset);
sp_err_t sp_elf_write(sp_elf_t* elf, sp_io_writer_t* out);
sp_err_t sp_elf_write_to_file(sp_elf_t* elf, sp_str_t path);
sp_elf_t* sp_elf_read(sp_io_reader_t* in);
sp_elf_t* sp_elf_read_from_file(sp_str_t path);

#endif // SP_ELF_H



#if defined(SP_IMPLEMENTATION) && !defined(SP_ELF_IMPLEMENTATION)
  #define SP_ELF_IMPLEMENTATION
#endif

#if defined(SP_ELF_IMPLEMENTATION)

bool sp_elf_is_align_valid(u64 v) {
  return v == 0 || (v & (v - 1)) == 0;
}

sp_elf_t* sp_elf_new() {
  return sp_elf_new_alloc(sp_context_get()->allocator);
}

sp_elf_t* sp_elf_new_alloc(sp_mem_t allocator) {
  sp_context_push_allocator(allocator);
  sp_elf_t* elf = sp_alloc_type(sp_elf_t);
  elf->allocator = allocator;
  sp_context_pop();

  return elf;
}

sp_elf_t* sp_elf_new_with_null_section() {
  sp_elf_t* elf = sp_elf_new();

  sp_da_push(elf->sections, ((sp_elf_section_t){
    .name = sp_str_lit(""),
    .index = 0,
    .type = SHT_NULL,
    .alignment = 0,
  }));

  return elf;
}

void sp_elf_free(sp_elf_t* elf) {
  if (!elf) return;
  sp_mem_t alloc = elf->allocator;

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    if (!sp_str_empty(sec->name) && sec->name.data != SP_NULLPTR) {
      sp_mem_allocator_free(alloc, (void*)sec->name.data);
    }
    if (sec->buffer.data) {
      sp_mem_allocator_free(alloc, sec->buffer.data);
    }
  }

  sp_da_free(elf->sections);
  sp_str_ht_free(elf->section_map);
  sp_mem_allocator_free(alloc, elf);
}

sp_elf_section_t* sp_elf_add_section(sp_elf_t* elf, sp_str_t name, u32 kind, u64 alignment) {
  sp_require_as_null(elf);
  sp_require_as_null(!sp_str_empty(name));
  sp_require_as_null(sp_elf_is_align_valid(alignment));

  sp_context_push_allocator(elf->allocator);

  sp_elf_section_t section = {
    .name = sp_str_copy(name),
    .index = sp_da_size(elf->sections),
    .type = kind,
    .alignment = alignment,
  };

  sp_da_push(elf->sections, section);
  sp_str_ht_insert(elf->section_map, section.name, section.index);

  sp_context_pop();

  return sp_da_back(elf->sections);
}

sp_elf_section_t* sp_elf_find_section_by_index(sp_elf_t* elf, u32 index) {
  sp_require_as_null(elf);
  sp_require_as_null(sp_da_bounds_ok(elf->sections, index));

  return &elf->sections[index];
}

sp_elf_section_t* sp_elf_find_section_by_name(sp_elf_t* elf, sp_str_t name) {
  sp_require_as_null(elf);
  sp_require_as_null(sp_str_ht_get(elf->section_map, name));

  u32 index = *sp_str_ht_get(elf->section_map, name);
  return &elf->sections[index];
}

u32 sp_elf_num_sections(sp_elf_t* elf) {
  return elf ? sp_da_size(elf->sections) : 0;
}

void sp_elf_buffer_grow(sp_elf_buffer_t* buffer, u64 size) {
  if (size <= buffer->capacity) {
    return;
  }

  u64 capacity = SP_MAX(buffer->capacity, SP_ELF_DEFAULT_BUFFER_SIZE);
  while (capacity < size) {
    capacity *= 2;
  }

  buffer->capacity = capacity;
  buffer->data = (u8*)sp_realloc(buffer->data, capacity);
}

void* sp_elf_section_reserve_aligned(sp_elf_section_t* section, u64 size, u64 align) {
  sp_assert(section);

  u64 offset = sp_align_offset(section->buffer.size, align);
  u64 total = offset + size;
  sp_elf_buffer_grow(&section->buffer, total);

  section->buffer.size = total;
  return section->buffer.data + offset;
}

u8* sp_elf_section_reserve_bytes(sp_elf_section_t* section, u64 size) {
  return (u8*)sp_elf_section_reserve_aligned(section, size, 1);
}

void* sp_elf_section_reserve_entry(sp_elf_section_t* section) {
  return sp_elf_section_reserve_aligned(section, section->entry_size, 1);
}

u32 sp_elf_hash(sp_str_t name) {
  u32 h = 0;
  sp_for(i, name.len) {
    h = (h << 4) + (u8)name.data[i];
    u32 g = h & 0xf0000000;
    if (g) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

u32 sp_elf_add_string(sp_elf_section_t* strtab, sp_str_t str) {
  if (!strtab) return SP_ELF_NULL_INDEX;

  if (!strtab->buffer.size) {
    u8* p = sp_elf_section_reserve_bytes(strtab, 1);
    *p = 0;
  }

  if (sp_str_empty(str)) {
    return 0;
  }

  u32 index = (u32)strtab->buffer.size;
  u8* p = sp_elf_section_reserve_bytes(strtab, str.len + 1);
  sp_mem_copy(str.data, p, str.len);
  p[str.len] = 0;

  return index;
}

sp_elf_section_t* sp_elf_symtab_new(sp_elf_t* elf) {
  if (!elf) return SP_NULLPTR;

  u32 strtab_idx = sp_elf_add_section(elf, sp_str_lit(".strtab"), SHT_STRTAB, 1)->index;
  u32 symtab_idx = sp_elf_add_section(elf, sp_str_lit(".symtab"), SHT_SYMTAB, 8)->index;

  sp_elf_section_t* strtab = sp_elf_find_section_by_index(elf, strtab_idx);
  sp_elf_section_t* symtab = sp_elf_find_section_by_index(elf, symtab_idx);

  symtab->link = strtab_idx;
  symtab->entry_size = sizeof(Elf64_Sym);

  sp_elf_add_string(strtab, sp_str_lit(""));

  Elf64_Sym* sym0 = (Elf64_Sym*)sp_elf_section_reserve_entry(symtab);
  sp_mem_zero(sym0, sizeof(Elf64_Sym));

  return symtab;
}

u32 sp_elf_section_num_entries(sp_elf_section_t* symtab) {
  return (u32)(symtab->buffer.size / symtab->entry_size);
}

Elf64_Sym* sp_elf_symtab_get(sp_elf_section_t* symtab, u32 index) {
  SP_ASSERT(symtab && symtab->type == SHT_SYMTAB);
  SP_ASSERT(index < sp_elf_section_num_entries(symtab));
  return (Elf64_Sym*)symtab->buffer.data + index;
}

Elf64_Rela* sp_elf_rela_get(sp_elf_section_t* rela, u32 index) {
  SP_ASSERT(rela && rela->type == SHT_RELA);
  SP_ASSERT(index < sp_elf_section_num_entries(rela));
  return (Elf64_Rela*)rela->buffer.data + index;
}

u32 sp_elf_strtab_size(sp_elf_section_t* strtab) {
  SP_ASSERT(strtab && strtab->type == SHT_STRTAB);
  return (u32)strtab->buffer.size;
}

sp_str_t sp_elf_strtab_get(sp_elf_section_t* strtab, u32 offset) {
  SP_ASSERT(strtab && strtab->type == SHT_STRTAB);
  SP_ASSERT(offset < strtab->buffer.size);
  return sp_str_view((const c8*)strtab->buffer.data + offset);
}

u32 sp_elf_add_symbol(sp_elf_section_t* symbols, sp_elf_t* elf, sp_str_t name, u64 value, u64 size, u8 bind, u8 type, u16 shndx) {
  sp_require_as(symbols, SP_ELF_NULL_INDEX);
  sp_require_as(elf, SP_ELF_NULL_INDEX);

  sp_elf_section_t* strings = sp_elf_find_section_by_index(elf, symbols->link);
  u32 sym_idx = sp_elf_section_num_entries(symbols);

  Elf64_Sym* sym = (Elf64_Sym*)sp_elf_section_reserve_entry(symbols);
  sym->st_name = sp_elf_add_string(strings, name);
  sym->st_value = value;
  sym->st_size = size;
  sym->st_info = ELF64_ST_INFO(bind, type);
  sym->st_other = 0;
  sym->st_shndx = shndx;

  return sym_idx;
}

sp_elf_section_t* sp_elf_rela_new(sp_elf_t* elf, sp_elf_section_t* target) {
  if (!elf) return SP_NULLPTR;
  if (!target) return SP_NULLPTR;

  sp_elf_section_t* symtab = sp_elf_find_section_by_name(elf, sp_str_lit(".symtab"));
  if (!symtab) return SP_NULLPTR;

  sp_str_t name = sp_fmt(".rela{}", sp_fmt_str(target->name));
  sp_elf_section_t* rela = sp_elf_add_section(elf, name, SHT_RELA, 8);
  if (!rela) return SP_NULLPTR;
  rela->link = symtab->index;
  rela->info = target->index;
  rela->entry_size = sizeof(Elf64_Rela);

  return rela;
}

void sp_elf_add_relocation(sp_elf_section_t* rela, u64 offset, u32 sym_idx, u32 type, s64 addend) {
  if (!rela) return;

  Elf64_Rela* rel = (Elf64_Rela*)sp_elf_section_reserve_entry(rela);
  rel->r_offset = offset;
  rel->r_info = ELF64_R_INFO(sym_idx, type);
  rel->r_addend = addend;
}

void sp_elf_symtab_sort(sp_elf_section_t* symtab, sp_elf_t* elf) {
  if (!symtab) return;
  SP_ASSERT(symtab->type == SHT_SYMTAB);

  u32 count = sp_elf_section_num_entries(symtab);
  if (count <= 1) {
    symtab->info = count;
    return;
  }

  sp_da(u32) old_to_new = SP_NULLPTR;
  sp_for(i, count) {
    sp_da_push(old_to_new, 0);
  }

  Elf64_Sym* syms = (Elf64_Sym*)symtab->buffer.data;
  Elf64_Sym* new_syms = sp_alloc_n(Elf64_Sym, count);
  u32 local_idx = 0;
  SP_UNUSED(count);

  sp_for(i, count) {
    if (ELF64_ST_BIND(syms[i].st_info) == STB_LOCAL) {
      old_to_new[i] = local_idx;
      new_syms[local_idx++] = syms[i];
    }
  }

  u32 first_nonlocal = local_idx;

  sp_for(i, count) {
    if (ELF64_ST_BIND(syms[i].st_info) != STB_LOCAL) {
      old_to_new[i] = local_idx;
      new_syms[local_idx++] = syms[i];
    }
  }

  sp_mem_copy(new_syms, syms, count * sizeof(Elf64_Sym));
  symtab->info = first_nonlocal;

  if (elf) {
    sp_for(i, sp_elf_num_sections(elf)) {
      sp_elf_section_t* sec = sp_elf_find_section_by_index(elf, i);
      if (sec->type == SHT_RELA && sec->link == symtab->index) {
        u32 rela_count = sp_elf_section_num_entries(sec);
        sp_for(j, rela_count) {
          Elf64_Rela* rel = sp_elf_rela_get(sec, j);
          u32 sym_idx = ELF64_R_SYM(rel->r_info);
          u32 type = ELF64_R_TYPE(rel->r_info);
          rel->r_info = ELF64_R_INFO(old_to_new[sym_idx], type);
        }
      }
    }
  }

  sp_da_free(old_to_new);
}

sp_elf_section_t* sp_elf_find_or_create_shstrtab(sp_elf_t* elf) {
  sp_elf_section_t* table = sp_elf_find_section_by_name(elf, sp_str_lit(".shstrtab"));
  if (table) {
    return table;
  }

  table = sp_elf_add_section(elf, sp_str_lit(".shstrtab"), SHT_STRTAB, 1);
  sp_elf_add_string(table, sp_str_lit(""));
  return table;
}

sp_err_t sp_elf_write(sp_elf_t* elf, sp_io_writer_t* out) {
  sp_require_as(elf, SP_ERR_LAZY);
  sp_require_as(out, SP_ERR_LAZY);

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    if (sec->type == SHT_SYMTAB) {
      sp_elf_symtab_sort(sec, elf);
    }
  }

  sp_elf_section_t* shstrtab = sp_elf_find_or_create_shstrtab(elf);
  u32 num_sections = sp_elf_num_sections(elf);

  sp_da(Elf64_Shdr) headers = SP_NULLPTR;
  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    sp_da_push(headers, ((Elf64_Shdr) {
      .sh_name = sp_elf_add_string(shstrtab, sec->name),
      .sh_type = sec->type,
      .sh_flags = sec->flags,
      .sh_addr = sec->addr,
      .sh_size = sec->buffer.size,
      .sh_link = sec->link,
      .sh_info = sec->info,
      .sh_addralign = sec->alignment,
      .sh_entsize = sec->entry_size,
    }));
  }

  u64 shoff = sp_align_offset(sizeof(Elf64_Ehdr), 4);
  u64 file_offset = shoff + num_sections * sizeof(Elf64_Shdr);

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    switch (sec->type) {
      case SHT_NULL:
      case SHT_NOBITS: {
        headers[i].sh_offset = 0;
        break;
      }
      case SHT_PROGBITS:
      case SHT_SYMTAB:
      case SHT_STRTAB:
      case SHT_RELA: {
        file_offset = sp_align_offset(file_offset, 16);
        headers[i].sh_offset = file_offset;
        headers[i].sh_size = sec->buffer.size;
        file_offset += sec->buffer.size;
        break;
      }
    }
  }

  Elf64_Ehdr ehdr = {
    .e_type = ET_REL,
    .e_machine = EM_X86_64,
    .e_version = EV_CURRENT,
    .e_entry = 0,
    .e_phoff = 0,
    .e_shoff = shoff,
    .e_flags = 0,
    .e_ehsize = sizeof(Elf64_Ehdr),
    .e_phentsize = 0,
    .e_phnum = 0,
    .e_shentsize = sizeof(Elf64_Shdr),
    .e_shnum = (u16)num_sections,
    .e_shstrndx = (u16)shstrtab->index,
  };
  ehdr.e_ident[EI_MAG0] = ELFMAG0;
  ehdr.e_ident[EI_MAG1] = ELFMAG1;
  ehdr.e_ident[EI_MAG2] = ELFMAG2;
  ehdr.e_ident[EI_MAG3] = ELFMAG3;
  ehdr.e_ident[EI_CLASS] = ELFCLASS64;
  ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
  ehdr.e_ident[EI_VERSION] = EV_CURRENT;
  ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;

  u64 written = 0;
  u64 n = 0;
  sp_try(sp_io_write(out, &ehdr, sizeof(Elf64_Ehdr), &n));
  written += n;
  sp_try(sp_io_pad(out, shoff - written, &n));
  written += n;

  sp_da_for(headers, i) {
    sp_try(sp_io_write(out, &headers[i], sizeof(Elf64_Shdr), &n));
    written += n;
  }

  sp_da_for(elf->sections, i) {
    sp_elf_section_t* sec = &elf->sections[i];
    switch (sec->type) {
      case SHT_NULL:
      case SHT_NOBITS: {
        break;
      }
      case SHT_PROGBITS:
      case SHT_SYMTAB:
      case SHT_STRTAB:
      case SHT_RELA: {
        sp_try(sp_io_pad(out, headers[i].sh_offset - written, &n));
        written += n;
        sp_try(sp_io_write(out, sec->buffer.data, sec->buffer.size, &n));
        written += n;
        break;
      }
    }
  }

  sp_da_free(headers);
  return SP_OK;
}

sp_err_t sp_elf_write_to_file(sp_elf_t* elf, sp_str_t path) {
  sp_io_writer_t f = SP_ZERO_INITIALIZE();
  sp_try(sp_io_writer_from_file(&f, path, SP_IO_WRITE_MODE_OVERWRITE));
  sp_err_t err = sp_elf_write(elf, &f);
  sp_io_writer_close(&f);
  return err;
}

sp_elf_t* sp_elf_read(sp_io_reader_t* in) {
  sp_require_as_null(in);

  Elf64_Ehdr ehdr = SP_ZERO_INITIALIZE();
  u64 bytes_read = 0;
  sp_io_read(in, &ehdr, sizeof(Elf64_Ehdr), &bytes_read);
  if (bytes_read != sizeof(Elf64_Ehdr)) {
    return SP_NULLPTR;
  }

  u16 num_sections = ehdr.e_shnum;
  sp_require_as_null(ehdr.e_shstrndx < num_sections);

  if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
      ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
      ehdr.e_ident[EI_MAG3] != ELFMAG3) {
    return SP_NULLPTR;
  }

  if (ehdr.e_ident[EI_CLASS] != ELFCLASS64) {
    return SP_NULLPTR;
  }

  if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
    return SP_NULLPTR;
  }

  if (!num_sections) {
    return sp_elf_new();
  }

  Elf64_Shdr* section_headers = sp_alloc_n(Elf64_Shdr, num_sections);
  sp_io_reader_seek(in, (s64)ehdr.e_shoff, SP_IO_SEEK_SET, SP_NULLPTR);
  sp_io_read(in, section_headers, num_sections * sizeof(Elf64_Shdr), &bytes_read);
  if (bytes_read != num_sections * sizeof(Elf64_Shdr)) {
    return SP_NULLPTR;
  }

  Elf64_Shdr* string_header = &section_headers[ehdr.e_shstrndx];

  c8* string_table = SP_NULLPTR;
  if (string_header->sh_size) {
    string_table = sp_alloc_n(c8, string_header->sh_size);
    sp_io_reader_seek(in, string_header->sh_offset, SP_IO_SEEK_SET, SP_NULLPTR);
    sp_io_read(in, string_table, string_header->sh_size, SP_NULLPTR);
  }

  sp_elf_t* elf = sp_elf_new();

  sp_assert(section_headers[0].sh_type == SHT_NULL);
  sp_da_push(elf->sections, ((sp_elf_section_t) {
    .type = SHT_NULL
  }));

  sp_for_range(it, 1, num_sections) {
    if (it == ehdr.e_shstrndx) {
      continue;
    }

    Elf64_Shdr* header = &section_headers[it];

    sp_assert(header->sh_name < string_header->sh_size);
    sp_str_t name = sp_str_view(string_table + header->sh_name);

    sp_elf_section_t* section = sp_elf_add_section(elf, name, header->sh_type, header->sh_addralign);
    section->flags = header->sh_flags;
    section->addr = header->sh_addr;
    section->link = header->sh_link;
    section->info = header->sh_info;
    section->entry_size = header->sh_entsize;

    if (header->sh_size) {
      switch (header->sh_type) {
        case SHT_NOBITS: {
          section->buffer.size = header->sh_size;
          break;
        }
        case SHT_NULL:
        case SHT_PROGBITS:
        case SHT_SYMTAB:
        case SHT_STRTAB:
        case SHT_RELA: {
          sp_io_reader_seek(in, header->sh_offset, SP_IO_SEEK_SET, SP_NULLPTR);
          u8* ptr = sp_elf_section_reserve_bytes(section, header->sh_size);
          sp_io_read(in, ptr, header->sh_size, SP_NULLPTR);
          break;
        }
      }
    }
  }

  return elf;
}

sp_elf_t* sp_elf_read_from_file(sp_str_t path) {
  sp_io_reader_t f = SP_ZERO_INITIALIZE();
  if (sp_io_reader_from_file(&f, path)) {
    return SP_NULLPTR;
  }
  sp_elf_t* elf = sp_elf_read(&f);
  sp_io_reader_close(&f);
  return elf;
}

#endif // SP_ELF_IMPLEMENTATION
