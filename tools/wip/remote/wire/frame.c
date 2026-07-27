#include "wire.h"

typedef struct {
  sp_test_wire_tag_t tag;
  u32 id;
} wire_frame_event_t;

typedef struct {
  sp_err_t err;
  wire_frame_event_t events [WIRE_TEST_MAX];
} wire_frame_expect_t;

typedef struct {
  const c8* name;
  const c8* bytes;
  wire_frame_expect_t expect;
} wire_frame_case_t;

static const wire_frame_case_t wire_frame_cases [] = {
  {
    .name = "empty",
    .expect = { .err = SP_ERR_IO_EOF },
  },
  {
    .name = "zero_tag",
    .bytes = "00 00000000",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "giant_len",
    .bytes = "02 ffffffff",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "truncated_header",
    .bytes = "02 0400",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "truncated_payload",
    .bytes = "02 04000000 aabb",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "short_frame",
    .bytes = "02 02000000 0700",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "unknown_skipped",
    .bytes = "ee 05000000 6a756e6b21 02 04000000 07000000",
    .expect = {
      .err = SP_ERR_IO_EOF,
      .events = { { .tag = SP_TEST_WIRE_START, .id = 7 } },
    },
  },
  {
    .name = "unknown_truncated",
    .bytes = "ee 05000000 6a75",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "appended_fields",
    .bytes = "02 08000000 07000000 deadbeef 02 04000000 09000000",
    .expect = {
      .err = SP_ERR_IO_EOF,
      .events = {
        { .tag = SP_TEST_WIRE_START, .id = 7 },
        { .tag = SP_TEST_WIRE_START, .id = 9 },
      },
    },
  },
  {
    .name = "str_overrun",
    .bytes = "03 0c000000 01000000 02000000 20000000",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "count_overrun",
    .bytes = "01 14000000 01000000 00000000 00000000 00000000 ffffff7f",
    .expect = { .err = SP_ERR },
  },
  {
    .name = "status_unknown",
    .bytes = "04 19000000 05000000 09 0000000000000000 00000000 00000000 00000000",
    .expect = {
      .err = SP_ERR_IO_EOF,
      .events = { { .tag = SP_TEST_WIRE_RESULT, .id = 5 } },
    },
  },
};

sp_test_each(wire, frame, wire_frame_case_t, wire_frame_cases) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t bytes = wire_hex(mem, it->bytes);

  sp_io_reader_t reader;
  sp_io_reader_from_mem(&reader, bytes.data, bytes.len);

  wire_count(want, it->expect.events, at, it->expect.events[at].tag);

  u32 count = 0;
  sp_err_t err = SP_OK;
  while (true) {
    sp_test_wire_event_t event = sp_zero;
    err = sp_test_wire_read(&reader, mem, &event);
    if (err) break;
    sp_must_lt(t, count, want);
    sp_expect_eq(t, event.tag, it->expect.events[count].tag);
    sp_expect_eq(t, wire_event_id(&event), it->expect.events[count].id);
    count++;
  }

  sp_expect_eq(t, err, it->expect.err);
  sp_expect_eq(t, count, want);
  return SP_OK;
}
