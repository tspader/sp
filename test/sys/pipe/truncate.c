#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_WIN32)

sp_test(sys, open_truncate_on_pipe) {
  sp_str_t name = sp_test_format(t, "\\\\.\\pipe\\sp-truncate-{}", sp_fmt_uint(GetCurrentProcessId()));
  u16 wide [64] = sp_zero;
  sp_must(t, name.len < sp_carr_len(wide));
  sp_for(i, name.len) {
    wide[i] = (u16)name.data[i];
  }

  HANDLE server = CreateNamedPipeW((const WCHAR*)wide, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 16, 16, 0, SP_NULLPTR);
  sp_must(t, server != INVALID_HANDLE_VALUE);

  sp_sys_fd_t client = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_s(sp_fs_get_cwd(), name, SP_SYS_OPEN_MODE_WO, SP_SYS_OPEN_TRUNCATE, sp_sys_default_file_perms, &client));

  u64 written = 0;
  sp_expect_ok(t, sp_sys_write(client, "A", 1, &written));
  sp_expect_eq(t, written, 1);

  c8 byte = 0;
  DWORD read = 0;
  sp_must(t, ReadFile(server, &byte, 1, &read, SP_NULLPTR));
  sp_expect_eq(t, byte, 'A');

  sp_sys_close(client);
  CloseHandle(server);
  return SP_OK;
}

#endif
