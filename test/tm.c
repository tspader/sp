
UTEST(sp_tm, timer_basic) {
  sp_tm_timer_t timer = sp_tm_start_timer();

  // busy wait for at least 1ms
  sp_tm_point_t start = sp_tm_now_point();
  while (sp_tm_point_diff(sp_tm_now_point(), start) < 1000000ULL) {}

  u64 elapsed = sp_tm_read_timer(&timer);
  ASSERT_GE(elapsed, 1000000ULL);  // at least 1ms in ns
}

UTEST(sp_tm, timer_monotonic) {
  sp_tm_timer_t timer = sp_tm_start_timer();
  u64 t1 = sp_tm_read_timer(&timer);
  u64 t2 = sp_tm_read_timer(&timer);
  ASSERT_GE(t2, t1);
}

UTEST(sp_tm, timer_lap) {
  sp_tm_timer_t timer = sp_tm_start_timer();

  sp_tm_point_t start = sp_tm_now_point();
  while (sp_tm_point_diff(sp_tm_now_point(), start) < 1000000ULL) {
  }
  u64 lap1 = sp_tm_lap_timer(&timer);

  start = sp_tm_now_point();
  while (sp_tm_point_diff(sp_tm_now_point(), start) < 1000000ULL) {
  }
  u64 lap2 = sp_tm_lap_timer(&timer);

  ASSERT_GE(lap1, 1000000ULL);
  ASSERT_GE(lap2, 1000000ULL);
}

UTEST(sp_tm, epoch_to_iso) {
  sp_tm_epoch_t epoch = sp_tm_now_epoch();
  sp_str_t iso = sp_tm_to_iso8601(epoch);
  ASSERT_GE(iso.len, 20);
  ASSERT_EQ(iso.data[4], '-');
  ASSERT_EQ(iso.data[7], '-');
  ASSERT_EQ(iso.data[10], 'T');
  ASSERT_EQ(iso.data[13], ':');
  ASSERT_EQ(iso.data[16], ':');
  ASSERT_EQ(iso.data[iso.len - 1], 'Z');
}

UTEST(sp_tm, date_time) {
  sp_tm_date_time_t dt = sp_tm_get_date_time();
  ASSERT_GE(dt.year, 2020);
  ASSERT_GE(dt.month, 1);
  ASSERT_LE(dt.month, 12);
  ASSERT_GE(dt.day, 1);
  ASSERT_LE(dt.day, 31);
  ASSERT_GE(dt.hour, 0);
  ASSERT_LE(dt.hour, 23);
  ASSERT_GE(dt.minute, 0);
  ASSERT_LE(dt.minute, 59);
  ASSERT_GE(dt.second, 0);
  ASSERT_LE(dt.second, 59);
  ASSERT_GE(dt.millisecond, 0);
  ASSERT_LE(dt.millisecond, 999);
}
