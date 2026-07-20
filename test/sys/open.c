#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_open)

UTEST_F(sys_open, read_refuses_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_read_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .fail = true } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  });
}

UTEST_F(sys_open, write_refuses_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_write_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .fail = true } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  });
}

UTEST_F(sys_open, read_forbids_write) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_read_forbids_write",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "B", .fail = true } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  });
}

UTEST_F(sys_open, write_forbids_read) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_write_forbids_read",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_READ, .read = { .count = 1, .fail = true } },
    },
  });
}

UTEST_F(sys_open, read_write_allows_both) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_read_write_allows_both",
    .setup = {
      { .path = "file.bin", .content = "AB" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_RW } },
      { .kind = SYS_STEP_READ, .read = { .count = 2, .expect = "AB" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "CD" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "ABCD" },
    },
  });
}

UTEST_F(sys_open, write_preserves_content) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_write_preserves_content",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAA" },
    },
  });
}

UTEST_F(sys_open, write_refuses_directory) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_write_refuses_directory",
    .setup = {
      { .path = "dir", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "dir", .mode = SP_SYS_OPEN_MODE_WO, .fail = true } },
    },
  });
}

UTEST_F(sys_open, create_creates_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_create_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  });
}

UTEST_F(sys_open, create_opens_existing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_create_opens_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAA" },
    },
  });
}

UTEST_F(sys_open, excl_refuses_existing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_excl_refuses_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_EXCLUSIVE, .fail = true } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAA" },
    },
  });
}

UTEST_F(sys_open, excl_refuses_symlink) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_excl_refuses_symlink",
    .setup = {
      { .path = "lnk", .kind = SYS_SETUP_SYMLINK, .target = "victim" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "lnk", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_EXCLUSIVE, .fail = true } },
    },
    .expect = {
      { .path = "victim" },
    },
  });
}

UTEST_F(sys_open, excl_implies_create) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_excl_implies_create",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_EXCLUSIVE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true },
    },
  });
}

UTEST_F(sys_open, truncate_empties_existing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_truncate_empties_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  });
}

UTEST_F(sys_open, truncate_refuses_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_truncate_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE, .fail = true } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  });
}

UTEST_F(sys_open, create_truncate_creates_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_create_truncate_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  });
}

UTEST_F(sys_open, create_truncate_empties_existing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_create_truncate_empties_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  });
}

UTEST_F(sys_open, append_preserves_content) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_append_preserves_content",
    .setup = {
      { .path = "file.bin", .content = "AA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "BB" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AABB" },
    },
  });
}

UTEST_F(sys_open, create_append_creates_missing_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_create_append_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "A" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  });
}

UTEST_EMPTY_FIXTURE(sys_open_dir)

UTEST_F(sys_open_dir, refuses_file) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_dir_refuses_file",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "file.bin", .fail = true } },
    },
  });
}

UTEST_F(sys_open_dir, refuses_missing_path) {
  SKIP_ON_WASM()
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_open_dir_refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "dir", .fail = true } },
    },
  });
}
