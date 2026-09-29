#ifndef FS_SIM_H
#define FS_SIM_H

#include "sp.h"

#define SIM_MAX_DIRS 4
#define SIM_MAX_ENTRIES 4
#define SIM_MAX_OPENS 8
#define SIM_MAX_REMOVED 8
#define SIM_MAX_REMOVED_PATH 64

typedef enum {
  SIM_OP_UNLINK,
  SIM_OP_RMDIR,
} sim_op_t;

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
  sp_fs_kind_t stat;
  sp_err_t unlink;
  bool absent;
} sim_entry_t;

typedef struct {
  const c8* path;
  sp_err_t open;
  sp_err_t reopen;
  sp_err_t it_open;
  sp_err_t read;
  sp_err_t rmdir;
  u32 batch;
  sim_entry_t entries [SIM_MAX_ENTRIES];
} sim_dir_t;

typedef struct {
  const sim_dir_t* dir;
  u32 position;
  u32 listed [SIM_MAX_ENTRIES];
} sim_open_t;

typedef struct {
  u32 opens;
  u32 dirs;
  u32 closes;
  u32 fd_closes;
  u32 rmdirs;
  u32 nofollow;
} sim_count_t;

typedef struct {
  c8 path [SIM_MAX_REMOVED_PATH];
  sim_op_t op;
} sim_removed_t;

typedef struct {
  const sim_dir_t* dirs;
  sim_count_t count;
  sim_open_t opened [SIM_MAX_OPENS];
  sim_removed_t removed [SIM_MAX_REMOVED];
  u32 num_removed;
  bool gone [SIM_MAX_DIRS][SIM_MAX_ENTRIES];
  sp_sys_vtable_t vt;
  const sp_sys_vtable_t* saved;
} sim_t;

void sim_begin(sim_t* sim, const sim_dir_t* dirs);
void sim_end(sim_t* sim);

#endif // FS_SIM_H
