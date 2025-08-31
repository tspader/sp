Use sp_ring_buffer for efficient circular buffers with automatic wrapping.

## Good
```c
sp_ring_buffer_t events;
sp_ring_buffer_init(&events, 100, sizeof(event_t));

event_t evt = { .type = KEY_PRESS, .key = 'A' };
sp_ring_buffer_push_overwrite(&events, &evt);

sp_ring_buffer_for(events, it) {
    event_t* e = sp_rb_it(it, event_t);
    process_event(e);
}

sp_ring_buffer_rfor(events, it) {
    event_t* e = sp_rb_it(it, event_t);
    if (e->type == KEY_PRESS) break;
}

sp_ring_buffer_destroy(&events);
```

## Bad
```c
event_t events[100];
int head = 0;
int tail = 0;
int count = 0;

if (count == 100) {
    head = (head + 1) % 100;
} else {
    count++;
}
events[tail] = evt;
tail = (tail + 1) % 100;

for (int i = 0; i < count; i++) {
    int idx = (head + i) % 100;
    process_event(&events[idx]);
}
```