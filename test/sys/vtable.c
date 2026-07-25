#include "test.h"
#include "utest.h"

UTEST_EMPTY_FIXTURE(sys_vtable)

typedef struct {
  s32         init;
  s64         read;
  s64         write;
  s64         pread;
  s64         pwrite;
  s64         transfer;
  sp_sys_fd_t get_root;
  s64         get_exe_path;
  s64         get_cwd_path;
  s64         get_storage_path;
  s64         get_config_path;
  sp_sys_fd_t open;
  sp_sys_fd_t open_dir;
  s32         close;
  s32         pipe;
  s32         mkdir;
  s32         rmdir;
  s32         unlink;
  s32         rename;
  s32         link;
  s32         symlink;
  s32         get_path_metadata;
  s32         get_link_metadata;
  s32         get_file_metadata;
  s32         chmod;
  s32         clock_gettime;
  s32         nanosleep;
  s64         canonicalize_path;
  s32         fd_ready;
  s32         fd_wait;
  s32         fds_wait;
  s32         socket_open;
  s32         socket_bind;
  s32         socket_listen;
  s32         socket_connect;
  s32         socket_error;
  s32         socket_accept;
  s32         socket_close;
  s64         socket_recv;
  s64         socket_send;
  s32         socket_wait;
  s32         socket_set_nonblocking;
  s32         socket_reuse_addr;
  s32         socket_local_port;
  void*       alloc;
  s32         free;
  void*       memcpy;
  void*       memmove;
  void*       memset;
  s32         memcmp;
  s32         assert;
  s32         exit;
  s32         env;
  s64         lseek;
  s32         chdir;
  s32         fs_it_open;
  s32         fs_it_next;
  s32         fs_it_close;
} sys_vtable_results_t;

static sys_vtable_results_t sys_vtable_results;

static void sys_vtable_mock_init(void) {
  sys_vtable_results.init = 69;
}

static sp_err_t sys_vtable_mock_read(sp_sys_fd_t fd, void* buf, u64 count, u64* bytes_read) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_write(sp_sys_fd_t fd, const void* buf, u64 count, u64* bytes_written) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_pread(sp_sys_fd_t fd, void* buf, u64 count, u64 offset, u64* bytes_read) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_pwrite(sp_sys_fd_t fd, const void* buf, u64 count, u64 offset, u64* bytes_written) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_transfer(sp_sys_fd_t in, u64* in_pos, sp_sys_fd_t out, u64* out_pos, u64 count, u64* bytes_moved) {
  return (sp_err_t)69;
}

static sp_sys_fd_t sys_vtable_mock_get_root(s32 it) {
  return 69;
}

static s64 sys_vtable_mock_get_exe_path(c8* buf, u64 size) {
  return 69;
}

static s64 sys_vtable_mock_get_cwd_path(c8* buf, u64 size) {
  return 69;
}

static s64 sys_vtable_mock_get_storage_path(c8* buf, u64 size) {
  return 69;
}

static s64 sys_vtable_mock_get_config_path(c8* buf, u64 size) {
  return 69;
}

static sp_err_t sys_vtable_mock_open(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_open_mode_t mode, u32 flags, sp_sys_fd_t* out) {
  *out = 69;
  return (sp_err_t)69;
}

static sp_sys_fd_t sys_vtable_mock_open_dir(sp_sys_fd_t fd, const c8* path, u32 len) {
  return 69;
}

static sp_err_t sys_vtable_mock_close(sp_sys_fd_t fd) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_pipe(sp_sys_fd_t* read_end, sp_sys_fd_t* write_end) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_mkdir(sp_sys_fd_t fd, const c8* path, u32 len, s32 mode) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_rmdir(sp_sys_fd_t fd, const c8* path, u32 len) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_unlink(sp_sys_fd_t fd, const c8* path, u32 len) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_rename(sp_sys_fd_t from_fd, const c8* from, u32 from_len, sp_sys_fd_t to_fd, const c8* to, u32 to_len) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_link(sp_sys_fd_t from_fd, const c8* existing, u32 existing_len, sp_sys_fd_t to_fd, const c8* alias, u32 alias_len) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_symlink(const c8* existing, u32 existing_len, sp_sys_fd_t to_fd, const c8* alias, u32 alias_len) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_get_path_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_get_link_metadata(sp_sys_fd_t fd, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_get_file_metadata(sp_sys_fd_t fd, sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_chmod(sp_sys_fd_t fd, const c8* path, u32 len, const sp_sys_file_meta_t* st) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_clock_gettime(s32 clockid, sp_sys_timespec_t* ts) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_nanosleep(const sp_sys_timespec_t* req, sp_sys_timespec_t* rem) {
  return (sp_err_t)69;
}

static s64 sys_vtable_mock_canonicalize_path(const c8* path, u32 len, c8* buf, u64 size) {
  return 69;
}

static sp_err_t sys_vtable_mock_fd_ready(sp_sys_fd_t fd, u8* ready) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_fd_wait(sp_sys_fd_t fd) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_fds_wait(const sp_sys_fd_t* fds, u8* ready, u64 nfds) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_open(sp_sys_socket_t* out) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_bind(sp_sys_socket_t socket, sp_sys_ipv4_t addr) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_listen(sp_sys_socket_t socket, u32 backlog) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_connect(sp_sys_socket_t socket, sp_sys_ipv4_t addr) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_error(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_accept(sp_sys_socket_t listener, sp_sys_socket_t* out) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_close(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_recv(sp_sys_socket_t socket, void* buf, u64 count, u64* bytes_read) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_send(sp_sys_socket_t socket, const void* buf, u64 count, u64* bytes_written) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_wait(sp_sys_socket_t socket, bool readable, u32 timeout_ms) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_set_nonblocking(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_reuse_addr(sp_sys_socket_t socket) {
  return (sp_err_t)69;
}

static sp_err_t sys_vtable_mock_socket_local_port(sp_sys_socket_t socket, u16* out) {
  return (sp_err_t)69;
}

static void* sys_vtable_mock_alloc(u64 size) {
  return (void*)(uintptr_t)69;
}

static void sys_vtable_mock_free(void* ptr, u64 size) {
  sys_vtable_results.free = 69;
}

static void* sys_vtable_mock_memcpy(void* dest, const void* src, u64 n) {
  return (void*)(uintptr_t)69;
}

static void* sys_vtable_mock_memmove(void* dest, const void* src, u64 n) {
  return (void*)(uintptr_t)69;
}

static void* sys_vtable_mock_memset(void* dest, u8 fill, u64 n) {
  return (void*)(uintptr_t)69;
}

static s32 sys_vtable_mock_memcmp(const void* a, const void* b, u64 n) {
  return 69;
}

static void sys_vtable_mock_assert(bool cond) {
  sys_vtable_results.assert = 69;
}

static void sys_vtable_mock_exit(s32 code) {
  sys_vtable_results.exit = 69;
}

static void sys_vtable_mock_env(const c8** env, u32* len) {
  sys_vtable_results.env = 69;
}

static s64 sys_vtable_mock_lseek(sp_sys_fd_t fd, s64 offset, s32 whence) {
  return 69;
}

static sp_err_t sys_vtable_mock_chdir(const c8* path, u32 len) {
  return (sp_err_t)69;
}

static s32 sys_vtable_mock_fs_it_open(sp_sys_fd_t fd, sp_sys_fs_it_t* it, const c8* path, u32 path_len, void* buf, u64 cap) {
  return 69;
}

static s32 sys_vtable_mock_fs_it_next(sp_sys_fs_it_t* it, sp_sys_fs_entry_t* out) {
  return 69;
}

static void sys_vtable_mock_fs_it_close(sp_sys_fs_it_t* it) {
  sys_vtable_results.fs_it_close = 69;
}

static const sp_sys_vtable_t sys_vtable_mock = {
  .init                   = sys_vtable_mock_init,
  .read                   = sys_vtable_mock_read,
  .write                  = sys_vtable_mock_write,
  .pread                  = sys_vtable_mock_pread,
  .pwrite                 = sys_vtable_mock_pwrite,
  .transfer               = sys_vtable_mock_transfer,
  .get_root               = sys_vtable_mock_get_root,
  .get_exe_path           = sys_vtable_mock_get_exe_path,
  .get_cwd_path           = sys_vtable_mock_get_cwd_path,
  .get_storage_path       = sys_vtable_mock_get_storage_path,
  .get_config_path        = sys_vtable_mock_get_config_path,
  .open                   = sys_vtable_mock_open,
  .open_dir               = sys_vtable_mock_open_dir,
  .close                  = sys_vtable_mock_close,
  .pipe                   = sys_vtable_mock_pipe,
  .mkdir                  = sys_vtable_mock_mkdir,
  .rmdir                  = sys_vtable_mock_rmdir,
  .unlink                 = sys_vtable_mock_unlink,
  .rename                 = sys_vtable_mock_rename,
  .link                   = sys_vtable_mock_link,
  .symlink                = sys_vtable_mock_symlink,
  .get_path_metadata      = sys_vtable_mock_get_path_metadata,
  .get_link_metadata      = sys_vtable_mock_get_link_metadata,
  .get_file_metadata      = sys_vtable_mock_get_file_metadata,
  .chmod                  = sys_vtable_mock_chmod,
  .clock_gettime          = sys_vtable_mock_clock_gettime,
  .nanosleep              = sys_vtable_mock_nanosleep,
  .canonicalize_path      = sys_vtable_mock_canonicalize_path,
  .fd_ready               = sys_vtable_mock_fd_ready,
  .fd_wait                = sys_vtable_mock_fd_wait,
  .fds_wait               = sys_vtable_mock_fds_wait,
  .socket_open            = sys_vtable_mock_socket_open,
  .socket_bind            = sys_vtable_mock_socket_bind,
  .socket_listen          = sys_vtable_mock_socket_listen,
  .socket_connect         = sys_vtable_mock_socket_connect,
  .socket_error           = sys_vtable_mock_socket_error,
  .socket_accept          = sys_vtable_mock_socket_accept,
  .socket_close           = sys_vtable_mock_socket_close,
  .socket_recv            = sys_vtable_mock_socket_recv,
  .socket_send            = sys_vtable_mock_socket_send,
  .socket_wait            = sys_vtable_mock_socket_wait,
  .socket_set_nonblocking = sys_vtable_mock_socket_set_nonblocking,
  .socket_reuse_addr      = sys_vtable_mock_socket_reuse_addr,
  .socket_local_port      = sys_vtable_mock_socket_local_port,
  .alloc                  = sys_vtable_mock_alloc,
  .free                   = sys_vtable_mock_free,
  .memcpy                 = sys_vtable_mock_memcpy,
  .memmove                = sys_vtable_mock_memmove,
  .memset                 = sys_vtable_mock_memset,
  .memcmp                 = sys_vtable_mock_memcmp,
  .assert                 = sys_vtable_mock_assert,
  .exit                   = sys_vtable_mock_exit,
  .env                    = sys_vtable_mock_env,
  .lseek                  = sys_vtable_mock_lseek,
  .chdir                  = sys_vtable_mock_chdir,
  .fs_it_open             = sys_vtable_mock_fs_it_open,
  .fs_it_next             = sys_vtable_mock_fs_it_next,
  .fs_it_close            = sys_vtable_mock_fs_it_close,
};

UTEST_F(sys_vtable, every_function_dispatches) {
  sys_vtable_results = sp_zero_s(sys_vtable_results_t);
  sys_vtable_results_t* r = &sys_vtable_results;
  sp_sys_ipv4_t addr = sp_zero;
  sp_sys_fd_t fds [1] = sp_zero;
  u8 ready = 0;

  const sp_sys_vtable_t* old = sp_sys_set_vtable(&sys_vtable_mock);

  r->read = sp_sys_read(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->write = sp_sys_write(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->pread = sp_sys_pread(0, SP_NULLPTR, 0, 0, SP_NULLPTR);
  r->pwrite = sp_sys_pwrite(0, SP_NULLPTR, 0, 0, SP_NULLPTR);
  r->transfer = sp_sys_transfer(0, SP_NULLPTR, 0, SP_NULLPTR, 0, SP_NULLPTR);
  r->get_root = sp_sys_get_root(0);
  r->get_exe_path = sp_sys_get_exe_path(SP_NULLPTR, 0);
  r->get_cwd_path = sp_sys_get_cwd_path(SP_NULLPTR, 0);
  r->get_storage_path = sp_sys_get_storage_path(SP_NULLPTR, 0);
  r->get_config_path = sp_sys_get_config_path(SP_NULLPTR, 0);
  {
    sp_sys_fd_t opened = SP_SYS_INVALID_FD;
    sp_sys_open(0, SP_NULLPTR, 0, SP_SYS_OPEN_MODE_RO, 0, &opened);
    r->open = opened;
  }
  r->open_dir = sp_sys_open_dir(0, SP_NULLPTR, 0);
  r->close = sp_sys_close(0);
  r->pipe = sp_sys_pipe(SP_NULLPTR, SP_NULLPTR);
  r->mkdir = sp_sys_mkdir(0, SP_NULLPTR, 0, 0);
  r->rmdir = sp_sys_rmdir(0, SP_NULLPTR, 0);
  r->unlink = sp_sys_unlink(0, SP_NULLPTR, 0);
  r->rename = sp_sys_rename(0, SP_NULLPTR, 0, 0, SP_NULLPTR, 0);
  r->link = sp_sys_link(0, SP_NULLPTR, 0, 0, SP_NULLPTR, 0);
  r->symlink = sp_sys_symlink(SP_NULLPTR, 0, 0, SP_NULLPTR, 0);
  r->get_path_metadata = sp_sys_get_path_metadata(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->get_link_metadata = sp_sys_get_link_metadata(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->get_file_metadata = sp_sys_get_file_metadata(0, SP_NULLPTR);
  r->chmod = sp_sys_chmod(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->clock_gettime = sp_sys_clock_gettime(0, SP_NULLPTR);
  r->nanosleep = sp_sys_nanosleep(SP_NULLPTR, SP_NULLPTR);
  r->canonicalize_path = sp_sys_canonicalize_path(SP_NULLPTR, 0, SP_NULLPTR, 0);
  r->fd_ready = sp_sys_fd_ready(0, &ready);
  r->fd_wait = sp_sys_fd_wait(0);
  r->fds_wait = sp_sys_fds_wait(fds, &ready, 0);
  r->socket_open = sp_sys_socket_open(SP_NULLPTR);
  r->socket_bind = sp_sys_socket_bind(0, addr);
  r->socket_listen = sp_sys_socket_listen(0, 0);
  r->socket_connect = sp_sys_socket_connect(0, addr);
  r->socket_error = sp_sys_socket_error(0);
  r->socket_accept = sp_sys_socket_accept(0, SP_NULLPTR);
  r->socket_close = sp_sys_socket_close(0);
  r->socket_recv = sp_sys_socket_recv(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->socket_send = sp_sys_socket_send(0, SP_NULLPTR, 0, SP_NULLPTR);
  r->socket_wait = sp_sys_socket_wait(0, false, 0);
  r->socket_set_nonblocking = sp_sys_socket_set_nonblocking(0);
  r->socket_reuse_addr = sp_sys_socket_reuse_addr(0);
  r->socket_local_port = sp_sys_socket_local_port(0, SP_NULLPTR);
  r->alloc = sp_sys_alloc(0);
  sp_sys_free(SP_NULLPTR, 0);
  r->memcpy = sp_sys_memcpy(SP_NULLPTR, SP_NULLPTR, 0);
  r->memmove = sp_sys_memmove(SP_NULLPTR, SP_NULLPTR, 0);
  r->memset = sp_sys_memset(SP_NULLPTR, 0, 0);
  r->memcmp = sp_sys_memcmp(SP_NULLPTR, SP_NULLPTR, 0);
  sp_sys_assert(true);
  sp_sys_exit(0);
  sp_sys_env(SP_NULLPTR, SP_NULLPTR);
  r->lseek = sp_sys_lseek(0, 0, 0);
  r->chdir = sp_sys_chdir(SP_NULLPTR, 0);
  r->fs_it_open = sp_sys_fs_it_open(0, SP_NULLPTR, SP_NULLPTR, 0, SP_NULLPTR, 0);
  r->fs_it_next = sp_sys_fs_it_next(SP_NULLPTR, SP_NULLPTR);
  sp_sys_fs_it_close(SP_NULLPTR);

  const sp_sys_vtable_t* swapped = sp_sys_set_vtable(old);

  EXPECT_TRUE(old == &sp_sys_vtable_platform);
  EXPECT_TRUE(swapped == &sys_vtable_mock);
  EXPECT_EQ(r->init, 69);
  EXPECT_EQ(r->read, 69);
  EXPECT_EQ(r->write, 69);
  EXPECT_EQ(r->pread, 69);
  EXPECT_EQ(r->pwrite, 69);
  EXPECT_EQ(r->transfer, 69);
  EXPECT_EQ(r->get_root, 69);
  EXPECT_EQ(r->get_exe_path, 69);
  EXPECT_EQ(r->get_cwd_path, 69);
  EXPECT_EQ(r->get_storage_path, 69);
  EXPECT_EQ(r->get_config_path, 69);
  EXPECT_EQ(r->open, 69);
  EXPECT_EQ(r->open_dir, 69);
  EXPECT_EQ(r->close, 69);
  EXPECT_EQ(r->pipe, 69);
  EXPECT_EQ(r->mkdir, 69);
  EXPECT_EQ(r->rmdir, 69);
  EXPECT_EQ(r->unlink, 69);
  EXPECT_EQ(r->rename, 69);
  EXPECT_EQ(r->link, 69);
  EXPECT_EQ(r->symlink, 69);
  EXPECT_EQ(r->get_path_metadata, 69);
  EXPECT_EQ(r->get_link_metadata, 69);
  EXPECT_EQ(r->get_file_metadata, 69);
  EXPECT_EQ(r->chmod, 69);
  EXPECT_EQ(r->clock_gettime, 69);
  EXPECT_EQ(r->nanosleep, 69);
  EXPECT_EQ(r->canonicalize_path, 69);
  EXPECT_EQ(r->fd_ready, 69);
  EXPECT_EQ(r->fd_wait, 69);
  EXPECT_EQ(r->fds_wait, 69);
  EXPECT_EQ(r->socket_open, 69);
  EXPECT_EQ(r->socket_bind, 69);
  EXPECT_EQ(r->socket_listen, 69);
  EXPECT_EQ(r->socket_connect, 69);
  EXPECT_EQ(r->socket_error, 69);
  EXPECT_EQ(r->socket_accept, 69);
  EXPECT_EQ(r->socket_close, 69);
  EXPECT_EQ(r->socket_recv, 69);
  EXPECT_EQ(r->socket_send, 69);
  EXPECT_EQ(r->socket_wait, 69);
  EXPECT_EQ(r->socket_set_nonblocking, 69);
  EXPECT_EQ(r->socket_reuse_addr, 69);
  EXPECT_EQ(r->socket_local_port, 69);
  EXPECT_EQ((u64)r->alloc, (u64)69);
  EXPECT_EQ(r->free, 69);
  EXPECT_EQ((u64)r->memcpy, (u64)69);
  EXPECT_EQ((u64)r->memmove, (u64)69);
  EXPECT_EQ((u64)r->memset, (u64)69);
  EXPECT_EQ(r->memcmp, 69);
  EXPECT_EQ(r->assert, 69);
  EXPECT_EQ(r->exit, 69);
  EXPECT_EQ(r->env, 69);
  EXPECT_EQ(r->lseek, 69);
  EXPECT_EQ(r->chdir, 69);
  EXPECT_EQ(r->fs_it_open, 69);
  EXPECT_EQ(r->fs_it_next, 69);
  EXPECT_EQ(r->fs_it_close, 69);
}
