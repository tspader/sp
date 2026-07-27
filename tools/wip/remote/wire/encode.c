#include "wire.h"

sp_test_each(wire, encode, wire_case_t, wire_cases) {
  sp_mem_t mem = sp_test_arena(t);
  sp_test_wire_event_t event = wire_event(mem, &it->event);

  sp_io_dyn_mem_writer_t writer;
  sp_io_dyn_mem_writer_init(mem, &writer);
  sp_must_ok(t, sp_test_wire_write(&writer.base, &event));

  sp_str_t want = wire_hex(mem, it->bytes);
  sp_str_t bytes = sp_io_dyn_mem_writer_as_str(&writer);
  sp_must_eq(t, bytes.len, want.len);
  sp_must_mem_eq(t, bytes.data, want.data, want.len);
  return SP_OK;
}

sp_test(wire, nonce) {
  sp_io_dyn_mem_writer_t writer;
  sp_io_dyn_mem_writer_init(sp_test_arena(t), &writer);
  sp_must_ok(t, sp_test_wire_write_nonce(&writer.base, wire_nonce));

  sp_str_t bytes = sp_io_dyn_mem_writer_as_str(&writer);
  sp_must_eq(t, bytes.len, (u32)SP_TEST_WIRE_NONCE_SIZE);
  sp_must_mem_eq(t, bytes.data, wire_nonce, SP_TEST_WIRE_NONCE_SIZE);
  return SP_OK;
}
