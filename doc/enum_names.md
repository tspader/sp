Use SP_ENUM_NAME_CASE() to generate string names for enum values.

## Good
```c
typedef enum {
    EVENT_INIT,
    EVENT_UPDATE,
    EVENT_DRAW,
    EVENT_QUIT
} event_type_t;

const c8* event_type_to_cstr(event_type_t type) {
    switch (type) {
        SP_SWITCH_ENUM_TO_CSTR(EVENT_INIT);
        SP_SWITCH_ENUM_TO_CSTR(EVENT_UPDATE);
        SP_SWITCH_ENUM_TO_CSTR(EVENT_DRAW);
        SP_SWITCH_ENUM_TO_CSTR(EVENT_QUIT);
        default: return "UNKNOWN";
    }
}

sp_str_t event_type_to_str(event_type_t type) {
    return SP_CSTR(event_type_to_cstr(type));
}

const c8* event_type_to_lower(event_type_t type) {
    switch (type) {
        case EVENT_INIT:   return "init";
        case EVENT_UPDATE: return "update";
        case EVENT_DRAW:   return "draw";
        case EVENT_QUIT:   return "quit";
        default: return "unknown";
    }
}

void log_event(event_type_t type) {
    SP_LOG("Processing event: {}", SP_FMT_CSTR(event_type_to_cstr(type)));
}
```

## Bad
```c
const char* event_type_to_string(event_type_t type) {
    switch (type) {
        case EVENT_INIT: return "EVENT_INIT";
        case EVENT_UPDATE: return "EVENT_UPDATE";
        case EVENT_DRAW: return "EVENT_DRAW";
        case EVENT_QUIT: return "EVENT_QUIT";
        default: return "UNKNOWN";
    }
}

printf("Processing event: %s\n", event_type_to_string(type));
```

# Tags
- api.strings.cstr
- api.logging