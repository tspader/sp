#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  s64 (*call)(void);
  s64 expect;
} test_t;

static s64 recorded;

static sp_err_t mock_read(sp_sys_fd_t fd, void* buf, u64 count, u64* bytes_read) {
  return (sp_err_t)69;
}

static sp_err_t mock_write(sp_sys_fd_t fd, const void* buf, u64 count, u64* bytes_written) {
  return (sp_err_t)69;
}

static sp_err_t mock_pread(sp_sys_fd_t fd, void* buf, u64 count, u64 offset, u64* bytes_read) {
  return (sp_err_t)69;
}

static sp_err_t mock_pwrite(sp_sys_fd_t fd, const void* buf, u64 count, u64 offset, u64* bytes_written) {
  return (sp_err_t)69;
}

static sp_err_t mock_transfer(sp_sys_fd_t in, u64* in_pos, sp_sys_fd_t out, u64 count, u64* bytes_moved) {
  return (sp_err_t)69;
}

static sp_err_t mock_transfer_positional(sp_sys_fd_t in, u64* in_pos, sp_sys_fd_t out, u64 count, u64 offset, u64* bytes_moved) {
  return (sp_err_t)69;
}

static sp_sys_fd_t mock_get_root(s32 it) {
  return 69;
}

static s64 mock_get_exe_path(c8* buf, u64 size) {
  return 69;
}

static s64 mock_get_cwd_path(c8* buf, u64 size) {
  return 69;
}

static s64 mock_get_storage_path(c8* buf, u64 size) {
  return 69;
}

static s64 mock_get_config_path(c8* buf, u64 size) {
  return 69;
}

static sp_err_t mock_open(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_open_mode_t mode, u32 flags, sp_sys_fd_t* out) {
  *out = 69;
  return (sp_err_t)69;
}

static sp_err_t mock_open_dir(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_fd_t* out) {
  *out = 69;
  return (sp_err_t)69;
}

static sp_err_t mock_close(sp_sys_fd_t fd) {
  return (sp_err_t)69;
}

static sp_err_t mock_pipe(sp_sys_pipe_t* pipe, sp_sys_pipe_desc_t desc) {
  return (sp_err_t)69;
}

static sp_err_t mock_pipe_ready(sp_sys_fd_t fd, u8* ready) {
  return (sp_err_t)69;
}

static sp_err_t mock_mkdir(sp_sys_fd_t fd, const c8* path, u32 len, s32 mode) {
  return (sp_err_t)69;
}

static sp_err_t mock_rmdir(sp_sys_fd_t fd, const c8* path, u32 len) {
  return (sp_err_t)69;
}

static sp_err_t mock_unlink(sp_sys_fd_t fd, const c8* path, u32 len) {
  return (sp_err_t)69;
}

static sp_err_t mock_rename(sp_sys_fd_t from_fd, const c8* from, u32 from_len, sp_sys_fd_t to_fd, const c8* to, u32 to_len) {
  return (sp_err_t)69;
}

static sp_err_t mock_link(sp_sys_fd_t from_fd, const c8* existing, u32 existing_len, sp_sys_fd_t to_fd, const c8* alias, u32 alias_len) {
  return (sp_err_t)69;
}

static sp_err_t mock_symlink(const c8* existing, u32 existing_len, sp_sys_fd_t to_fd, const c8* alias, u32 alias_len) {
  return (sp_err_t)69;
}

static sp_err_t mock_get_path_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t mock_get_link_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t mock_get_file_metadata(sp_sys_fd_t fd, sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t mock_chmod(sp_sys_fd_t fd, const c8* path, u32 len, const sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t mock_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  return (sp_err_t)69;
}

static sp_err_t mock_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  return (sp_err_t)69;
}

static bool mock_futex_wait(u32* addr, u32 expected, const sp_sys_timespec_t* timeout) {
  return true;
}

static void mock_futex_wake(u32* addr) {
  recorded = 69;
}

static void mock_futex_wake_all(u32* addr) {
  recorded = 69;
}

static s64 mock_canonicalize_path(const c8* path, u32 len, c8* buf, u64 size) {
  return 69;
}

static sp_err_t mock_event_open(sp_sys_event_t* out) {
  return (sp_err_t)69;
}

static sp_err_t mock_event_signal(sp_sys_event_t event) {
  return (sp_err_t)69;
}

static sp_err_t mock_event_clear(sp_sys_event_t event) {
  return (sp_err_t)69;
}

static sp_err_t mock_wait(const sp_sys_fd_t* fds, u64 n, u32 timeout_ms, u64* signaled) {
  return (sp_err_t)69;
}

static sp_err_t mock_tty_get(sp_sys_fd_t fd, sp_sys_tty_attr_t* attr) {
  return (sp_err_t)69;
}

static sp_err_t mock_tty_set(sp_sys_fd_t fd, const sp_sys_tty_attr_t* attr) {
  return (sp_err_t)69;
}

static sp_err_t mock_tty_size(sp_sys_fd_t fd, u32* cols, u32* rows) {
  return (sp_err_t)69;
}

static bool mock_is_tty(sp_sys_fd_t fd) {
  return true;
}

static sp_err_t mock_tty_ready(sp_sys_fd_t fd, u8* ready) {
  return (sp_err_t)69;
}

static sp_err_t mock_tty_mode_apply(sp_sys_tty_attr_t* in, sp_sys_tty_attr_t* out, sp_sys_tty_mode_t mode) {
  return (sp_err_t)69;
}

static sp_err_t mock_tty_use_vt(sp_sys_fd_t fd) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_open(sp_sys_socket_t* out, sp_sys_handle_desc_t desc) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_bind(sp_sys_socket_t socket, sp_sys_ipv4_t addr) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_listen(sp_sys_socket_t socket, u32 backlog) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_connect(sp_sys_socket_t socket, sp_sys_ipv4_t addr) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_error(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_accept(sp_sys_socket_t listener, sp_sys_handle_desc_t desc, sp_sys_socket_t* out) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_close(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_recv(sp_sys_socket_t socket, void* buf, u64 count, u64* bytes_read) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_send(sp_sys_socket_t socket, const void* buf, u64 count, u64* bytes_written) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_wait(sp_sys_socket_t socket, bool readable, u32 timeout_ms) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_set_nonblocking(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_reuse_addr(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_no_delay(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t mock_socket_local_port(sp_sys_socket_t socket, u16* out) {
  return (sp_err_t)69;
}

static void* mock_alloc(u64 size) {
  return (void*)(uintptr_t)69;
}

static void mock_free(void* ptr, u64 size) {
  recorded = 69;
}

static void* mock_memcpy(void* dest, const void* src, u64 n) {
  return (void*)(uintptr_t)69;
}

static void* mock_memmove(void* dest, const void* src, u64 n) {
  return (void*)(uintptr_t)69;
}

static void* mock_memset(void* dest, u8 fill, u64 n) {
  return (void*)(uintptr_t)69;
}

static s32 mock_memcmp(const void* a, const void* b, u64 n) {
  return 69;
}

static void mock_assert(bool cond) {
  recorded = 69;
}

static void mock_exit(s32 code) {
  recorded = 69;
}

static void mock_env(const c8** env, u32* len) {
  recorded = 69;
}

static s64 mock_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  return 69;
}

static sp_err_t mock_chdir(const c8* path, u32 len) {
  return (sp_err_t)69;
}

static sp_err_t mock_dir_from_fd(sp_sys_fd_t fd, sp_sys_dir_t* out) {
  return (sp_err_t)69;
}

static sp_err_t mock_dir_read(sp_sys_dir_t* dir, sp_mem_buffer_t* buf) {
  return (sp_err_t)69;
}

static sp_err_t mock_dir_parse(sp_sys_dir_t* dir, sp_mem_buffer_t* buf, u64* cursor, sp_sys_dir_entry_t* out) {
  return (sp_err_t)69;
}

static sp_err_t mock_dir_close(sp_sys_dir_t* dir) {
  return (sp_err_t)69;
}

static const sp_sys_vtable_t mock = {
  .read                   = mock_read,
  .write                  = mock_write,
  .pread                  = mock_pread,
  .pwrite                 = mock_pwrite,
  .transfer               = mock_transfer,
  .transfer_positional    = mock_transfer_positional,
  .get_root               = mock_get_root,
  .get_exe_path           = mock_get_exe_path,
  .get_cwd_path           = mock_get_cwd_path,
  .get_storage_path       = mock_get_storage_path,
  .get_config_path        = mock_get_config_path,
  .open                   = mock_open,
  .open_dir               = mock_open_dir,
  .close                  = mock_close,
  .pipe                   = mock_pipe,
  .pipe_ready             = mock_pipe_ready,
  .mkdir                  = mock_mkdir,
  .rmdir                  = mock_rmdir,
  .unlink                 = mock_unlink,
  .rename                 = mock_rename,
  .link                   = mock_link,
  .symlink                = mock_symlink,
  .get_path_metadata      = mock_get_path_metadata,
  .get_link_metadata      = mock_get_link_metadata,
  .get_file_metadata      = mock_get_file_metadata,
  .chmod                  = mock_chmod,
  .clock_gettime          = mock_clock_gettime,
  .nanosleep              = mock_nanosleep,
  .futex_wait             = mock_futex_wait,
  .futex_wake             = mock_futex_wake,
  .futex_wake_all         = mock_futex_wake_all,
  .canonicalize_path      = mock_canonicalize_path,
  .event_open             = mock_event_open,
  .event_signal           = mock_event_signal,
  .event_clear            = mock_event_clear,
  .wait                   = mock_wait,
  .tty_get                = mock_tty_get,
  .tty_set                = mock_tty_set,
  .tty_size               = mock_tty_size,
  .is_tty                 = mock_is_tty,
  .tty_ready              = mock_tty_ready,
  .tty_mode_apply         = mock_tty_mode_apply,
  .tty_use_vt             = mock_tty_use_vt,
  .socket_open            = mock_socket_open,
  .socket_bind            = mock_socket_bind,
  .socket_listen          = mock_socket_listen,
  .socket_connect         = mock_socket_connect,
  .socket_error           = mock_socket_error,
  .socket_accept          = mock_socket_accept,
  .socket_close           = mock_socket_close,
  .socket_recv            = mock_socket_recv,
  .socket_send            = mock_socket_send,
  .socket_wait            = mock_socket_wait,
  .socket_set_nonblocking = mock_socket_set_nonblocking,
  .socket_reuse_addr      = mock_socket_reuse_addr,
  .socket_no_delay        = mock_socket_no_delay,
  .socket_local_port      = mock_socket_local_port,
  .alloc                  = mock_alloc,
  .free                   = mock_free,
  .memcpy                 = mock_memcpy,
  .memmove                = mock_memmove,
  .memset                 = mock_memset,
  .memcmp                 = mock_memcmp,
  .assert                 = mock_assert,
  .exit                   = mock_exit,
  .env                    = mock_env,
  .lseek                  = mock_lseek,
  .chdir                  = mock_chdir,
  .dir_from_fd            = mock_dir_from_fd,
  .dir_read               = mock_dir_read,
  .dir_parse              = mock_dir_parse,
  .dir_close              = mock_dir_close,
};

static s64 call_read(void) {
  return (s64)sp_sys_read(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_write(void) {
  return (s64)sp_sys_write(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_pread(void) {
  return (s64)sp_sys_pread(0, SP_NULLPTR, 0, 0, SP_NULLPTR);
}

static s64 call_pwrite(void) {
  return (s64)sp_sys_pwrite(0, SP_NULLPTR, 0, 0, SP_NULLPTR);
}

static s64 call_transfer(void) {
  return (s64)sp_sys_transfer(0, SP_NULLPTR, 0, 0, SP_NULLPTR);
}

static s64 call_transfer_positional(void) {
  return (s64)sp_sys_transfer_positional(0, SP_NULLPTR, 0, 0, 0, SP_NULLPTR);
}

static s64 call_get_root(void) {
  return (s64)sp_sys_get_root(0);
}

static s64 call_get_exe_path(void) {
  return sp_sys_get_exe_path(SP_NULLPTR, 0);
}

static s64 call_get_cwd_path(void) {
  return sp_sys_get_cwd_path(SP_NULLPTR, 0);
}

static s64 call_get_storage_path(void) {
  return sp_sys_get_storage_path(SP_NULLPTR, 0);
}

static s64 call_get_config_path(void) {
  return sp_sys_get_config_path(SP_NULLPTR, 0);
}

static s64 call_open(void) {
  sp_sys_fd_t out = SP_SYS_INVALID_FD;
  sp_sys_open(0, SP_NULLPTR, 0, SP_SYS_OPEN_MODE_RO, 0, &out);
  return (s64)out;
}

static s64 call_open_dir(void) {
  sp_sys_fd_t out = SP_SYS_INVALID_FD;
  sp_sys_open_dir(0, SP_NULLPTR, 0, &out);
  return (s64)out;
}

static s64 call_close(void) {
  return (s64)sp_sys_close(0);
}

static s64 call_pipe(void) {
  return (s64)sp_sys_pipe(SP_NULLPTR, sp_zero_s(sp_sys_pipe_desc_t));
}

static s64 call_pipe_ready(void) {
  u8 ready = 0;
  return (s64)sp_sys_pipe_ready(0, &ready);
}

static s64 call_mkdir(void) {
  return (s64)sp_sys_mkdir(0, SP_NULLPTR, 0, 0);
}

static s64 call_rmdir(void) {
  return (s64)sp_sys_rmdir(0, SP_NULLPTR, 0);
}

static s64 call_unlink(void) {
  return (s64)sp_sys_unlink(0, SP_NULLPTR, 0);
}

static s64 call_rename(void) {
  return (s64)sp_sys_rename(0, SP_NULLPTR, 0, 0, SP_NULLPTR, 0);
}

static s64 call_link(void) {
  return (s64)sp_sys_link(0, SP_NULLPTR, 0, 0, SP_NULLPTR, 0);
}

static s64 call_symlink(void) {
  return (s64)sp_sys_symlink(SP_NULLPTR, 0, 0, SP_NULLPTR, 0);
}

static s64 call_get_path_metadata(void) {
  return (s64)sp_sys_get_path_metadata(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_get_link_metadata(void) {
  return (s64)sp_sys_get_link_metadata(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_get_file_metadata(void) {
  return (s64)sp_sys_get_file_metadata(0, SP_NULLPTR);
}

static s64 call_chmod(void) {
  return (s64)sp_sys_chmod(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_clock_gettime(void) {
  return (s64)sp_sys_clock_gettime(0, SP_NULLPTR);
}

static s64 call_nanosleep(void) {
  return (s64)sp_sys_nanosleep(SP_NULLPTR, SP_NULLPTR);
}

static s64 call_futex_wait(void) {
  return (s64)sp_sys_futex_wait(SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_futex_wake(void) {
  recorded = 0;
  sp_sys_futex_wake(SP_NULLPTR);
  return recorded;
}

static s64 call_futex_wake_all(void) {
  recorded = 0;
  sp_sys_futex_wake_all(SP_NULLPTR);
  return recorded;
}

static s64 call_canonicalize_path(void) {
  return sp_sys_canonicalize_path(SP_NULLPTR, 0, SP_NULLPTR, 0);
}

static s64 call_event_open(void) {
  return (s64)sp_sys_event_open(SP_NULLPTR);
}

static s64 call_event_signal(void) {
  return (s64)sp_sys_event_signal(sp_zero_s(sp_sys_event_t));
}

static s64 call_event_clear(void) {
  return (s64)sp_sys_event_clear(sp_zero_s(sp_sys_event_t));
}

static s64 call_wait(void) {
  sp_sys_fd_t fds [1] = sp_zero;
  return (s64)sp_sys_wait(fds, 0, 0, SP_NULLPTR);
}

static s64 call_tty_get(void) {
  return (s64)sp_sys_tty_get(0, SP_NULLPTR);
}

static s64 call_tty_set(void) {
  return (s64)sp_sys_tty_set(0, SP_NULLPTR);
}

static s64 call_tty_size(void) {
  return (s64)sp_sys_tty_size(0, SP_NULLPTR, SP_NULLPTR);
}

static s64 call_is_tty(void) {
  return (s64)sp_sys_is_tty(0);
}

static s64 call_tty_ready(void) {
  u8 ready = 0;
  return (s64)sp_sys_tty_ready(0, &ready);
}

static s64 call_tty_mode_apply(void) {
  return (s64)sp_sys_tty_mode_apply(SP_NULLPTR, SP_NULLPTR, SP_SYS_TTY_MODE_RAW);
}

static s64 call_tty_use_vt(void) {
  return (s64)sp_sys_tty_use_vt(0);
}

static s64 call_socket_open(void) {
  return (s64)sp_sys_socket_open(SP_NULLPTR, sp_zero_s(sp_sys_handle_desc_t));
}

static s64 call_socket_bind(void) {
  return (s64)sp_sys_socket_bind(0, sp_zero_s(sp_sys_ipv4_t));
}

static s64 call_socket_listen(void) {
  return (s64)sp_sys_socket_listen(0, 0);
}

static s64 call_socket_connect(void) {
  return (s64)sp_sys_socket_connect(0, sp_zero_s(sp_sys_ipv4_t));
}

static s64 call_socket_error(void) {
  return (s64)sp_sys_socket_error(0);
}

static s64 call_socket_accept(void) {
  return (s64)sp_sys_socket_accept(0, sp_zero_s(sp_sys_handle_desc_t), SP_NULLPTR);
}

static s64 call_socket_close(void) {
  return (s64)sp_sys_socket_close(0);
}

static s64 call_socket_recv(void) {
  return (s64)sp_sys_socket_recv(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_socket_send(void) {
  return (s64)sp_sys_socket_send(0, SP_NULLPTR, 0, SP_NULLPTR);
}

static s64 call_socket_wait(void) {
  return (s64)sp_sys_socket_wait(0, false, 0);
}

static s64 call_socket_set_nonblocking(void) {
  return (s64)sp_sys_socket_set_nonblocking(0);
}

static s64 call_socket_reuse_addr(void) {
  return (s64)sp_sys_socket_reuse_addr(0);
}

static s64 call_socket_no_delay(void) {
  return (s64)sp_sys_socket_no_delay(0);
}

static s64 call_socket_local_port(void) {
  return (s64)sp_sys_socket_local_port(0, SP_NULLPTR);
}

static s64 call_alloc(void) {
  return (s64)(uintptr_t)sp_sys_alloc(0);
}

static s64 call_free(void) {
  recorded = 0;
  sp_sys_free(SP_NULLPTR, 0);
  return recorded;
}

static s64 call_memcpy(void) {
  return (s64)(uintptr_t)sp_sys_memcpy(SP_NULLPTR, SP_NULLPTR, 0);
}

static s64 call_memmove(void) {
  return (s64)(uintptr_t)sp_sys_memmove(SP_NULLPTR, SP_NULLPTR, 0);
}

static s64 call_memset(void) {
  return (s64)(uintptr_t)sp_sys_memset(SP_NULLPTR, 0, 0);
}

static s64 call_memcmp(void) {
  return (s64)sp_sys_memcmp(SP_NULLPTR, SP_NULLPTR, 0);
}

static s64 call_assert(void) {
  recorded = 0;
  sp_sys_assert(true);
  return recorded;
}

static s64 call_exit(void) {
  recorded = 0;
  sp_sys_exit(0);
  return recorded;
}

static s64 call_env(void) {
  recorded = 0;
  sp_sys_env(SP_NULLPTR, SP_NULLPTR);
  return recorded;
}

static s64 call_lseek(void) {
  return sp_sys_lseek(0, 0, 0);
}

static s64 call_chdir(void) {
  return (s64)sp_sys_chdir(SP_NULLPTR, 0);
}

static s64 call_dir_from_fd(void) {
  return (s64)sp_sys_dir_from_fd(0, SP_NULLPTR);
}

static s64 call_dir_read(void) {
  return (s64)sp_sys_dir_read(SP_NULLPTR, SP_NULLPTR);
}

static s64 call_dir_parse(void) {
  return (s64)sp_sys_dir_parse(SP_NULLPTR, SP_NULLPTR, SP_NULLPTR, SP_NULLPTR);
}

static s64 call_dir_close(void) {
  return (s64)sp_sys_dir_close(SP_NULLPTR);
}

static const test_t tests [] = {
  { "read", call_read, 69 },
  { "write", call_write, 69 },
  { "pread", call_pread, 69 },
  { "pwrite", call_pwrite, 69 },
  { "transfer", call_transfer, 69 },
  { "transfer_positional", call_transfer_positional, 69 },
  { "get_root", call_get_root, 69 },
  { "get_exe_path", call_get_exe_path, 69 },
  { "get_cwd_path", call_get_cwd_path, 69 },
  { "get_storage_path", call_get_storage_path, 69 },
  { "get_config_path", call_get_config_path, 69 },
  { "open", call_open, 69 },
  { "open_dir", call_open_dir, 69 },
  { "close", call_close, 69 },
  { "pipe", call_pipe, 69 },
  { "pipe_ready", call_pipe_ready, 69 },
  { "mkdir", call_mkdir, 69 },
  { "rmdir", call_rmdir, 69 },
  { "unlink", call_unlink, 69 },
  { "rename", call_rename, 69 },
  { "link", call_link, 69 },
  { "symlink", call_symlink, 69 },
  { "get_path_metadata", call_get_path_metadata, 69 },
  { "get_link_metadata", call_get_link_metadata, 69 },
  { "get_file_metadata", call_get_file_metadata, 69 },
  { "chmod", call_chmod, 69 },
  { "clock_gettime", call_clock_gettime, 69 },
  { "nanosleep", call_nanosleep, 69 },
  { "futex_wait", call_futex_wait, 1 },
  { "futex_wake", call_futex_wake, 69 },
  { "futex_wake_all", call_futex_wake_all, 69 },
  { "canonicalize_path", call_canonicalize_path, 69 },
  { "event_open", call_event_open, 69 },
  { "event_signal", call_event_signal, 69 },
  { "event_clear", call_event_clear, 69 },
  { "wait", call_wait, 69 },
  { "tty_get", call_tty_get, 69 },
  { "tty_set", call_tty_set, 69 },
  { "tty_size", call_tty_size, 69 },
  { "is_tty", call_is_tty, 1 },
  { "tty_ready", call_tty_ready, 69 },
  { "tty_mode_apply", call_tty_mode_apply, 69 },
  { "tty_use_vt", call_tty_use_vt, 69 },
  { "socket_open", call_socket_open, 69 },
  { "socket_bind", call_socket_bind, 69 },
  { "socket_listen", call_socket_listen, 69 },
  { "socket_connect", call_socket_connect, 69 },
  { "socket_error", call_socket_error, 69 },
  { "socket_accept", call_socket_accept, 69 },
  { "socket_close", call_socket_close, 69 },
  { "socket_recv", call_socket_recv, 69 },
  { "socket_send", call_socket_send, 69 },
  { "socket_wait", call_socket_wait, 69 },
  { "socket_set_nonblocking", call_socket_set_nonblocking, 69 },
  { "socket_reuse_addr", call_socket_reuse_addr, 69 },
  { "socket_no_delay", call_socket_no_delay, 69 },
  { "socket_local_port", call_socket_local_port, 69 },
  { "alloc", call_alloc, 69 },
  { "free", call_free, 69 },
  { "memcpy", call_memcpy, 69 },
  { "memmove", call_memmove, 69 },
  { "memset", call_memset, 69 },
  { "memcmp", call_memcmp, 69 },
  { "assert", call_assert, 69 },
  { "exit", call_exit, 69 },
  { "env", call_env, 69 },
  { "lseek", call_lseek, 69 },
  { "chdir", call_chdir, 69 },
  { "dir_from_fd", call_dir_from_fd, 69 },
  { "dir_read", call_dir_read, 69 },
  { "dir_parse", call_dir_parse, 69 },
  { "dir_close", call_dir_close, 69 },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  const sp_sys_vtable_t* old = sp_sys_set_vtable(&mock);
  s64 got = c->call();
  const sp_sys_vtable_t* swapped = sp_sys_set_vtable(old);

  sp_expect(t, old == &sp_sys_vtable_platform);
  sp_expect(t, swapped == &mock);
  sp_expect_eq(t, got, c->expect);
  return SP_OK;
}

sp_test_each_fn(sys, vtable, test_t, tests, run, .serial = true);
