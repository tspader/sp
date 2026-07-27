#include "wire.h"

sp_test_each(wire, decode, wire_case_t, wire_cases) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t bytes = wire_hex(mem, it->bytes);

  sp_io_reader_t reader;
  sp_io_reader_from_mem(&reader, bytes.data, bytes.len);

  sp_test_wire_event_t got = sp_zero;
  sp_must_ok(t, sp_test_wire_read(&reader, mem, &got));
  sp_err_t err = wire_expect_event(t, &got, &it->event);
  if (err) return err;
  sp_must_eq(t, sp_test_wire_read(&reader, mem, &got), SP_ERR_IO_EOF);
  return SP_OK;
}
