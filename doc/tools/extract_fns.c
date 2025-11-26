#include "sp.h"

typedef struct {
  sp_str_t return_type;
  sp_str_t rest;
} sp_tool_parsed_fn_t;

s32 sp_tool_sort_kernel_fn_name_alphabetical(const void* a, const void* b) {
  sp_tool_parsed_fn_t* fn_a = (sp_tool_parsed_fn_t*)a;
  sp_tool_parsed_fn_t* fn_b = (sp_tool_parsed_fn_t*)b;

  return sp_str_compare_alphabetical(fn_a->rest, fn_b->rest);
}

s32 main(s32 num_args, const c8** args) {
  sp_str_t exe = sp_fs_get_exe_path();
  sp_str_t build = sp_fs_parent_path(exe);
  sp_str_t sp = sp_fs_parent_path(build);
  sp_str_t header = sp_fs_join_path(sp, sp_str_lit("sp.h"));

  sp_str_t content = sp_io_read_file(header);
  SP_ASSERT(!sp_str_empty(content));

  sp_da(sp_tool_parsed_fn_t) functions = SP_NULLPTR;
  u32 max_return_type_len = 0;

  sp_str_t remaining = content;
  while (!sp_str_empty(remaining)) {
    sp_str_pair_t pair = sp_str_cleave_c8(remaining, '\n');
    sp_str_t line = pair.first;
    remaining = pair.second;

    if (sp_str_starts_with(line, SP_LIT("SP_API"))) {
      sp_str_t function = sp_str_trim(line);
      function = sp_str_sub(function, 7, function.len - 7);
      function = sp_str_trim(function);

      // Parse into return type and rest
      sp_str_pair_t split = sp_str_cleave_c8(function, ' ');
      sp_str_t return_type = sp_str_trim(split.first);
      sp_str_t rest = sp_str_trim(split.second);

      sp_tool_parsed_fn_t parsed = SP_ZERO_INITIALIZE();
      parsed.return_type = return_type;
      parsed.rest = rest;

      sp_da_push(functions, parsed);

      if (return_type.len > max_return_type_len) {
        max_return_type_len = return_type.len;
      }
    }
  }

  sp_dyn_array_sort(functions, sp_tool_sort_kernel_fn_name_alphabetical);

  sp_da_for(functions, i) {
    sp_str_t padded_return_type = sp_str_pad(functions[i].return_type, max_return_type_len);
    SP_LOG("{} {}", SP_FMT_STR(padded_return_type), SP_FMT_STR(functions[i].rest));
  }

  SP_EXIT_SUCCESS();
}
