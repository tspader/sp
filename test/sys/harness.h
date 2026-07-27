#ifndef SYS_HARNESS_H
#define SYS_HARNESS_H

#define SP_PRIVATE_HEADER
#include "sp.h"
#include "sp/sp_test.h"

#define SYS_CASE_MAX_SETUP 8
#define SYS_CASE_MAX_STEPS 8
#define SYS_CASE_MAX_EXPECT 8
#define SYS_CASE_MAX_FDS 4
#define SYS_CASE_BUF_SIZE 64

typedef enum {
  SYS_SETUP_FILE,
  SYS_SETUP_DIR,
  SYS_SETUP_SYMLINK,
} sys_setup_kind_t;

typedef struct {
  const c8* path;
  sys_setup_kind_t kind;
  union {
    const c8* content;
    const c8* target;
  };
} sys_setup_t;

typedef enum {
  SYS_STEP_NONE,
  SYS_STEP_OPEN,
  SYS_STEP_OPEN_DIR,
  SYS_STEP_READ,
  SYS_STEP_WRITE,
} sys_step_kind_t;

typedef struct {
  sys_step_kind_t kind;
  union {
    struct { u32 slot; const c8* path; sp_sys_open_mode_t mode; u32 flags; sp_err_t err; } open;
    struct { u32 slot; const c8* path; sp_err_t err; } open_dir;
    struct { u32 slot; u64 count; const c8* expect; sp_err_t err; } read;
    struct { u32 slot; const c8* data; sp_err_t err; } write;
  };
} sys_step_t;

typedef struct {
  const c8* path;
  bool exists;
  const c8* content;
} sys_expect_t;

typedef struct {
  const c8* name;
  sys_setup_t setup [SYS_CASE_MAX_SETUP];
  sys_step_t steps [SYS_CASE_MAX_STEPS];
  sys_expect_t expect [SYS_CASE_MAX_EXPECT];
} sys_case_t;

static sp_test_once_t sys_symlink_probe = sp_zero;

sp_err_t sys_case_run(sp_test_t* t, sys_case_t* c);

#endif
