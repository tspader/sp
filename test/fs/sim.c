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

static s64 slot_of(sp_sys_fd_t fd) {
  return (s64)fd - FD_BASE;
}

static sp_str_t resolve(sp_mem_t mem, sp_sys_fd_t fd, const c8* path, u32 len) {
  if (fd == sp_sys_get_root(0)) return sp_str(path, len);
  s64 slot = slot_of(fd);
  sp_assert(slot >= 0 && slot < (s64)active->count.dirs);
  return sp_fs_join_path(mem, sp_cstr_as_str(active->opened[slot].dir->path), sp_str(path, len));
}

static void mark_gone(sp_str_t path) {
  const sim_dir_t* dir = find_dir(sp_fs_parent_path(path));
  const sim_entry_t* entry = find_entry(path);
  if (entry) active->gone[dir - active->dirs][entry - dir->entries] = true;
}

static void record(sp_str_t path, sim_op_t op) {
  sp_assert(active->num_removed < SIM_MAX_REMOVED);
  sp_assert(path.len < SIM_MAX_REMOVED_PATH);
  sim_removed_t* out = &active->removed[active->num_removed++];
  sp_str_copy_to(path, out->path, SIM_MAX_REMOVED_PATH);
  out->op = op;
  mark_gone(path);
}

static bool dir_empty(const sim_dir_t* dir) {
  sp_carr_for(dir->entries, it) {
    const sim_entry_t* entry = &dir->entries[it];
    if (!entry->name) break;
    if (entry->absent) continue;
    if (!active->gone[dir - active->dirs][it]) return false;
  }
  return true;
}

static sp_err_t open_dir(sp_sys_fd_t fd, const c8* path, u32 len, u32 flags, sp_sys_fd_t* out) {
  active->count.opens++;
  if (flags & SP_SYS_OPEN_DIR_NO_FOLLOW) active->count.nofollow++;
  if (active->count.dirs == SIM_MAX_OPENS) return SP_ERR_SYS_TOO_MANY_FILES;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_str_t full = resolve(s.mem, fd, path, len);
  const sim_entry_t* entry = find_entry(full);
  const sim_dir_t* dir = find_dir(full);
  sp_mem_end_scratch(s);
  if (entry && entry->absent) return SP_ERR_SYS_NOT_FOUND;
  if (!dir) return SP_ERR_SYS_NOT_FOUND;
  if (dir->open) return dir->open;
  sp_for(o, active->count.dirs) {
    if (active->opened[o].dir == dir && dir->reopen) return dir->reopen;
  }
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
  s64 slot = slot_of(fd);
  if (active->opened[slot].dir->it_open) return active->opened[slot].dir->it_open;
  *out = sp_zero_s(sp_sys_dir_it_t);
  out->fd = fd;
  out->state = slot;
  return SP_OK;
}

static sp_err_t dir_it_read(sp_sys_dir_it_t* it, sp_mem_buffer_t* buf) {
  sim_open_t* slot = &active->opened[it->state];
  const sim_dir_t* dir = slot->dir;
  if (dir->read) return dir->read;

  u32 batch = dir->batch ? dir->batch : SIM_MAX_ENTRIES;
  u32 position = 0;
  buf->len = 0;
  sp_carr_for(dir->entries, e) {
    if (!dir->entries[e].name) break;
    if (active->gone[dir - active->dirs][e]) continue;
    if (position++ < slot->position) continue;
    if (buf->len == batch) break;
    slot->listed[buf->len++] = e;
  }
  slot->position += (u32)buf->len;
  return SP_OK;
}

static sp_err_t dir_it_parse(sp_sys_dir_it_t* it, sp_mem_buffer_t* buf, u64* cursor, sp_sys_dir_entry_t* out) {
  sim_open_t* slot = &active->opened[it->state];
  const sim_entry_t* entry = &slot->dir->entries[slot->listed[*cursor]];
  *cursor += 1;

  u32 len = (u32)sp_cstr_len(entry->name);
  sp_assert(len <= buf->capacity);
  sp_mem_copy(buf->data, entry->name, len);

  *out = sp_zero_s(sp_sys_dir_entry_t);
  out->name = sp_ptr_cast(const c8*, buf->data);
  out->len = len;
  out->kind = entry->kind;
  return SP_OK;
}

static sp_err_t dir_it_close(sp_sys_dir_it_t* it) {
  active->count.closes++;
  return SP_OK;
}

static sp_err_t get_link_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_str_t full = resolve(s.mem, fd, path, len);
  const sim_entry_t* entry = find_entry(full);
  const sim_dir_t* dir = find_dir(full);
  sp_mem_end_scratch(s);

  *st = sp_zero_s(sp_sys_file_meta_t);
  if (entry && entry->absent) return SP_ERR_SYS_NOT_FOUND;
  if (entry && entry->stat != SP_FS_KIND_NONE) {
    st->kind = entry->stat;
    return SP_OK;
  }
  if (dir) {
    st->kind = SP_FS_KIND_DIR;
    return SP_OK;
  }
  if (entry) return SP_OK;
  return SP_ERR_SYS_NOT_FOUND;
}

static sp_err_t unlink_entry(sp_sys_fd_t fd, const c8* path, u32 len) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_str_t full = resolve(s.mem, fd, path, len);
  const sim_entry_t* entry = find_entry(full);
  sp_err_t err = SP_ERR_SYS_NOT_FOUND;
  if (entry && !entry->absent) err = entry->unlink;
  if (entry && !err) record(full, SIM_OP_UNLINK);
  sp_mem_end_scratch(s);
  return err;
}

static sp_err_t remove_dir(sp_sys_fd_t fd, const c8* path, u32 len) {
  active->count.rmdirs++;
  sp_mem_arena_marker_t s = sp_mem_begin_scratch();
  sp_str_t full = resolve(s.mem, fd, path, len);
  const sim_dir_t* dir = find_dir(full);
  sp_err_t err = SP_ERR_SYS_NOT_FOUND;
  if (dir) err = dir->rmdir;
  if (dir && !err && !dir_empty(dir)) err = SP_ERR_SYS_NOT_EMPTY;
  if (dir && !err) record(full, SIM_OP_RMDIR);
  if (dir && err == SP_ERR_SYS_NOT_FOUND) mark_gone(full);
  sp_mem_end_scratch(s);
  return err;
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
