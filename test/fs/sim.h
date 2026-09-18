#ifndef FS_SIM_H
#define FS_SIM_H

#include "sp.h"

#define SIM_MAX_DIRS 4
#define SIM_MAX_ENTRIES 4
#define SIM_MAX_OPENS 4

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
  sp_fs_kind_t stat;
} sim_entry_t;

typedef struct {
  const c8* path;
  sp_err_t from_fd;
  sp_err_t read;
  sim_entry_t entries [SIM_MAX_ENTRIES];
} sim_dir_t;

typedef struct {
  const sim_dir_t* dir;
  bool served;
} sim_open_t;

typedef struct {
  u32 opens;
  u32 dirs;
  u32 closes;
  u32 fd_closes;
} sim_count_t;

typedef struct {
  const sim_dir_t* dirs;
  sim_count_t count;
  sim_open_t opened [SIM_MAX_OPENS];
  sp_sys_vtable_t vt;
  const sp_sys_vtable_t* saved;
} sim_t;

void sim_begin(sim_t* sim, const sim_dir_t* dirs);
void sim_end(sim_t* sim);

#endif // FS_SIM_H
