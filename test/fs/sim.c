#include "sim.h"

#define FD_BASE 7777

static sim_t* active;

static const sim_dir_t* find_dir(sp_str_t path) {
  sp_for(it, SIM_MAX_DIRS) {
    const sim_dir_t* dir = &active->dirs[it];
    if (!dir->path) break;
    if (sp_str_equal_cstr(path, dir->path)) return dir;
  }
  return SP_NULLPTR;
}

static const sim_entry_t* find_entry(sp_str_t path) {
  const sim_dir_t* dir = find_dir(sp_fs_parent_path(path));
  if (!dir) return SP_NULLPTR;

  sp_str_t name = sp_fs_get_name(path);
  sp_carr_for(dir->entries, it) {
    const sim_entry_t* entry = &dir->entries[it];
    if (!entry->name) break;
    if (sp_str_equal_cstr(name, entry->name)) return entry;
  }
  return SP_NULLPTR;
}

static u32 dir_len(const sim_dir_t* dir) {
  u32 n = 0;
  sp_carr_for(dir->entries, it) {
    if (!dir->entries[it].name) break;
    n++;
  }
  return n;
}

static sp_err_t open_dir(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_fd_t* out) {
  active->count.opens++;
  const sim_dir_t* dir = find_dir(sp_str(path, len));
  if (!dir) return SP_ERR_SYS_NOT_FOUND;
  sp_assert(active->count.dirs < SIM_MAX_OPENS);
  active->opened[active->count.dirs] = (sim_open_t) { .dir = dir };
  *out = (sp_sys_fd_t)(FD_BASE + active->count.dirs);
  active->count.dirs++;
  return SP_OK;
}

static sp_err_t close_fd(sp_sys_fd_t fd) {
  active->count.fd_closes++;
  return SP_OK;
}

static sp_err_t dir_from_fd(sp_sys_fd_t fd, sp_sys_dir_t* out) {
  s64 handle = (s64)fd - FD_BASE;
  if (active->opened[handle].dir->from_fd) return active->opened[handle].dir->from_fd;
  *out = sp_zero_s(sp_sys_dir_t);
  out->handle = handle;
  return SP_OK;
}

static sp_err_t dir_read(sp_sys_dir_t* dir, sp_mem_buffer_t* buf) {
  sim_open_t* slot = &active->opened[dir->handle];
  if (slot->dir->read) return slot->dir->read;
  buf->len = slot->served ? 0 : dir_len(slot->dir);
  slot->served = true;
  return SP_OK;
}

static sp_err_t dir_parse(sp_sys_dir_t* dir, sp_mem_buffer_t* buf, u64* cursor, sp_sys_dir_entry_t* out) {
  const sim_entry_t* entry = &active->opened[dir->handle].dir->entries[*cursor];
  *cursor += 1;

  *out = sp_zero_s(sp_sys_dir_entry_t);
  out->name = entry->name;
  out->len = (u32)sp_cstr_len(entry->name);
  out->kind = entry->kind;
  return SP_OK;
}

static sp_err_t dir_close(sp_sys_dir_t* dir) {
  active->count.closes++;
  return SP_OK;
}

static sp_err_t get_link_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  const sim_entry_t* entry = find_entry(sp_str(path, len));
  if (!entry) return SP_ERR_SYS_NOT_FOUND;
  *st = sp_zero_s(sp_sys_file_meta_t);
  st->kind = entry->stat;
  return SP_OK;
}

void sim_begin(sim_t* sim, const sim_dir_t* dirs) {
  *sim = (sim_t) { .dirs = dirs, .vt = sp_sys_vtable_platform };
  sim->vt.open_dir = open_dir;
  sim->vt.close = close_fd;
  sim->vt.dir_from_fd = dir_from_fd;
  sim->vt.dir_read = dir_read;
  sim->vt.dir_parse = dir_parse;
  sim->vt.dir_close = dir_close;
  sim->vt.get_link_metadata = get_link_metadata;
  sim->saved = sp_sys_set_vtable(&sim->vt);
  active = sim;
}

void sim_end(sim_t* sim) {
  sp_sys_set_vtable(sim->saved);
  active = SP_NULLPTR;
}
