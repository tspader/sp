#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_glob.h"

#define str(s) sp_str_lit(s)

s32 main(s32 num_args, const c8** args) {
  sp_mem_t mem = sp_mem_os_new();

  sp_str_t exe = sp_fs_get_name(sp_fs_get_exe_path_a(mem));
  sp_str_t usage = sp_fmt("usage: {} {.cyan} {.yellow}", sp_fmt_str(exe), sp_fmt_cstr("$triple"), sp_fmt_cstr("glob"));
  switch (num_args) {
    case 3: break;
    default: sp_log_str(usage); return 1;
  }

  sp_str_t triple = sp_str_view(args[1]);
  sp_str_t pattern = sp_str_view(args[2]);
  sp_str_t cwd = sp_fs_get_cwd_a(mem);
  sp_str_t build = sp_fs_join_path_a(mem, cwd, str("build"));
  build = sp_fs_join_path_a(mem, build, triple);
  build = sp_fs_join_path_a(mem, build, str("test"));
  if (!sp_fs_exists_a(build)) {
    sp_fatal("error: {.cyan} doesn't exist", sp_fmt_str(build));
  }

  sp_da(sp_str_t) parts = sp_str_split_c8(triple, '-');
  sp_os_kind_t os = sp_zero();
  if      (sp_str_equal(parts[1], str("linux"))) os = SP_OS_LINUX;
  else if (sp_str_equal(parts[1], str("windows"))) os = SP_OS_WIN32;
  else if (sp_str_equal(parts[1], str("macos"))) os = SP_OS_MACOS;

  if (os == SP_OS_MACOS && !sp_str_equal(parts[0], str("aarch64")) && !sp_str_equal(parts[0], str("arm64"))) {
    sp_fatal("error: only arm64 macOS is supported (miles is arm); got {.cyan}", sp_fmt_str(parts[0]));
  }

  sp_glob_set_t* glob = sp_glob_set_new();
  sp_glob_set_add_str(glob, pattern);
  if (sp_str_at(pattern, -1) != '*') {
    sp_glob_set_add_str(glob, sp_fmt("{}*", sp_fmt_str(pattern)));
  }
  sp_glob_set_build(glob);

  sp_glob_set_t* blacklist = sp_glob_set_new();
  sp_glob_set_add(blacklist, "elf");
  sp_glob_set_add(blacklist, "elf.exe");
  sp_glob_set_add(blacklist, "process");
  sp_glob_set_add(blacklist, "process.exe");
  sp_glob_set_build(blacklist);
  //sp_glob_t* glob = sp_glob_new_str(pattern);

  typedef struct {
    sp_str_t path;
    sp_str_t name;
  } test_t;
  sp_da(test_t) tests = sp_zero();

  sp_fs_for(mem, build, it) {
    if (sp_glob_set_match(blacklist, it.entry.name)) continue;
    if (sp_glob_set_match(glob, it.entry.name)) {
      sp_io_reader_t io = sp_zero();
      sp_io_reader_from_file(&io, it.entry.path);

      u64 size = sp_zero();
      if (sp_io_reader_size(&io, &size)) sp_fatal("failed to open {.cyan}", it.entry.path);

      u8 buffer [8] = sp_zero();
      if (sp_io_read(&io, buffer, 8, SP_NULLPTR)) sp_fatal("failed to read magic for {.cyan}", it.entry.path);

      static u8 elf   [] = { 0x7F, 'E', 'L', 'F' };
      static u8 pe    [] = { 'M', 'Z' };
      static u8 mh64  [] = { 0xCF, 0xFA, 0xED, 0xFE };
      static u8 mh32  [] = { 0xCE, 0xFA, 0xED, 0xFE };
      static u8 mhbe64[] = { 0xFE, 0xED, 0xFA, 0xCF };
      static u8 mhbe32[] = { 0xFE, 0xED, 0xFA, 0xCE };
      static u8 fat   [] = { 0xCA, 0xFE, 0xBA, 0xBE };
      static u8 fatle [] = { 0xBE, 0xBA, 0xFE, 0xCA };

      bool match = false;
      switch (os) {
        case SP_OS_LINUX: {
          match = sp_mem_is_equal(buffer, elf, sizeof(elf));
          break;
        }
        case SP_OS_WIN32: {
          match = sp_mem_is_equal(buffer, pe, sizeof(pe));
          break;
        }
        case SP_OS_MACOS: {
          match = sp_mem_is_equal(buffer, mh64,   sizeof(mh64))
               || sp_mem_is_equal(buffer, mh32,   sizeof(mh32))
               || sp_mem_is_equal(buffer, mhbe64, sizeof(mhbe64))
               || sp_mem_is_equal(buffer, mhbe32, sizeof(mhbe32))
               || sp_mem_is_equal(buffer, fat,    sizeof(fat))
               || sp_mem_is_equal(buffer, fatle,  sizeof(fatle));
          break;
        }
      }
      if (!match) continue;

      test_t test = {
        .name = sp_str_copy(it.entry.name),
        .path = sp_str_copy(it.entry.path)
      };
      sp_da_push(tests, test);
    }
  }

  typedef struct {
    sp_str_t display;
    sp_ps_config_t config;
  } command_t;
  sp_da(command_t) commands = sp_zero();

  sp_da_for(tests, it) {
    test_t test = tests[it];
    command_t cmd = sp_zero();
    sp_ps_io_config_t io = {
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_CREATE },
    };
    switch (os) {
      case SP_OS_LINUX: {
        cmd.display = test.path;
        cmd.config = (sp_ps_config_t) { .command = test.path, .io = io };
        break;
      }
      case SP_OS_WIN32: {
        sp_str_t inner = sp_fmt("cd C:/Users/spader/source/sp; ./build/{}/test/{}", sp_fmt_str(triple), sp_fmt_str(test.name));
        sp_str_t powershell = sp_fmt("powershell -Command {.quote}", sp_fmt_str(inner));
        cmd.display = sp_fmt("ssh -q spader@piotr {}", sp_fmt_str(powershell));
        cmd.config = (sp_ps_config_t) {
          .command = str("ssh"),
          .args = { str("-q"), str("spader@piotr"), powershell },
          .io = io,
        };
        break;
      }
      case SP_OS_MACOS: {
        sp_str_t inner = sp_fmt("cd ~/source/sp && ./build/{}/test/{}", sp_fmt_str(triple), sp_fmt_str(test.name));
        cmd.display = sp_fmt("ssh -q spader@miles {.quote}", sp_fmt_str(inner));
        cmd.config = (sp_ps_config_t) {
          .command = str("ssh"),
          .args = { str("-q"), str("spader@miles"), inner },
          .io = io,
        };
        break;
      }
    }
    sp_da_push(commands, cmd);
  }

  u32 width = str("name").len;
  sp_da_for(tests, it) {
    if (tests[it].name.len > width) width = tests[it].name.len;
  }

  sp_log("{:<$ .gray} {.gray}", sp_fmt_uint(width), sp_fmt_cstr("name"), sp_fmt_cstr("command"));
  sp_da_for(commands, it) {
    sp_log("{:>$ .cyan} {}", sp_fmt_uint(width), sp_fmt_str(tests[it].name), sp_fmt_str(commands[it].display));
  }
  sp_log("");

  sp_da(sp_ps_output_t) results = sp_zero();
  s32 status = 0;
  sp_da_for(commands, it) {
    sp_print("{:>$}...", sp_fmt_uint(width), sp_fmt_str(tests[it].name));
    sp_ps_output_t result = sp_ps_run(commands[it].config);
    sp_da_push(results, result);
    if (result.status.exit_code) {
      status = 1;
      sp_log("{.fg red}", sp_fmt_cstr("fail"));
    }
    else {
      sp_log("{.fg green}", sp_fmt_cstr("ok"));
    }
  }

  sp_da_for(results, it) {
    if (!results[it].status.exit_code) continue;
    sp_log("");
    sp_log("{.fg red}: {}", sp_fmt_cstr("FAIL"), sp_fmt_str(tests[it].name));
    sp_log_str(commands[it].display);
    sp_log_str(results[it].out);
    sp_log_str(results[it].err);
  }

  return status;
}
