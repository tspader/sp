Use sp_file_monitor for cross-platform file system change detection.

## Good
```c
void on_file_change(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
    if (change->events & SP_FILE_CHANGE_EVENT_MODIFIED) {
        if (sp_str_ends_with(change->file_name, SP_LIT(".c"))) {
            trigger_rebuild();
        }
    }
}

sp_file_monitor_t monitor;
sp_file_monitor_init_debounce(&monitor, on_file_change, 
    SP_FILE_CHANGE_EVENT_MODIFIED | SP_FILE_CHANGE_EVENT_ADDED,
    userdata, 100);

sp_file_monitor_add_directory(&monitor, SP_LIT("src"));
sp_file_monitor_add_directory(&monitor, SP_LIT("include"));

while (running) {
    sp_file_monitor_process_changes(&monitor);
    sp_os_sleep_ms(50);
}
```

## Bad
```c
#ifdef _WIN32
    HANDLE dir = CreateFile("src", FILE_LIST_DIRECTORY, ...);
    FILE_NOTIFY_INFORMATION buffer[1024];
    ReadDirectoryChangesW(dir, buffer, ...);
#else
    int fd = inotify_init();
    inotify_add_watch(fd, "src", IN_MODIFY);
    struct inotify_event events[100];
    read(fd, events, sizeof(events));
#endif
```