#include "wire.h"

sp_test(wire, binary) {
  sp_mem_t mem = sp_test_arena(t);

  const u8 raw [] = { 0x00, 0xff, 0x0a, 0x22, 0x5c };
  sp_test_wire_event_t event = {
    .tag = SP_TEST_WIRE_FAILURE,
    .failure = { .message = sp_str((const c8*)raw, sizeof(raw)) },
  };

  sp_io_dyn_mem_writer_t writer;
  sp_io_dyn_mem_writer_init(mem, &writer);
  sp_must_ok(t, sp_test_wire_write(&writer.base, &event));

  sp_io_reader_t reader;
  wire_flip(&writer, &reader);
  sp_test_wire_event_t got = sp_zero;
  sp_must_ok(t, sp_test_wire_read(&reader, mem, &got));
  sp_must_eq(t, got.tag, SP_TEST_WIRE_FAILURE);
  sp_must_eq(t, got.failure.message.len, (u32)sizeof(raw));
  sp_must_mem_eq(t, got.failure.message.data, raw, sizeof(raw));
  return SP_OK;
}
