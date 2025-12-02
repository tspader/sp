#include "sp.h"

#define SP_TEST_IMPLEMENTATION
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
  sp_tm_epoch_t epoch = sp_tm_now_epoch();
  sp_str_t iso = sp_tm_to_iso8601(epoch);
  EXPECT_GE(iso.len, 20);
  EXPECT_EQ(iso.data[4], '-');
  EXPECT_EQ(iso.data[7], '-');
  EXPECT_EQ(iso.data[10], 'T');
  EXPECT_EQ(iso.data[13], ':');
  EXPECT_EQ(iso.data[16], ':');
  EXPECT_EQ(iso.data[iso.len - 1], 'Z');
}

UTEST(tm, date_time) {
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
