#include "sp.h"
#include "test.h"
#include "utest.h"

#if defined(SP_WIN32)

typedef struct {
  sp_str_t input;
  const u16* expected;
  u32 expected_len;
  const c8* label;
} nt_path_case_t;

static bool nt_path_starts_with(sp_nt_unicode_string_t s, const u16* prefix, u32 plen) {
  u32 slen = s.Length / (u16)sizeof(u16);
  if (slen < plen) return false;
  sp_for(i, plen) {
    if (s.Buffer[i] != prefix[i]) return false;
  }
  return true;
}

static bool nt_path_ends_with(sp_nt_unicode_string_t s, const u16* suffix, u32 slen) {
  u32 nlen = s.Length / (u16)sizeof(u16);
  if (nlen < slen) return false;
  sp_for(i, slen) {
    if (s.Buffer[nlen - slen + i] != suffix[i]) return false;
  }
  return true;
}

UTEST(nt_path, drive_absolute_forward_slashes) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:/foo/bar"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 prefix[] = {'\\','?','?','\\','C',':','\\'};
  EXPECT_TRUE(nt_path_starts_with(path.name, prefix, 7));

  static const u16 suffix[] = {'f','o','o','\\','b','a','r'};
  EXPECT_TRUE(nt_path_ends_with(path.name, suffix, 7));

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, drive_absolute_backslashes) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\foo\\bar"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected[] = {'\\','?','?','\\','C',':','\\','f','o','o','\\','b','a','r'};
  EXPECT_EQ(path.name.Length, (u16)(sizeof(expected)));

  sp_for(i, 14) {
    EXPECT_EQ(path.name.Buffer[i], expected[i]);
  }

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, unc_path) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("\\\\server\\share\\foo"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 prefix[] = {'\\','?','?','\\','U','N','C','\\'};
  EXPECT_TRUE(nt_path_starts_with(path.name, prefix, 8));

  static const u16 suffix[] = {'s','e','r','v','e','r','\\','s','h','a','r','e','\\','f','o','o'};
  EXPECT_TRUE(nt_path_ends_with(path.name, suffix, 16));

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, verbatim_passthrough) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("\\\\?\\C:\\foo"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 prefix[] = {'\\','?','?','\\','C',':','\\'};
  EXPECT_TRUE(nt_path_starts_with(path.name, prefix, 7));

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, dotdot_resolved) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\foo\\..\\bar"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected[] = {'\\','?','?','\\','C',':','\\','b','a','r'};
  EXPECT_EQ(path.name.Length, (u16)(sizeof(expected)));
  sp_for(i, 10) {
    EXPECT_EQ(path.name.Buffer[i], expected[i]);
  }

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, dot_resolved) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\foo\\.\\bar"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected[] = {'\\','?','?','\\','C',':','\\','f','o','o','\\','b','a','r'};
  EXPECT_EQ(path.name.Length, (u16)(sizeof(expected)));

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, mixed_slashes) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:/foo\\bar/baz"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected[] = {'\\','?','?','\\','C',':','\\','f','o','o','\\','b','a','r','\\','b','a','z'};
  EXPECT_EQ(path.name.Length, (u16)(sizeof(expected)));

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, trailing_dot_stripped) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\foo.txt."), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected[] = {'\\','?','?','\\','C',':','\\','f','o','o','.','t','x','t'};
  EXPECT_EQ(path.name.Length, (u16)(sizeof(expected)));

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, non_ascii) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\caf\xC3\xA9.txt"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected[] = {'\\','?','?','\\','C',':','\\','c','a','f', 0x00E9, '.','t','x','t'};
  EXPECT_EQ(path.name.Length, (u16)(sizeof(expected)));
  sp_for(i, 15) {
    EXPECT_EQ(path.name.Buffer[i], expected[i]);
  }

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, empty_rejected) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT(""), &path);
  EXPECT_FALSE(SP_NT_SUCCESS(st));
}

UTEST(nt_path, length_field_matches_buffer) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\abc"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));
  u32 count = 0;
  while (path.name.Buffer[count] && count < 256) count++;
  EXPECT_EQ((u32)(path.name.Length / sizeof(u16)), count);
  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, long_verbatim_exceeds_max_path) {
  c8 long_path[512];
  u32 n = 0;
  const c8* prefix = "\\\\?\\C:\\";
  sp_for(i, 7) long_path[n++] = prefix[i];
  sp_for(i, 400) long_path[n++] = 'a';
  long_path[n++] = '\\';
  long_path[n++] = 'f';
  long_path[n] = 0;

  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path((sp_str_t){ .data = long_path, .len = n }, &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));

  static const u16 expected_prefix[] = {'\\','?','?','\\','C',':','\\'};
  EXPECT_TRUE(nt_path_starts_with(path.name, expected_prefix, 7));
  EXPECT_TRUE(path.name.Length / sizeof(u16) > 260);

  sp_sys_nt_path_free(&path);
}

UTEST(nt_path, rtl_allocates_buffer) {
  sp_sys_nt_path_t path;
  sp_nt_status_t st = sp_sys_nt_path(SP_LIT("C:\\x"), &path);
  EXPECT_TRUE(SP_NT_SUCCESS(st));
  EXPECT_NE(path.heap_buffer, SP_NULLPTR);
  sp_sys_nt_path_free(&path);
  EXPECT_EQ(path.heap_buffer, SP_NULLPTR);
}

#else

UTEST(nt_path, skipped_non_windows) {
  UTEST_SKIP("nt path tests require Windows");
}

#endif

SP_TEST_MAIN()
