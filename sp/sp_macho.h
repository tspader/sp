#ifndef SP_MACHO_H
#define SP_MACHO_H

#include "sp.h"

#if defined(SP_MACHO)

#define SP_MACHO_ARENA_BLOCK_SIZE 4096

typedef enum {
  SP_MACHO_SYM_LOCAL,
  SP_MACHO_SYM_EXTERNAL,
} sp_macho_sym_bind_t;

typedef struct {
  sp_str_t name;
  u32 index;
  u64 align;
  u64 flags;
  sp_da(u8) data;
} sp_macho_section_t;

typedef struct {
  sp_str_t name;
  u64 value;
  u8 sect;
  sp_macho_sym_bind_t bind;
} sp_macho_symbol_t;

typedef struct {
  sp_allocator_t allocator;
  sp_mem_arena_t* arena;
  sp_da(sp_macho_section_t) sections;
  sp_da(sp_macho_symbol_t) symbols;
} sp_macho_t;

sp_macho_t* sp_macho_new();
void sp_macho_free(sp_macho_t* m);
u32 sp_macho_add_section(sp_macho_t* m, sp_str_t name, u64 align);
sp_macho_section_t* sp_macho_get_section(sp_macho_t* m, u32 index);
u8* sp_macho_section_push(sp_macho_t* m, u32 sect, const void* data, u64 size);
u32 sp_macho_add_symbol(sp_macho_t* m, sp_str_t name, u32 sect, u64 offset, sp_macho_sym_bind_t bind);
sp_err_t sp_macho_write(sp_macho_t* m, sp_io_writer_t* out);
sp_err_t sp_macho_write_to_file(sp_macho_t* m, sp_str_t path);

#endif

#if defined(SP_IMPLEMENTATION) && defined(SP_MACHO)

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach/machine.h>

sp_macho_t* sp_macho_new() {
  sp_allocator_t allocator = sp_context_get()->allocator;
  sp_macho_t* m = sp_alloc_type(sp_macho_t);
  m->allocator = allocator;
  m->arena = sp_mem_arena_new(SP_MACHO_ARENA_BLOCK_SIZE);
  return m;
}

void sp_macho_free(sp_macho_t* m) {
  if (!m) return;
  sp_allocator_t allocator = m->allocator;
  sp_mem_arena_destroy(m->arena);
  sp_mem_allocator_free(allocator, m);
}

u32 sp_macho_add_section(sp_macho_t* m, sp_str_t name, u64 align) {
  if (!m) return 0;
  if (sp_str_empty(name)) return 0;

  sp_context_push_allocator(sp_mem_arena_as_allocator(m->arena));

  u32 index = sp_da_size(m->sections) + 1;
  sp_macho_section_t section = {
    .name = sp_str_copy_a(sp_mem_arena_as_allocator(m->arena), name),
    .index = index,
    .align = align,
    .flags = 0,
  };
  sp_da_push(m->sections, section);

  sp_context_pop();
  return index;
}

sp_macho_section_t* sp_macho_get_section(sp_macho_t* m, u32 index) {
  if (!m) return SP_NULLPTR;
  if (index == 0 || index > sp_da_size(m->sections)) return SP_NULLPTR;
  return &m->sections[index - 1];
}

u8* sp_macho_section_push(sp_macho_t* m, u32 sect, const void* data, u64 size) {
  sp_macho_section_t* s = sp_macho_get_section(m, sect);
  if (!s) return SP_NULLPTR;
  if (!size) return SP_NULLPTR;

  sp_context_push_allocator(sp_mem_arena_as_allocator(m->arena));

  u64 old_size = sp_da_size(s->data);
  u64 new_size = old_size + size;
  sp_da_reserve(s->data, new_size);
  sp_da_head(s->data)->size = (u32)new_size;
  u8* dest = s->data + old_size;
  if (data) {
    sp_mem_copy(data, dest, (u32)size);
  }

  sp_context_pop();
  return dest;
}

u32 sp_macho_add_symbol(sp_macho_t* m, sp_str_t name, u32 sect, u64 offset, sp_macho_sym_bind_t bind) {
  if (!m) return 0;

  sp_context_push_allocator(sp_mem_arena_as_allocator(m->arena));

  u32 idx = sp_da_size(m->symbols);
  sp_macho_symbol_t sym = {
    .name = sp_str_copy_a(sp_mem_arena_as_allocator(m->arena), name),
    .value = offset,
    .sect = (u8)sect,
    .bind = bind,
  };
  sp_da_push(m->symbols, sym);

  sp_context_pop();
  return idx;
}

u64 sp_macho_align(u64 offset, u64 align) {
  if (align == 0) return offset;
  u64 mask = align - 1;
  return (offset + mask) & ~mask;
}

sp_err_t sp_macho_write(sp_macho_t* m, sp_io_writer_t* out) {
  if (!m || !out) return SP_ERR_LAZY;

  u32 nsects = sp_da_size(m->sections);
  u32 nsyms = sp_da_size(m->symbols);

  u32 ncmds = 1;
  u32 sizeofcmds = sizeof(struct segment_command_64) + nsects * sizeof(struct section_64);

  if (nsyms > 0) {
    ncmds += 2;
    sizeofcmds += sizeof(struct symtab_command);
    sizeofcmds += sizeof(struct dysymtab_command);
  }

  ncmds += 1;
  sizeofcmds += sizeof(struct build_version_command);

  u64 header_size = sizeof(struct mach_header_64) + sizeofcmds;

  u64 file_offset = header_size;
  sp_da(u64) section_offsets = SP_NULLPTR;
  sp_da(u64) section_sizes = SP_NULLPTR;

  sp_da_for(m->sections, i) {
    sp_macho_section_t* sec = &m->sections[i];
    u64 align = 1ULL << sec->align;
    file_offset = sp_macho_align(file_offset, align);
    sp_da_push(section_offsets, file_offset);
    u64 sz = sp_da_size(sec->data);
    sp_da_push(section_sizes, sz);
    file_offset += sz;
  }

  u64 symtab_offset = sp_macho_align(file_offset, 8);
  u64 strtab_offset = symtab_offset + nsyms * sizeof(struct nlist_64);

  u64 strtab_size = 2;
  sp_da_for(m->symbols, i) {
    strtab_size += m->symbols[i].name.len + 2;
  }
  strtab_size = sp_macho_align(strtab_size, 8);

  struct mach_header_64 header = {
    .magic = MH_MAGIC_64,
    .cputype = CPU_TYPE_ARM64,
    .cpusubtype = CPU_SUBTYPE_ARM64_ALL,
    .filetype = MH_OBJECT,
    .ncmds = ncmds,
    .sizeofcmds = sizeofcmds,
    .flags = MH_SUBSECTIONS_VIA_SYMBOLS,
    .reserved = 0,
  };
  sp_try(sp_io_write(out, &header, sizeof(header), SP_NULLPTR));

  u64 total_vmsize = 0;
  u64 total_filesize = 0;
  sp_da_for(m->sections, i) {
    u64 align = 1ULL << m->sections[i].align;
    total_vmsize = sp_macho_align(total_vmsize, align);
    total_vmsize += section_sizes[i];
    total_filesize += section_sizes[i];
  }

  struct segment_command_64 seg = {
    .cmd = LC_SEGMENT_64,
    .cmdsize = (u32)(sizeof(struct segment_command_64) + nsects * sizeof(struct section_64)),
    .vmaddr = 0,
    .vmsize = total_vmsize,
    .fileoff = header_size,
    .filesize = total_filesize,
    .maxprot = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE,
    .initprot = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE,
    .nsects = nsects,
    .flags = 0,
  };
  sp_mem_zero(seg.segname, 16);
  sp_try(sp_io_write(out, &seg, sizeof(seg), SP_NULLPTR));

  u64 vmaddr = 0;
  sp_da_for(m->sections, i) {
    sp_macho_section_t* sec = &m->sections[i];
    u64 align = 1ULL << sec->align;
    vmaddr = sp_macho_align(vmaddr, align);

    struct section_64 sect = {
      .addr = vmaddr,
      .size = section_sizes[i],
      .offset = (u32)section_offsets[i],
      .align = (u32)sec->align,
      .reloff = 0,
      .nreloc = 0,
      .flags = S_REGULAR,
      .reserved1 = 0,
      .reserved2 = 0,
      .reserved3 = 0,
    };

    sp_mem_zero(sect.sectname, 16);
    sp_mem_zero(sect.segname, 16);

    sp_str_t sectname = sec->name;
    sp_str_t segname = sp_str_lit("__DATA");
    if (sectname.len > 16) sectname.len = 16;
    sp_mem_copy(sectname.data, sect.sectname, (u32)sectname.len);
    sp_mem_copy(segname.data, sect.segname, (u32)segname.len);

    sp_try(sp_io_write(out, &sect, sizeof(sect), SP_NULLPTR));
    vmaddr += section_sizes[i];
  }

  if (nsyms > 0) {
    struct symtab_command symtab = {
      .cmd = LC_SYMTAB,
      .cmdsize = sizeof(struct symtab_command),
      .symoff = (u32)symtab_offset,
      .nsyms = nsyms,
      .stroff = (u32)strtab_offset,
      .strsize = (u32)strtab_size,
    };
    sp_try(sp_io_write(out, &symtab, sizeof(symtab), SP_NULLPTR));

    u32 nlocal = 0;
    u32 nextdef = 0;
    sp_da_for(m->symbols, i) {
      switch (m->symbols[i].bind) {
        case SP_MACHO_SYM_LOCAL: {
          nlocal++;
          break;
        }
        case SP_MACHO_SYM_EXTERNAL: {
          nextdef++;
          break;
        }
      }
    }

    struct dysymtab_command dysymtab = {
      .cmd = LC_DYSYMTAB,
      .cmdsize = sizeof(struct dysymtab_command),
      .ilocalsym = 0,
      .nlocalsym = nlocal,
      .iextdefsym = nlocal,
      .nextdefsym = nextdef,
      .iundefsym = nlocal + nextdef,
      .nundefsym = 0,
    };
    sp_try(sp_io_write(out, &dysymtab, sizeof(dysymtab), SP_NULLPTR));
  }

  struct build_version_command buildver = {
    .cmd = LC_BUILD_VERSION,
    .cmdsize = sizeof(struct build_version_command),
    .platform = PLATFORM_MACOS,
    .minos = 0x000E0000,
    .sdk = 0x000E0000,
    .ntools = 0,
  };
  sp_try(sp_io_write(out, &buildver, sizeof(buildver), SP_NULLPTR));

  u64 written = sizeof(struct mach_header_64) + sizeofcmds;
  sp_da_for(m->sections, i) {
    sp_macho_section_t* sec = &m->sections[i];
    while (written < section_offsets[i]) {
      u8 zero = 0;
      sp_try(sp_io_write(out, &zero, 1, SP_NULLPTR));
      written++;
    }
    if (sp_da_size(sec->data) > 0) {
      sp_try(sp_io_write(out, sec->data, sp_da_size(sec->data), SP_NULLPTR));
      written += sp_da_size(sec->data);
    }
  }

  while (written < symtab_offset) {
    u8 zero = 0;
    sp_io_write(out, &zero, 1);
    written++;
  }

  sp_da_for(m->symbols, i) {
    sp_macho_symbol_t* sym = &m->symbols[i];
    struct nlist_64 nl = SP_ZERO_INITIALIZE();

    u32 strx = 2;
    sp_for(j, i) {
      strx += (u32)m->symbols[j].name.len + 2;
    }

    nl.n_un.n_strx = strx;
    nl.n_type = N_SECT;
    if (sym->bind == SP_MACHO_SYM_EXTERNAL) {
      nl.n_type |= N_EXT;
    }
    nl.n_sect = sym->sect;
    nl.n_desc = 0;
    nl.n_value = sym->value;

    sp_try(sp_io_write(out, &nl, sizeof(nl), SP_NULLPTR));
  }

  c8 strtab_header[2] = {' ', '\0'};
  sp_try(sp_io_write(out, strtab_header, 2, SP_NULLPTR));

  sp_da_for(m->symbols, i) {
    sp_macho_symbol_t* sym = &m->symbols[i];
    c8 underscore = '_';
    sp_try(sp_io_write(out, &underscore, 1, SP_NULLPTR));
    sp_try(sp_io_write(out, sym->name.data, sym->name.len, SP_NULLPTR));
    c8 null = '\0';
    sp_try(sp_io_write(out, &null, 1, SP_NULLPTR));
  }

  u64 strtab_written = 2;
  sp_da_for(m->symbols, i) {
    strtab_written += m->symbols[i].name.len + 2;
  }
  while (strtab_written < strtab_size) {
    u8 zero = 0;
    sp_try(sp_io_write(out, &zero, 1, SP_NULLPTR));
    strtab_written++;
  }

  sp_da_free(section_offsets);
  sp_da_free(section_sizes);

  return SP_ERR_OK;
}

sp_err_t sp_macho_write_to_file(sp_macho_t* m, sp_str_t path) {
  sp_io_writer_t f = SP_ZERO_INITIALIZE();
  sp_try(sp_io_writer_from_file(&f, path, SP_IO_WRITE_MODE_OVERWRITE));
  sp_err_t err = sp_macho_write(m, &f);
  sp_io_writer_close(&f);
  return err;
}

#endif

#endif
