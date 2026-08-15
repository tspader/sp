#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_WIN32)

#define MAX_UNITS 16

#define A16 "AAAAAAAAAAAAAAAA"
#define A400 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 \
             A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 \
             A16 A16 A16 A16 A16

typedef struct {
  bool fail;
  u16 exact [MAX_UNITS];
  u16 prefix [MAX_UNITS];
  u16 suffix [MAX_UNITS];
  u32 longer_than;
  bool nul_len;
  bool inline_buf;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "drive_absolute_forward_slashes",
    .input = "C:/A/B",
    .expect = {
      .prefix = { '\\','?','?','\\','C',':','\\' },
      .suffix = { 'A','\\','B' },
    },
  },
  {
    .name = "drive_absolute_backslashes",
    .input = "C:\\A\\B",
    .expect = {
      .exact = { '\\','?','?','\\','C',':','\\','A','\\','B' },
    },
  },
  {
    .name = "unc_path",
    .input = "\\\\A\\B\\C",
    .expect = {
      .prefix = { '\\','?','?','\\','U','N','C','\\' },
      .suffix = { 'A','\\','B','\\','C' },
    },
  },
  {
    .name = "verbatim_passthrough",
    .input = "\\\\?\\C:\\A",
    .expect = {
      .prefix = { '\\','?','?','\\','C',':','\\' },
    },
  },
  {
    .name = "dotdot_resolved",
    .input = "C:\\A\\..\\B",
    .expect = {
      .exact = { '\\','?','?','\\','C',':','\\','B' },
    },
  },
  {
    .name = "dot_resolved",
    .input = "C:\\A\\.\\B",
    .expect = {
      .exact = { '\\','?','?','\\','C',':','\\','A','\\','B' },
    },
  },
  {
    .name = "mixed_slashes",
    .input = "C:/A\\B/C",
    .expect = {
      .exact = { '\\','?','?','\\','C',':','\\','A','\\','B','\\','C' },
    },
  },
  {
    .name = "trailing_dot_stripped",
    .input = "C:\\A.txt.",
    .expect = {
      .exact = { '\\','?','?','\\','C',':','\\','A','.','t','x','t' },
    },
  },
  {
    .name = "non_ascii",
    .input = "C:\\caf\xC3\xA9.txt",
    .expect = {
      .exact = { '\\','?','?','\\','C',':','\\','c','a','f',0x00E9,'.','t','x','t' },
    },
  },
  {
    .name = "empty_rejected",
    .input = "",
    .expect = {
      .fail = true,
    },
  },
  {
    .name = "length_field_matches_buffer",
    .input = "C:\\A",
    .expect = {
      .nul_len = true,
    },
  },
  {
    .name = "long_verbatim_exceeds_max_path",
    .input = "\\\\?\\C:\\" A400 "\\F",
    .expect = {
      .prefix = { '\\','?','?','\\','C',':','\\' },
      .longer_than = 260,
    },
  },
  {
    .name = "buffer_is_inline",
    .input = "C:\\A",
    .expect = {
      .inline_buf = true,
    },
  },
};

static u32 units_len(const u16 units [MAX_UNITS]) {
  u32 n = 0;
  sp_for(it, MAX_UNITS) {
    if (!units[it]) break;
    n++;
  }
  return n;
}

sp_test_each(fs, nt_path, test_t, tests) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(sp_cstr_as_str(it->input), &path);

  if (it->expect.fail) {
    sp_expect(t, !SP_NT_SUCCESS(st));
    return SP_OK;
  }
  sp_must(t, SP_NT_SUCCESS(st));

  u32 len = path.name.Length / (u32)sizeof(u16);

  u32 exact = units_len(it->expect.exact);
  if (exact) {
    sp_must_eq(t, len, exact);
    sp_expect_arr_eq(t, path.name.Buffer, it->expect.exact, exact);
  }

  u32 prefix = units_len(it->expect.prefix);
  if (prefix) {
    sp_must_ge(t, len, prefix);
    sp_expect_arr_eq(t, path.name.Buffer, it->expect.prefix, prefix);
  }

  u32 suffix = units_len(it->expect.suffix);
  if (suffix) {
    sp_must_ge(t, len, suffix);
    sp_expect_arr_eq(t, path.name.Buffer + (len - suffix), it->expect.suffix, suffix);
  }

  if (it->expect.longer_than) {
    sp_expect_gt(t, len, it->expect.longer_than);
  }
  if (it->expect.nul_len) {
    u32 n = 0;
    while (path.name.Buffer[n] && n < 256) n++;
    sp_expect_eq(t, len, n);
  }
  if (it->expect.inline_buf) {
    sp_expect_eq(t, path.name.Buffer, path.data);
  }
  return SP_OK;
}

#else

sp_test(fs, nt_path_skipped_non_windows) {
  return sp_test_skip(t, "nt path tests require Windows");
}

#endif
