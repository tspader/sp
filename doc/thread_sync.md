Use sp_thread and synchronization primitives for cross-platform threading.

## Good
```c
s32 worker_thread(void* userdata) {
    worker_data_t* data = (worker_data_t*)userdata;

    while (!data->stop) {
        sp_mutex_lock(&data->queue_mutex);
        if (data->queue_size > 0) {
            task_t task = data->queue[--data->queue_size];
            sp_mutex_unlock(&data->queue_mutex);

            process_task(&task);

            sp_semaphore_signal(&data->done_sem);
        } else {
            sp_mutex_unlock(&data->queue_mutex);
            sp_os_sleep_ms(10);
        }
    }
    return 0;
}

sp_thread_t thread;
sp_thread_init(&thread, worker_thread, &worker_data);
sp_semaphore_wait(&worker_data.done_sem);
sp_thread_join(&thread);
```

## Bad
```c
#ifdef _WIN32
    HANDLE thread = CreateThread(NULL, 0, worker_thread, data, 0, NULL);
    WaitForSingleObject(thread, INFINITE);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, worker_thread, data);
    pthread_join(thread, NULL);
#endif
```
