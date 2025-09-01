Including timing information in status messages.

## GOOD

```c
typedef struct {
  sp_str_t name;
  f64 start_time;
  f64 end_time;
} build_task_t;

void report_task_complete(build_task_t* task) {
  f64 duration = task->end_time - task->start_time;

  sp_str_t status;
  if (duration < 1.0) {
    status = sp_format(
      "{:fg green} {} {:fg brightblack}ms",
      SP_FMT_CSTR("+"),
      SP_FMT_STR(task->name),
      SP_FMT_S32((s32)(duration * 1000))
    );
  } else {
    status = sp_format(
      "{:fg green} {} {:fg brightblack}s",
      SP_FMT_CSTR("+"),
      SP_FMT_STR(task->name),
      SP_FMT_F64(duration)
    );
  }
  sp_log(status);
}
```

# Tags
- api.logging
- api.os.formatting
- api.os.time
