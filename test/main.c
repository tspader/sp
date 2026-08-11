#define SP_IMPLEMENTATION
#include "sp/sp_test.h"

// test/sys/inherit.c re-executes this binary as [exe probe <op> <handle>] to
// observe a handle from inside a child process. Exit 0 means the op
// succeeded, 1 means it failed, 2 means the invocation was malformed.
static s32 probe(const c8* op, const c8* handle) {
  sp_sys_fd_t fd = (sp_sys_fd_t)sp_parse_s64(sp_cstr_as_str(handle));
  u64 n = 0;

  if (sp_cstr_equal(op, "read")) {
    c8 byte = 0;
    if (sp_sys_read(fd, &byte, 1, &n)) return 1;
    return n == 1 && byte == 'A' ? 0 : 1;
  }
  if (sp_cstr_equal(op, "write")) {
    if (sp_sys_write(fd, "A", 1, &n)) return 1;
    return n == 1 ? 0 : 1;
  }
  if (sp_cstr_equal(op, "send")) {
    if (sp_sys_socket_send((sp_sys_socket_t)fd, "A", 1, &n)) return 1;
    return n == 1 ? 0 : 1;
  }
  return 2;
}

static s32 entry(s32 argc, const c8** argv) {
  if (argc == 4 && sp_cstr_equal(argv[1], "probe")) {
    return probe(argv[2], argv[3]);
  }
  return sp_test_main(argc, argv, SP_NULLPTR);
}

SP_MAIN(entry)
