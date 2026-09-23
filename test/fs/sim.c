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

static sp_str_t resolve(sp_sys_fd_t fd, const c8* path, u32 len, c8 buf [SP_PATH_MAX]) {
  s64 slot = (s64)fd - FD_BASE;
  if (slot < 0 || slot >= (s64)active->count.dirs) return sp_str(path, len);

  sp_str_t base = sp_cstr_as_str(active->opened[slot].dir->path);
  sp_assert(base.len + 1 + len < SP_PATH_MAX);
  sp_str_copy_to(base, buf, SP_PATH_MAX);
  buf[base.len] = '/';
  sp_mem_copy(buf + base.len + 1, path, len);
  return sp_str(buf, base.len + 1 + len);
}

static void record(sp_str_t path, sp_fs_kind_t kind) {
  sp_assert(active->num_removed < SIM_MAX_REMOVED);
  sp_assert(path.len < SIM_MAX_REMOVED_PATH);
  sim_removed_t* out = &active->removed[active->num_removed++];
  sp_str_copy_to(path, out->path, SIM_MAX_REMOVED_PATH);
  out->kind = kind;
}

static sp_err_t open_dir(sp_sys_fd_t fd, const c8* path, u32 len, u32 flags, sp_sys_fd_t* out) {
  active->count.opens++;
  c8 buf [SP_PATH_MAX];
  const sim_dir_t* dir = find_dir(resolve(fd, path, len, buf));
  if (!dir) return SP_ERR_SYS_NOT_FOUND;
  if (dir->open) return dir->open;
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

static sp_err_t dir_it_open(sp_sys_fd_t fd, sp_sys_dir_it_t* out) {
  s64 slot = (s64)fd - FD_BASE;
  if (active->opened[slot].dir->it_open) return active->opened[slot].dir->it_open;
  *out = sp_zero_s(sp_sys_dir_it_t);
  out->fd = fd;
  out->state = slot;
  return SP_OK;
}

static sp_err_t dir_it_read(sp_sys_dir_it_t* it, sp_mem_buffer_t* buf) {
  sim_open_t* slot = &active->opened[it->state];
  if (slot->dir->read) return slot->dir->read;
  buf->len = slot->served ? 0 : dir_len(slot->dir);
  slot->served = true;
  return SP_OK;
}

static sp_err_t dir_it_parse(sp_sys_dir_it_t* it, sp_mem_buffer_t* buf, u64* cursor, sp_sys_dir_entry_t* out) {
  const sim_entry_t* entry = &active->opened[it->state].dir->entries[*cursor];
  *cursor += 1;

  *out = sp_zero_s(sp_sys_dir_entry_t);
  out->name = entry->name;
  out->len = (u32)sp_cstr_len(entry->name);
  out->kind = entry->kind;
  return SP_OK;
}

static sp_err_t dir_it_close(sp_sys_dir_it_t* it) {
  active->count.closes++;
  return SP_OK;
}

static sp_err_t get_link_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  c8 buf [SP_PATH_MAX];
  sp_str_t full = resolve(fd, path, len, buf);
  *st = sp_zero_s(sp_sys_file_meta_t);

  const sim_entry_t* entry = find_entry(full);
  if (entry) {
    st->kind = entry->stat;
    return SP_OK;
  }
  if (find_dir(full)) {
    st->kind = SP_FS_KIND_DIR;
    return SP_OK;
  }
  return SP_ERR_SYS_NOT_FOUND;
}

static sp_err_t unlink_entry(sp_sys_fd_t fd, const c8* path, u32 len) {
  c8 buf [SP_PATH_MAX];
  sp_str_t full = resolve(fd, path, len, buf);
  const sim_entry_t* entry = find_entry(full);
  if (!entry) return SP_ERR_SYS_NOT_FOUND;
  if (entry->unlink) return entry->unlink;
  record(full, SP_FS_KIND_FILE);
  return SP_OK;
}

static sp_err_t remove_dir(sp_sys_fd_t fd, const c8* path, u32 len) {
  c8 buf [SP_PATH_MAX];
  sp_str_t full = resolve(fd, path, len, buf);
  const sim_dir_t* dir = find_dir(full);
  if (!dir) return SP_ERR_SYS_NOT_FOUND;
  if (dir->rmdir) return dir->rmdir;
  record(full, SP_FS_KIND_DIR);
  return SP_OK;
}

void sim_begin(sim_t* sim, const sim_dir_t* dirs) {
  *sim = (sim_t) { .dirs = dirs, .vt = sp_sys_vtable_platform };
  sim->vt.open_dir = open_dir;
  sim->vt.close = close_fd;
  sim->vt.dir_it_open = dir_it_open;
  sim->vt.dir_it_read = dir_it_read;
  sim->vt.dir_it_parse = dir_it_parse;
  sim->vt.dir_it_close = dir_it_close;
  sim->vt.get_link_metadata = get_link_metadata;
  sim->vt.unlink = unlink_entry;
  sim->vt.rmdir = remove_dir;
  sim->saved = sp_sys_set_vtable(&sim->vt);
  active = sim;
}

void sim_end(sim_t* sim) {
  sp_sys_set_vtable(sim->saved);
  active = SP_NULLPTR;
}
