Use sp_dyn_array to build a dynamic array (e.g. of strings)

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

SDL_Process* process = SDL_CreateProcess(args, SP_SDL_PIPE_STDIO);

sp_dyn_array_for(args, i) {
  if (args[i]) sp_free((void*)args[i]);
}
sp_dyn_array_clear(args);
```

# Tags
- api.sp_dynamic_array_t
- api.strings.cstr
- api.os.formatting
