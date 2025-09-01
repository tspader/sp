Coloring output based on values or thresholds.

## GOOD

```c
sp_str_t format_test_result(u32 passed, u32 total) {
  sp_str_t color = (passed == total) ? SP_LIT("green") : SP_LIT("yellow");

  return sp_format(
    "{:fg {}}/{} tests passed",
    color,
    SP_FMT_U32(passed),
    SP_FMT_U32(total)
  );
}

sp_str_t format_size(u64 bytes) {
  f64 mb = bytes / (1024.0 * 1024.0);

  sp_str_t color;
  if (mb < 1.0)        color = SP_LIT("green");
  else if (mb < 10.0)  color = SP_LIT("yellow");
  else if (mb < 100.0) color = SP_LIT("brightyellow");
  else                 color = SP_LIT("red");

  return sp_format(
    "{:fg .2f} MB",
    color,
    SP_FMT_F64(mb)
  );
}
```
