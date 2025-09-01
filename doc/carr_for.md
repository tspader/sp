Use SP_CARR_FOR() to iterate over C arrays with automatic size calculation.

## Good
```c
sp_formatter_t formatters[] = {
    { .id = SP_FMT_ID(STR), .fn = sp_fmt_format_str },
    { .id = SP_FMT_ID(U32), .fn = sp_fmt_format_u32 },
    { .id = SP_FMT_ID(F32), .fn = sp_fmt_format_f32 },
};

SP_CARR_FOR(formatters, i) {
    if (arg.id == formatters[i].id) {
        formatters[i].fn(&builder, &arg);
        break;
    }
}
```

## Bad
```c
sp_formatter_t formatters[3] = {
    { .id = SP_FMT_ID(STR), .fn = sp_fmt_format_str },
    { .id = SP_FMT_ID(U32), .fn = sp_fmt_format_u32 },
    { .id = SP_FMT_ID(F32), .fn = sp_fmt_format_f32 },
};

for (int i = 0; i < 3; i++) {
    if (arg.id == formatters[i].id) {
        formatters[i].fn(&builder, &arg);
        break;
    }
}
```

# Tags
- usage.general