#include "wire.h"

typedef struct {
  sp_err_t err;
} wire_scan_expect_t;

typedef struct {
  const c8* name;
  u32 prefix;
  const c8* pre;
  u32 nonce_bytes;
  wire_scan_expect_t expect;
} wire_scan_case_t;

static const wire_scan_case_t wire_scan_cases [] = {
  {
    .name = "immediate",
    .nonce_bytes = SP_TEST_WIRE_NONCE_SIZE,
  },
  {
    .name = "noise",
    .pre = "41 42 0d 0a",
    .nonce_bytes = SP_TEST_WIRE_NONCE_SIZE,
  },
  {
    .name = "feint",
    .prefix = 3,
    .pre = "07",
    .nonce_bytes = SP_TEST_WIRE_NONCE_SIZE,
  },
  {
    .name = "self_overlap",
    .prefix = 1,
    .nonce_bytes = SP_TEST_WIRE_NONCE_SIZE,
  },
  {
    .name = "missing",
    .pre = "41 42 0d 0a",
    .expect = { .err = SP_ERR_IO_EOF },
  },
  {
    .name = "partial_eof",
    .pre = "41",
    .nonce_bytes = 7,
    .expect = { .err = SP_ERR_IO_EOF },
  },
};

sp_test_each(wire, scan, wire_scan_case_t, wire_scan_cases) {
  sp_mem_t mem = sp_test_arena(t);

  sp_io_dyn_mem_writer_t writer;
  sp_io_dyn_mem_writer_init(mem, &writer);
  sp_io_write_str(&writer.base, sp_str((const c8*)wire_nonce, it->prefix), SP_NULLPTR);
  sp_io_write_str(&writer.base, wire_hex(mem, it->pre), SP_NULLPTR);
  sp_io_write_str(&writer.base, sp_str((const c8*)wire_nonce, it->nonce_bytes), SP_NULLPTR);
  if (it->nonce_bytes == SP_TEST_WIRE_NONCE_SIZE) {
    sp_must_ok(t, sp_test_wire_write(&writer.base, &(sp_test_wire_event_t) {
      .tag = SP_TEST_WIRE_START,
      .start = { .id = 5 },
    }));
  }

  sp_io_reader_t reader;
  wire_flip(&writer, &reader);
  sp_must_eq(t, sp_test_wire_scan_nonce(&reader, wire_nonce), it->expect.err);
  if (it->expect.err) return SP_OK;

  sp_test_wire_event_t got = sp_zero;
  sp_must_ok(t, sp_test_wire_read(&reader, mem, &got));
  sp_must_eq(t, got.tag, SP_TEST_WIRE_START);
  sp_expect_eq(t, got.start.id, 5);
  sp_must_eq(t, sp_test_wire_read(&reader, mem, &got), SP_ERR_IO_EOF);
  return SP_OK;
}
