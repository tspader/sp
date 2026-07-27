#ifndef WIRE_TEST_H
#define WIRE_TEST_H

#include "sp/sp_test.h"

#define WIRE_TEST_MAX 4

#define wire_count(OUT, CARR, IT, EXPR) \
  u32 OUT = 0;                          \
  sp_carr_for_until(CARR, IT, EXPR) OUT++

static const u8 wire_nonce [SP_TEST_WIRE_NONCE_SIZE] = {
  0xf3, 0x00, 0xa1, 0xc9, 0x5e, 0x27, 0x87, 0x19
};

typedef struct {
  const c8* key;
  const c8* value;
} wire_kv_t;

typedef struct {
  sp_test_wire_tag_t tag;
  union {
    struct {
      u32 v;
      const c8* arch;
      const c8* os;
      const c8* abi;
      const c8* tests [WIRE_TEST_MAX];
    } plan;
    struct {
      u32 id;
    } start;
    struct {
      u32 id;
      u32 line;
      const c8* file;
      const c8* message;
      const c8* expected;
      const c8* actual;
      wire_kv_t kvs [WIRE_TEST_MAX];
    } failure;
    struct {
      u32 id;
      sp_test_wire_status_t status;
      u64 dur_ns;
      const c8* reason;
      const c8* notes [WIRE_TEST_MAX];
      const c8* logs [WIRE_TEST_MAX];
    } result;
    struct {
      u32 passed;
      u32 failed;
      u32 skipped;
      u32 updated;
      u64 dur_ns;
      const c8* capture;
    } summary;
  };
} wire_case_event_t;

typedef struct {
  const c8* name;
  wire_case_event_t event;
  const c8* bytes;
} wire_case_t;

static const wire_case_t wire_cases [] = {
  {
    .name = "start",
    .event = { .tag = SP_TEST_WIRE_START, .start = { .id = 2 } },
    .bytes = "02 04000000 02000000",
  },
  {
    .name = "plan",
    .event = {
      .tag = SP_TEST_WIRE_PLAN,
      .plan = { .v = 1, .arch = "A", .os = "L", .abi = "M", .tests = { "A.B", "A.C" } },
    },
    .bytes = "01 25000000 01000000 01000000 41 01000000 4c 01000000 4d"
             " 02000000 03000000 412e42 03000000 412e43",
  },
  {
    .name = "failure",
    .event = {
      .tag = SP_TEST_WIRE_FAILURE,
      .failure = {
        .id = 258,
        .line = 772,
        .file = "F",
        .message = "M",
        .expected = "E",
        .actual = "A",
        .kvs = { { .key = "K", .value = "V" } },
      },
    },
    .bytes = "03 2a000000 02010000 04030000 01000000 46 01000000 4d"
             " 01000000 45 01000000 41 01000000 01000000 4b 01000000 56",
  },
  {
    .name = "result",
    .event = {
      .tag = SP_TEST_WIRE_RESULT,
      .result = {
        .id = 1,
        .status = SP_TEST_WIRE_SKIP,
        .dur_ns = 0x1122334455667788,
        .reason = "R",
        .notes = { "N", "O" },
        .logs = { "L" },
      },
    },
    .bytes = "04 29000000 01000000 02 8877665544332211 01000000 52"
             " 02000000 01000000 4e 01000000 4f 01000000 01000000 4c",
  },
  {
    .name = "result_bare",
    .event = { .tag = SP_TEST_WIRE_RESULT },
    .bytes = "04 19000000 00000000 00 0000000000000000 00000000 00000000 00000000",
  },
  {
    .name = "summary",
    .event = {
      .tag = SP_TEST_WIRE_SUMMARY,
      .summary = { .passed = 1, .failed = 2, .skipped = 3, .updated = 4, .dur_ns = 5, .capture = "C" },
    },
    .bytes = "05 1d000000 01000000 02000000 03000000 04000000"
             " 0500000000000000 01000000 43",
  },
};

static sp_str_t wire_hex(sp_mem_t mem, const c8* hex) {
  sp_str_t text = sp_cstr_as_str(hex);
  u8* bytes = sp_alloc_n(mem, u8, text.len / 2);
  u32 len = 0;
  u32 at = 0;
  while (at < text.len) {
    if (text.data[at] == ' ') {
      at++;
      continue;
    }
    bytes[len++] = (u8)sp_parse_hex(sp_str(text.data + at, 2));
    at += 2;
  }
  return sp_str((const c8*)bytes, len);
}

static sp_str_t* wire_strs(sp_mem_t mem, const c8* const* items, u32 count) {
  sp_str_t* strs = sp_alloc_n(mem, sp_str_t, count);
  sp_for(it, count) {
    strs[it] = sp_cstr_as_str(items[it]);
  }
  return strs;
}

static sp_test_wire_event_t wire_event(sp_mem_t mem, const wire_case_event_t* c) {
  sp_test_wire_event_t event = { .tag = c->tag };
  switch (c->tag) {
    case SP_TEST_WIRE_PLAN: {
      wire_count(count, c->plan.tests, it, c->plan.tests[it]);
      event.plan = (sp_test_wire_plan_t) {
        .v = c->plan.v,
        .arch = sp_cstr_as_str(c->plan.arch),
        .os = sp_cstr_as_str(c->plan.os),
        .abi = sp_cstr_as_str(c->plan.abi),
        .tests = wire_strs(mem, c->plan.tests, count),
        .num_tests = count,
      };
      break;
    }
    case SP_TEST_WIRE_START: {
      event.start.id = c->start.id;
      break;
    }
    case SP_TEST_WIRE_FAILURE: {
      wire_count(count, c->failure.kvs, it, c->failure.kvs[it].key);
      sp_test_kv_t* kvs = sp_alloc_n(mem, sp_test_kv_t, count);
      sp_for(it, count) {
        kvs[it].key = sp_cstr_as_str(c->failure.kvs[it].key);
        kvs[it].value = sp_cstr_as_str(c->failure.kvs[it].value);
      }
      event.failure = (sp_test_wire_failure_t) {
        .id = c->failure.id,
        .line = c->failure.line,
        .file = sp_cstr_as_str(c->failure.file),
        .message = sp_cstr_as_str(c->failure.message),
        .expected = sp_cstr_as_str(c->failure.expected),
        .actual = sp_cstr_as_str(c->failure.actual),
        .kvs = kvs,
        .num_kvs = count,
      };
      break;
    }
    case SP_TEST_WIRE_RESULT: {
      wire_count(num_notes, c->result.notes, it, c->result.notes[it]);
      wire_count(num_logs, c->result.logs, it, c->result.logs[it]);
      event.result = (sp_test_wire_result_t) {
        .id = c->result.id,
        .status = c->result.status,
        .dur_ns = c->result.dur_ns,
        .reason = sp_cstr_as_str(c->result.reason),
        .notes = wire_strs(mem, c->result.notes, num_notes),
        .num_notes = num_notes,
        .logs = wire_strs(mem, c->result.logs, num_logs),
        .num_logs = num_logs,
      };
      break;
    }
    case SP_TEST_WIRE_SUMMARY: {
      event.summary = (sp_test_wire_summary_t) {
        .passed = c->summary.passed,
        .failed = c->summary.failed,
        .skipped = c->summary.skipped,
        .updated = c->summary.updated,
        .dur_ns = c->summary.dur_ns,
        .capture = sp_cstr_as_str(c->summary.capture),
      };
      break;
    }
  }
  return event;
}

static sp_err_t wire_expect_event(sp_test_t* t, const sp_test_wire_event_t* got, const wire_case_event_t* want) {
  sp_must_eq(t, got->tag, want->tag);
  switch (want->tag) {
    case SP_TEST_WIRE_PLAN: {
      sp_expect_eq(t, got->plan.v, want->plan.v);
      sp_expect_str_eq_c(t, got->plan.arch, want->plan.arch);
      sp_expect_str_eq_c(t, got->plan.os, want->plan.os);
      sp_expect_str_eq_c(t, got->plan.abi, want->plan.abi);
      wire_count(count, want->plan.tests, at, want->plan.tests[at]);
      sp_must_eq(t, got->plan.num_tests, count);
      sp_for(it, count) {
        sp_expect_str_eq_c(t, got->plan.tests[it], want->plan.tests[it]);
      }
      break;
    }
    case SP_TEST_WIRE_START: {
      sp_expect_eq(t, got->start.id, want->start.id);
      break;
    }
    case SP_TEST_WIRE_FAILURE: {
      sp_expect_eq(t, got->failure.id, want->failure.id);
      sp_expect_eq(t, got->failure.line, want->failure.line);
      sp_expect_str_eq_c(t, got->failure.file, want->failure.file);
      sp_expect_str_eq_c(t, got->failure.message, want->failure.message);
      sp_expect_str_eq_c(t, got->failure.expected, want->failure.expected);
      sp_expect_str_eq_c(t, got->failure.actual, want->failure.actual);
      wire_count(count, want->failure.kvs, at, want->failure.kvs[at].key);
      sp_must_eq(t, got->failure.num_kvs, count);
      sp_for(it, count) {
        sp_expect_str_eq_c(t, got->failure.kvs[it].key, want->failure.kvs[it].key);
        sp_expect_str_eq_c(t, got->failure.kvs[it].value, want->failure.kvs[it].value);
      }
      break;
    }
    case SP_TEST_WIRE_RESULT: {
      sp_expect_eq(t, got->result.id, want->result.id);
      sp_expect_eq(t, got->result.status, want->result.status);
      sp_expect_eq(t, got->result.dur_ns, want->result.dur_ns);
      sp_expect_str_eq_c(t, got->result.reason, want->result.reason);
      wire_count(num_notes, want->result.notes, at, want->result.notes[at]);
      sp_must_eq(t, got->result.num_notes, num_notes);
      sp_for(it, num_notes) {
        sp_expect_str_eq_c(t, got->result.notes[it], want->result.notes[it]);
      }
      wire_count(num_logs, want->result.logs, at, want->result.logs[at]);
      sp_must_eq(t, got->result.num_logs, num_logs);
      sp_for(it, num_logs) {
        sp_expect_str_eq_c(t, got->result.logs[it], want->result.logs[it]);
      }
      break;
    }
    case SP_TEST_WIRE_SUMMARY: {
      sp_expect_eq(t, got->summary.passed, want->summary.passed);
      sp_expect_eq(t, got->summary.failed, want->summary.failed);
      sp_expect_eq(t, got->summary.skipped, want->summary.skipped);
      sp_expect_eq(t, got->summary.updated, want->summary.updated);
      sp_expect_eq(t, got->summary.dur_ns, want->summary.dur_ns);
      sp_expect_str_eq_c(t, got->summary.capture, want->summary.capture);
      break;
    }
  }
  return SP_OK;
}

static u32 wire_event_id(const sp_test_wire_event_t* event) {
  switch (event->tag) {
    case SP_TEST_WIRE_START:   return event->start.id;
    case SP_TEST_WIRE_FAILURE: return event->failure.id;
    case SP_TEST_WIRE_RESULT:  return event->result.id;
    case SP_TEST_WIRE_PLAN:
    case SP_TEST_WIRE_SUMMARY: break;
  }
  return 0;
}

static void wire_flip(sp_io_dyn_mem_writer_t* writer, sp_io_reader_t* reader) {
  sp_str_t bytes = sp_io_dyn_mem_writer_as_str(writer);
  sp_io_reader_from_mem(reader, bytes.data, bytes.len);
}

#endif
