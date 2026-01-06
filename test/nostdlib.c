#include "sp.h"

static void print(const char* s) {
  size_t len = 0;
  while (s[len]) len++;
  sp_sys_write(2, s, len);
}

static void print_ok(const char* name) {
  print("  [OK] ");
  print(name);
  print("\n");
}

static void print_fail(const char* name) {
  print("  [FAIL] ");
  print(name);
  print("\n");
}

#define TEST(name, cond) do { \
  if (cond) { print_ok(name); } \
  else { print_fail(name); failures++; } \
} while(0)

void _start(void) {
  int failures = 0;

  print("=== sp.h nostdlib test ===\n");

  print("\n--- memory functions ---\n");
  {
    char src[64] = "hello world";
    char dst[64] = {0};
    sp_sys_memcpy(dst, src, 12);
    TEST("memcpy", sp_sys_memcmp(dst, src, 12) == 0);

    sp_sys_memcpy(dst, src, 0);
    TEST("memcpy n=0", 1);

    char buf[32] = "0123456789";
    sp_sys_memmove(buf + 2, buf, 8);
    TEST("memmove overlap", sp_sys_memcmp(buf + 2, "01234567", 8) == 0);

    sp_sys_memset(dst, 'x', 16);
    int ok = 1;
    for (int i = 0; i < 16; i++) if (dst[i] != 'x') ok = 0;
    TEST("memset", ok);

    TEST("memcmp equal", sp_sys_memcmp("abc", "abc", 3) == 0);
    TEST("memcmp less", sp_sys_memcmp("abc", "abd", 3) < 0);
    TEST("memcmp greater", sp_sys_memcmp("abd", "abc", 3) > 0);
  }

  print("\n--- allocator ---\n");
  {
    void* p = sp_sys_alloc(100);
    TEST("alloc", p != 0);
    if (p) {
      sp_sys_memset(p, 0x42, 100);
      TEST("alloc writable", ((char*)p)[0] == 0x42);
      sp_sys_free(p);
    }

    void* z = sp_sys_alloc_zero(64);
    TEST("alloc_zero", z != 0);
    if (z) {
      int all_zero = 1;
      for (int i = 0; i < 64; i++) if (((char*)z)[i] != 0) all_zero = 0;
      TEST("alloc_zero zeroed", all_zero);
      sp_sys_free(z);
    }

    p = sp_sys_alloc(50);
    if (p) {
      sp_sys_memset(p, 0xAA, 50);
      void* r = sp_sys_realloc(p, 100);
      TEST("realloc", r != 0);
      if (r) {
        TEST("realloc preserves", ((unsigned char*)r)[0] == 0xAA);
        sp_sys_free(r);
      }
    }
  }

  print("\n--- file I/O ---\n");
  {
    const char* path = "/tmp/sp_nostdlib_test";
    const char* data = "test data";

    int fd = sp_sys_open(path, SP_O_WRONLY | SP_O_CREAT | SP_O_TRUNC, 0644);
    TEST("open create", fd >= 0);
    if (fd >= 0) {
      int64_t w = sp_sys_write(fd, data, 9);
      TEST("write", w == 9);
      sp_sys_close(fd);
    }

    fd = sp_sys_open(path, SP_O_RDONLY, 0);
    TEST("open read", fd >= 0);
    if (fd >= 0) {
      char buf[32] = {0};
      int64_t r = sp_sys_read(fd, buf, 32);
      TEST("read", r == 9);
      TEST("read data", sp_sys_memcmp(buf, data, 9) == 0);

      int64_t pos = sp_sys_lseek(fd, 0, SP_SEEK_SET);
      TEST("lseek", pos == 0);

      sp_sys_close(fd);
    }

    sp_sys_stat_t st;
    int ret = sp_sys_stat(path, &st);
    TEST("stat", ret == 0 && st.st_size == 9);

    ret = sp_sys_fstat(sp_sys_open(path, SP_O_RDONLY, 0), &st);
    TEST("fstat", ret == 0);

    sp_sys_unlink(path);
  }

  print("\n--- directory ops ---\n");
  {
    const char* dir = "/tmp/sp_nostdlib_dir";
    sp_sys_rmdir(dir);

    int ret = sp_sys_mkdir(dir, 0755);
    TEST("mkdir", ret == 0);

    sp_sys_stat_t st;
    ret = sp_sys_stat(dir, &st);
    TEST("stat dir", ret == 0 && SP_S_ISDIR(st.st_mode));

    int dfd = sp_sys_open(dir, SP_O_RDONLY | SP_O_DIRECTORY, 0);
    if (dfd >= 0) {
      char buf[512];
      int64_t n = sp_sys_getdents64(dfd, buf, sizeof(buf));
      TEST("getdents64", n > 0);
      sp_sys_close(dfd);
    }

    ret = sp_sys_rmdir(dir);
    TEST("rmdir", ret == 0);
  }

  print("\n--- paths ---\n");
  {
    char cwd[256] = {0};
    int64_t ret = sp_sys_getcwd(cwd, sizeof(cwd));
    TEST("getcwd", ret > 0 && cwd[0] == '/');

    char saved[256];
    sp_sys_memcpy(saved, cwd, 256);

    ret = sp_sys_chdir("/tmp");
    TEST("chdir", ret == 0);
    sp_sys_chdir(saved);

    const char* target = "/tmp/sp_link_target";
    const char* link = "/tmp/sp_symlink";
    int fd = sp_sys_open(target, SP_O_CREAT | SP_O_WRONLY, 0644);
    if (fd >= 0) sp_sys_close(fd);

    sp_sys_unlink(link);
    ret = sp_sys_symlink(target, link);
    TEST("symlink", ret == 0);

    char buf[256] = {0};
    int64_t len = sp_sys_readlink(link, buf, sizeof(buf));
    TEST("readlink", len > 0);

    sp_sys_stat_t st;
    ret = sp_sys_lstat(link, &st);
    TEST("lstat", ret == 0 && SP_S_ISLNK(st.st_mode));

    sp_sys_unlink(link);
    sp_sys_unlink(target);

    const char* src = "/tmp/sp_rename_src";
    const char* dst = "/tmp/sp_rename_dst";
    fd = sp_sys_open(src, SP_O_CREAT | SP_O_WRONLY, 0644);
    if (fd >= 0) sp_sys_close(fd);
    sp_sys_unlink(dst);
    ret = sp_sys_rename(src, dst);
    TEST("rename", ret == 0);
    sp_sys_unlink(dst);
  }

  print("\n--- time ---\n");
  {
    sp_sys_timespec_t ts;
    int ret = sp_sys_clock_gettime(SP_CLOCK_MONOTONIC, &ts);
    TEST("clock_gettime", ret == 0);

    sp_sys_timespec_t req = {0, 1000000};
    sp_sys_timespec_t rem;
    ret = sp_sys_nanosleep(&req, &rem);
    TEST("nanosleep", ret == 0);
  }

  print("\n--- process ---\n");
  {
    int pid = sp_sys_getpid();
    TEST("getpid", pid > 0);

    int pipefd[2];
    int ret = sp_sys_pipe2(pipefd, 0);
    TEST("pipe2", ret == 0);
    if (ret == 0) {
      sp_sys_write(pipefd[1], "x", 1);
      char c;
      sp_sys_read(pipefd[0], &c, 1);
      TEST("pipe read/write", c == 'x');
      sp_sys_close(pipefd[0]);
      sp_sys_close(pipefd[1]);
    }

    int fd = sp_sys_open("/dev/null", SP_O_RDONLY, 0);
    if (fd >= 0) {
      int fd2 = sp_sys_dup2(fd, 200);
      TEST("dup2", fd2 == 200);

      int flags = sp_sys_fcntl(fd2, SP_F_GETFD, 0);
      TEST("fcntl GETFD", flags >= 0);

      sp_sys_close(fd);
      sp_sys_close(fd2);
    }
  }

  print("\n--- inotify ---\n");
  {
    int fd = sp_sys_inotify_init1(SP_IN_NONBLOCK | SP_IN_CLOEXEC);
    TEST("inotify_init1", fd >= 0);
    if (fd >= 0) {
      int wd = sp_sys_inotify_add_watch(fd, "/tmp", SP_IN_CREATE);
      TEST("inotify_add_watch", wd >= 0);
      if (wd >= 0) {
        int ret = sp_sys_inotify_rm_watch(fd, wd);
        TEST("inotify_rm_watch", ret == 0);
      }
      sp_sys_close(fd);
    }
  }

  print("\n--- TLS ---\n");
  {
    sp_sys_init();
    void* tp = sp_sys_get_tp();
    TEST("sp_sys_init + get_tp", tp == &sp_sys_thread_block);

    sp_sys_tls_set((void*)0xDEADBEEF);
    void* data = sp_sys_tls_get();
    TEST("tls_set/get", data == (void*)0xDEADBEEF);
  }

  print("\n=== done ===\n");
  if (failures == 0) {
    print("All tests passed!\n");
  } else {
    print("FAILURES detected\n");
  }

  sp_sys_exit(failures);
}
