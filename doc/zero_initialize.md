ALWAYS use SP_ZERO_INITIALIZE() to zero-initialize any memory. Memory returned from sp_alloc is always zero initialized.

## Good
```c
sp_file_monitor_t monitor = SP_ZERO_INITIALIZE();
sp_str_builder_t builder = SP_ZERO_INITIALIZE();
sp_ring_buffer_iterator_t it = SP_ZERO_INITIALIZE();

typedef struct {
    u32 count;
    f32* values;
    sp_mutex_t mutex;
} complex_data_t;
complex_data_t data = SP_ZERO_INITIALIZE();
```

## Bad
```c
sp_file_monitor_t monitor;
memset(&monitor, 0, sizeof(monitor));

sp_str_builder_t builder = {0};

complex_data_t data;
data.count = 0;
data.values = NULL;
```
