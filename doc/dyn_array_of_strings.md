Use sp_dyn_array to build dynamic arrays.

## GOOD

```c
sp_dyn_array(const c8*) args = SP_NULLPTR;
sp_dyn_array_push(args, "clang");
sp_dyn_array_push(args, "-c");

sp_dyn_array_for(include_paths, i) {
  sp_dyn_array_push(args, sp_str_to_cstr(
    sp_format("-I{}", SP_FMT_STR(include_paths[i]))
  ));
}

sp_dyn_array_push(args, sp_str_to_cstr(source_file));
sp_dyn_array_push(args, SP_NULLPTR);
```

# Tags
- api.sp_dynamic_array_t
- api.strings.cstr
- api.os.formatting
