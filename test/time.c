#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

UTEST(tm, timer_basic) {
  sp_tm_timer_t timer = sp_tm_start_timer();

  // busy wait for at least 1ms
  sp_tm_point_t start = sp_tm_now_point();
  while (sp_tm_point_diff(sp_tm_now_point(), start) < 1000000ULL) {}

  u64 elapsed = sp_tm_read_timer(&timer);
  EXPECT_GE(elapsed, 1000000ULL);  // at least 1ms in ns
}

UTEST(tm, timer_monotonic) {
  sp_tm_timer_t timer = sp_tm_start_timer();
  u64 t1 = sp_tm_read_timer(&timer);
  u64 t2 = sp_tm_read_timer(&timer);
  EXPECT_GE(t2, t1);
}

UTEST(tm, timer_lap) {
  sp_tm_timer_t timer = sp_tm_start_timer();

  sp_tm_point_t start = sp_tm_now_point();
  while (sp_tm_point_diff(sp_tm_now_point(), start) < 1000000ULL) {
  }
  u64 lap1 = sp_tm_lap_timer(&timer);

  start = sp_tm_now_point();
  while (sp_tm_point_diff(sp_tm_now_point(), start) < 1000000ULL) {
  }
  u64 lap2 = sp_tm_lap_timer(&timer);

  EXPECT_GE(lap1, 1000000ULL);
  EXPECT_GE(lap2, 1000000ULL);
}

UTEST(tm, epoch_to_iso) {
  SKIP_ON_FREESTANDING();
  sp_tm_epoch_t epoch = sp_tm_now_epoch();
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  sp_str_t iso = sp_tm_epoch_to_iso8601_a(s.mem, epoch);
  ASSERT_GE(iso.len, 20);
  EXPECT_EQ(iso.data[4], '-');
  EXPECT_EQ(iso.data[7], '-');
  EXPECT_EQ(iso.data[10], 'T');
  EXPECT_EQ(iso.data[13], ':');
  EXPECT_EQ(iso.data[16], ':');
  EXPECT_EQ(iso.data[iso.len - 1], 'Z');
  sp_mem_end_scratch_a(s);
}

UTEST(tm, date_time) {
  SKIP_ON_FREESTANDING();
  sp_tm_date_time_t dt = sp_tm_get_date_time();
  EXPECT_GE(dt.year, 2020);
  EXPECT_GE(dt.month, 1);
  EXPECT_LE(dt.month, 12);
  EXPECT_GE(dt.day, 1);
  EXPECT_LE(dt.day, 31);
  EXPECT_GE(dt.hour, 0);
  EXPECT_LE(dt.hour, 23);
  EXPECT_GE(dt.minute, 0);
  EXPECT_LE(dt.minute, 59);
  EXPECT_GE(dt.second, 0);
  EXPECT_LE(dt.second, 59);
  EXPECT_GE(dt.millisecond, 0);
  EXPECT_LE(dt.millisecond, 999);
}

// Time unit conversion tests

UTEST(tm, s_to_ms) {
  EXPECT_EQ(sp_tm_s_to_ms(0), 0ULL);
  EXPECT_EQ(sp_tm_s_to_ms(1), 1000ULL);
  EXPECT_EQ(sp_tm_s_to_ms(60), 60000ULL);
  EXPECT_EQ(sp_tm_s_to_ms(3600), 3600000ULL);
}

UTEST(tm, s_to_us) {
  EXPECT_EQ(sp_tm_s_to_us(0), 0ULL);
  EXPECT_EQ(sp_tm_s_to_us(1), 1000000ULL);
  EXPECT_EQ(sp_tm_s_to_us(60), 60000000ULL);
}

UTEST(tm, s_to_ns) {
  EXPECT_EQ(sp_tm_s_to_ns(0), 0ULL);
  EXPECT_EQ(sp_tm_s_to_ns(1), 1000000000ULL);
  EXPECT_EQ(sp_tm_s_to_ns(10), 10000000000ULL);
}

UTEST(tm, ms_to_s) {
  EXPECT_EQ(sp_tm_ms_to_s(0), 0ULL);
  EXPECT_EQ(sp_tm_ms_to_s(999), 0ULL);
  EXPECT_EQ(sp_tm_ms_to_s(1000), 1ULL);
  EXPECT_EQ(sp_tm_ms_to_s(1500), 1ULL);
  EXPECT_EQ(sp_tm_ms_to_s(60000), 60ULL);
}

UTEST(tm, ms_to_us) {
  EXPECT_EQ(sp_tm_ms_to_us(0), 0ULL);
  EXPECT_EQ(sp_tm_ms_to_us(1), 1000ULL);
  EXPECT_EQ(sp_tm_ms_to_us(1000), 1000000ULL);
}

UTEST(tm, ms_to_ns) {
  EXPECT_EQ(sp_tm_ms_to_ns(0), 0ULL);
  EXPECT_EQ(sp_tm_ms_to_ns(1), 1000000ULL);
  EXPECT_EQ(sp_tm_ms_to_ns(1000), 1000000000ULL);
}

UTEST(tm, us_to_s) {
  EXPECT_EQ(sp_tm_us_to_s(0), 0ULL);
  EXPECT_EQ(sp_tm_us_to_s(999999), 0ULL);
  EXPECT_EQ(sp_tm_us_to_s(1000000), 1ULL);
  EXPECT_EQ(sp_tm_us_to_s(1500000), 1ULL);
}

UTEST(tm, us_to_ms) {
  EXPECT_EQ(sp_tm_us_to_ms(0), 0ULL);
  EXPECT_EQ(sp_tm_us_to_ms(999), 0ULL);
  EXPECT_EQ(sp_tm_us_to_ms(1000), 1ULL);
  EXPECT_EQ(sp_tm_us_to_ms(1500), 1ULL);
}

UTEST(tm, us_to_ns) {
  EXPECT_EQ(sp_tm_us_to_ns(0), 0ULL);
  EXPECT_EQ(sp_tm_us_to_ns(1), 1000ULL);
  EXPECT_EQ(sp_tm_us_to_ns(1000), 1000000ULL);
}

UTEST(tm, ns_to_s) {
  EXPECT_EQ(sp_tm_ns_to_s(0), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_s(999999999), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_s(1000000000), 1ULL);
  EXPECT_EQ(sp_tm_ns_to_s(1500000000), 1ULL);
}

UTEST(tm, ns_to_ms) {
  EXPECT_EQ(sp_tm_ns_to_ms(0), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_ms(999999), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_ms(1000000), 1ULL);
  EXPECT_EQ(sp_tm_ns_to_ms(1500000), 1ULL);
}

UTEST(tm, ns_to_us) {
  EXPECT_EQ(sp_tm_ns_to_us(0), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_us(999), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_us(1000), 1ULL);
  EXPECT_EQ(sp_tm_ns_to_us(1500), 1ULL);
}

// Floating point conversion tests

UTEST(tm, ms_to_s_f) {
  EXPECT_NEAR(sp_tm_ms_to_s_f(0), 0.0, 0.0001);
  EXPECT_NEAR(sp_tm_ms_to_s_f(500), 0.5, 0.0001);
  EXPECT_NEAR(sp_tm_ms_to_s_f(1000), 1.0, 0.0001);
  EXPECT_NEAR(sp_tm_ms_to_s_f(1500), 1.5, 0.0001);
}

UTEST(tm, us_to_ms_f) {
  EXPECT_NEAR(sp_tm_us_to_ms_f(0), 0.0, 0.0001);
  EXPECT_NEAR(sp_tm_us_to_ms_f(500), 0.5, 0.0001);
  EXPECT_NEAR(sp_tm_us_to_ms_f(1000), 1.0, 0.0001);
  EXPECT_NEAR(sp_tm_us_to_ms_f(1500), 1.5, 0.0001);
}

UTEST(tm, ns_to_us_f) {
  EXPECT_NEAR(sp_tm_ns_to_us_f(0), 0.0, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_us_f(500), 0.5, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_us_f(1000), 1.0, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_us_f(1500), 1.5, 0.0001);
}

UTEST(tm, ns_to_ms_f) {
  EXPECT_NEAR(sp_tm_ns_to_ms_f(0), 0.0, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_ms_f(500000), 0.5, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_ms_f(1000000), 1.0, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_ms_f(1500000), 1.5, 0.0001);
}

UTEST(tm, ns_to_s_f) {
  EXPECT_NEAR(sp_tm_ns_to_s_f(0), 0.0, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_s_f(500000000), 0.5, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_s_f(1000000000), 1.0, 0.0001);
  EXPECT_NEAR(sp_tm_ns_to_s_f(1500000000), 1.5, 0.0001);
}

UTEST(tm, us_to_s_f) {
  EXPECT_NEAR(sp_tm_us_to_s_f(0), 0.0, 0.0001);
  EXPECT_NEAR(sp_tm_us_to_s_f(500000), 0.5, 0.0001);
  EXPECT_NEAR(sp_tm_us_to_s_f(1000000), 1.0, 0.0001);
  EXPECT_NEAR(sp_tm_us_to_s_f(1500000), 1.5, 0.0001);
}

UTEST(tm, roundtrip_s_ms) {
  EXPECT_EQ(sp_tm_ms_to_s(sp_tm_s_to_ms(42)), 42ULL);
  EXPECT_EQ(sp_tm_ms_to_s(sp_tm_s_to_ms(0)), 0ULL);
  EXPECT_EQ(sp_tm_ms_to_s(sp_tm_s_to_ms(3600)), 3600ULL);
}

UTEST(tm, roundtrip_ms_us) {
  EXPECT_EQ(sp_tm_us_to_ms(sp_tm_ms_to_us(42)), 42ULL);
  EXPECT_EQ(sp_tm_us_to_ms(sp_tm_ms_to_us(0)), 0ULL);
  EXPECT_EQ(sp_tm_us_to_ms(sp_tm_ms_to_us(1000)), 1000ULL);
}

UTEST(tm, roundtrip_us_ns) {
  EXPECT_EQ(sp_tm_ns_to_us(sp_tm_us_to_ns(42)), 42ULL);
  EXPECT_EQ(sp_tm_ns_to_us(sp_tm_us_to_ns(0)), 0ULL);
  EXPECT_EQ(sp_tm_ns_to_us(sp_tm_us_to_ns(1000000)), 1000000ULL);
}

// ISO8601 known-value tests

typedef struct {
  sp_tm_epoch_t epoch;
  const c8* expected;
} tm_iso8601_case_t;

UTEST(tm, iso8601_known_values) {
  tm_iso8601_case_t cases[] = {
    { { .s = 0,              .ns = 0 },         "1970-01-01T00:00:00.000Z" },
    { { .s = 946684800,      .ns = 0 },         "2000-01-01T00:00:00.000Z" },
    { { .s = 946684799,      .ns = 0 },         "1999-12-31T23:59:59.000Z" },
    { { .s = 2147483647,     .ns = 0 },         "2038-01-19T03:14:07.000Z" },
    { { .s = 2147483648ULL,  .ns = 0 },         "2038-01-19T03:14:08.000Z" },
    { { .s = 951782400,      .ns = 0 },         "2000-02-29T00:00:00.000Z" },
    { { .s = 1709208000,     .ns = 0 },         "2024-02-29T12:00:00.000Z" },
    { { .s = 1,              .ns = 0 },         "1970-01-01T00:00:01.000Z" },
    { { .s = 86400,          .ns = 0 },         "1970-01-02T00:00:00.000Z" },
    { { .s = 86399,          .ns = 0 },         "1970-01-01T23:59:59.000Z" },
    { { .s = 951868799,      .ns = 0 },         "2000-02-29T23:59:59.000Z" },
    { { .s = 951868800,      .ns = 0 },         "2000-03-01T00:00:00.000Z" },
    { { .s = 4107542399ULL,  .ns = 0 },         "2100-02-28T23:59:59.000Z" },
    { { .s = 4107542400ULL,  .ns = 0 },         "2100-03-01T00:00:00.000Z" },
    { { .s = 13574649599ULL, .ns = 0 },         "2400-02-29T23:59:59.000Z" },
    { { .s = 13574649600ULL, .ns = 0 },         "2400-03-01T00:00:00.000Z" },
    { { .s = 1704067199,     .ns = 0 },         "2023-12-31T23:59:59.000Z" },
    { { .s = 1704067200,     .ns = 0 },         "2024-01-01T00:00:00.000Z" },
    { { .s = 1706745599,     .ns = 0 },         "2024-01-31T23:59:59.000Z" },
    { { .s = 1706745600,     .ns = 0 },         "2024-02-01T00:00:00.000Z" },
    { { .s = 253402300799ULL, .ns = 0 },        "9999-12-31T23:59:59.000Z" },
  };

  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_tm_epoch_to_iso8601_a(s.mem, cases[i].epoch);
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
  sp_mem_end_scratch_a(s);
}

UTEST(tm, iso8601_millisecond_padding) {
  tm_iso8601_case_t cases[] = {
    { { .s = 0, .ns = 1000000 },    "1970-01-01T00:00:00.001Z" },
    { { .s = 0, .ns = 10000000 },   "1970-01-01T00:00:00.010Z" },
    { { .s = 0, .ns = 100000000 },  "1970-01-01T00:00:00.100Z" },
    { { .s = 0, .ns = 999000000 },  "1970-01-01T00:00:00.999Z" },
  };

  sp_mem_arena_marker_t s = sp_mem_begin_scratch_a();
  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_tm_epoch_to_iso8601_a(s.mem, cases[i].epoch);
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
  sp_mem_end_scratch_a(s);
}

UTEST(tm, epoch_to_date_time_known_values) {
  sp_tm_date_time_t dt = sp_tm_epoch_to_date_time(SP_RVAL(sp_tm_epoch_t) { .s = 1709208000, .ns = 123000000 });
  EXPECT_EQ(dt.year, 2024);
  EXPECT_EQ(dt.month, 2);
  EXPECT_EQ(dt.day, 29);
  EXPECT_EQ(dt.hour, 12);
  EXPECT_EQ(dt.minute, 0);
  EXPECT_EQ(dt.second, 0);
  EXPECT_EQ(dt.millisecond, 123);
}

UTEST(tm, fps_to_ns) {
  EXPECT_EQ(sp_tm_fps_to_ns(1), 1000000000ULL);
  EXPECT_EQ(sp_tm_fps_to_ns(30), 33333333ULL);
  EXPECT_EQ(sp_tm_fps_to_ns(60), 16666666ULL);
}

// UTEST(tm, sleep_ns_lower_bound) {
//   u64 sleep_ns = sp_tm_ms_to_ns(5);
//   sp_tm_point_t start = sp_tm_now_point();
//   sp_sleep_ns(sleep_ns);
//   sp_tm_point_t end = sp_tm_now_point();
//   u64 elapsed = sp_tm_point_diff(end, start);
//   EXPECT_GE(elapsed, sleep_ns);
// }
//
// UTEST(tm, sleep_ms_lower_bound) {
//   f64 sleep_ms = 10.0;
//   sp_tm_point_t start = sp_tm_now_point();
//   sp_sleep_ms(sleep_ms);
//   sp_tm_point_t end = sp_tm_now_point();
//   u64 elapsed = sp_tm_point_diff(end, start);
//   EXPECT_GE(elapsed, sp_tm_ms_to_ns(10));
// }
//
// UTEST(tm, os_sleep_ns_lower_bound) {
//   u64 sleep_ns = sp_tm_ms_to_ns(10);
//   sp_tm_point_t start = sp_tm_now_point();
//   sp_os_sleep_ns(sleep_ns);
//   sp_tm_point_t end = sp_tm_now_point();
//   u64 elapsed = sp_tm_point_diff(end, start);
//   EXPECT_GE(elapsed, sleep_ns);
// }
