# Process Output Parsing

Reading and parsing command output into structured data.

## GOOD

```c
sp_size_t len = 0;
s32 return_code;
void* output = SDL_ReadProcess(process, &len, &return_code);
SDL_DestroyProcess(process);

if (output && return_code == 0) {
  sp_str_t result = sp_str_from_cstr_sized((c8*)output, len);
  result = sp_str_trim(result);
  
  sp_dyn_array(sp_str_t) lines = sp_str_split_c8(result, '\n');
  sp_dyn_array_for(lines, i) {
    sp_str_t line = sp_str_trim(lines[i]);
    if (line.len > 0) {
      SP_LOG("  {}", SP_FMT_STR(line));
    }
  }
}

SDL_free(output);
```

# Tags
- api.strings.sp_str_t.copy
- api.strings.sp_str_t.common_operations
- api.sp_dynamic_array_t
- api.logging