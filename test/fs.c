#ifndef SP_TEST_AMALGAMATION
  #define SP_TEST_AMALGAMATION
  #define FS_TEST_OWNS_MAIN
#endif

#include "fs/normalize_path.c"
#include "fs/trim_path.c"
#include "fs/get_name.c"
#include "fs/parent_path.c"
#include "fs/get_ext.c"
#include "fs/get_stem.c"
#include "fs/join_path.c"
#include "fs/replace_ext.c"
#include "fs/is_root.c"
#include "fs/is_glob.c"
#include "fs/canonicalize_path.c"
#include "fs/get_exe_path.c"
#include "fs/get_cwd.c"
#include "fs/predicates.c"
#include "fs/create_dir.c"
#include "fs/create_file.c"
#include "fs/links.c"
#include "fs/copy.c"
#include "fs/copy_glob.c"
#include "fs/remove.c"
#include "fs/collect.c"
#include "fs/mod_time.c"
#include "fs/system_paths.c"
#include "fs/windows/wtf8.c"
#include "fs/windows/nt_path.c"

#ifdef FS_TEST_OWNS_MAIN
UTEST_MAIN()
#endif
