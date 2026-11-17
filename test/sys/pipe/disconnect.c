#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_WIN32)

sp_test(sys, read_after_pipe_disconnect) {
  sp_str_t name = sp_test_format(t, "\\\\.\\pipe\\sp-{}", sp_fmt_uint(GetCurrentProcessId()));
  u16 wide [64] = sp_zero;
  sp_must(t, name.len < sp_carr_len(wide));
  sp_for(i, name.len) {
    wide[i] = (u16)name.data[i];
  }

  HANDLE server = CreateNamedPipeW((const WCHAR*)wide, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 16, 16, 0, SP_NULLPTR);
  sp_must(t, server != INVALID_HANDLE_VALUE);
  HANDLE client = CreateFileW((const WCHAR*)wide, GENERIC_READ, 0, SP_NULLPTR, OPEN_EXISTING, 0, SP_NULLPTR);
  sp_must(t, client != INVALID_HANDLE_VALUE);

  DWORD written = 0;
  sp_must(t, WriteFile(server, "A", 1, &written, SP_NULLPTR));

  c8 byte = 0;
  u64 n = 0;
  sp_expect_ok(t, sp_sys_read((sp_sys_fd_t)client, &byte, 1, &n));
  sp_expect_eq(t, n, 1);

  sp_must(t, DisconnectNamedPipe(server));
  sp_expect_err_eq(t, sp_sys_read((sp_sys_fd_t)client, &byte, 1, &n), SP_ERR_SYS_BROKEN_PIPE);

  CloseHandle(client);
  CloseHandle(server);
  return SP_OK;
}

#endif
