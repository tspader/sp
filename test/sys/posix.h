#ifndef SYS_POSIX_H
#define SYS_POSIX_H

#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_POSIX)

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static bool posix_probe_path(sp_test_t* t, sp_str_t name, c8* buf, u32 size) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t root = sp_zero;
  if (sp_fs_canonicalize_path_at(mem, sp_test_dir(t), &root)) return false;

  sp_str_t path = sp_fs_join_path(mem, root, name);
  if (path.len >= size) return false;
  sp_cstr_copy_to_n(path.data, path.len, buf, size);
  return true;
}

static bool posix_fifos_supported(sp_test_t* t) {
  c8 path [SP_PATH_MAX];
  if (!posix_probe_path(t, sp_str_lit("P"), path, sizeof(path))) return true;
  if (mkfifo(path, 0644)) return errno != EPERM && errno != ENOTSUP;
  unlink(path);
  return true;
}

static bool posix_modes_supported(sp_test_t* t) {
  c8 path [SP_PATH_MAX];
  if (!posix_probe_path(t, sp_str_lit("M"), path, sizeof(path))) return true;
  s32 fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
  if (fd < 0) return true;
  close(fd);

  struct stat st = sp_zero;
  bool stored = !chmod(path, 0500) && !stat(path, &st) && (st.st_mode & 0777) == 0500;
  unlink(path);
  return stored;
}

static bool posix_names_supported(sp_test_t* t) {
  c8 path [SP_PATH_MAX];
  if (!posix_probe_path(t, sp_str_lit("N\\:"), path, sizeof(path))) return true;
  if (mkdir(path, 0755)) return errno != EINVAL;
  rmdir(path);
  return true;
}

#define posix_skip_without_fifos() if (!posix_fifos_supported(t)) return sp_test_skip(t, "fifos not available");
#define posix_skip_without_modes() if (!posix_modes_supported(t)) return sp_test_skip(t, "file modes are not stored");
#define posix_skip_without_names() if (!posix_names_supported(t)) return sp_test_skip(t, "names cannot hold a backslash or colon");

#else

#define posix_skip_without_fifos() ((void)0)
#define posix_skip_without_modes() ((void)0)
#define posix_skip_without_names() ((void)0)

#endif

#endif
