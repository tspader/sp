#define SP_IMPLEMENTATION
#include "../sp.h"

typedef struct CXIndexImpl* CXIndex;
typedef struct CXTranslationUnitImpl* CXTranslationUnit;

typedef enum {
  CXError_Success = 0,
  CXError_Failure = 1,
} CXErrorCode;

typedef CXErrorCode enum_CXErrorCode;

typedef struct {
  int dummy;
} CXUnsused;

static CXIndex clang_createIndex(int exclude_decls, int display_diags) {
  SP_UNUSED(exclude_decls);
  SP_UNUSED(display_diags);
  return (CXIndex)1;
}

static CXErrorCode clang_parseTranslationUnit2(
    CXIndex index,
    const char* source_filename,
    void* command_line_args,
    int num_command_line_args,
    void* unsaved_files,
    unsigned num_unsaved_files,
    unsigned options,
    CXTranslationUnit* out_unit) {
  SP_UNUSED(index);
  SP_UNUSED(source_filename);
  SP_UNUSED(command_line_args);
  SP_UNUSED(num_command_line_args);
  SP_UNUSED(unsaved_files);
  SP_UNUSED(num_unsaved_files);
  SP_UNUSED(options);
  if (out_unit) {
    *out_unit = (CXTranslationUnit)1;
  }
  return CXError_Success;
}

static void clang_disposeTranslationUnit(CXTranslationUnit unit) {
  SP_UNUSED(unit);
}

static void clang_disposeIndex(CXIndex index) {
  SP_UNUSED(index);
}

static bool process_ast(CXTranslationUnit unit) {
  SP_UNUSED(unit);
  return true;
}

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

bool process_file(sp_str_t path) {
  CXIndex index = clang_createIndex(0, 1);
  CXTranslationUnit unit = NULL;
  bool success = false;
  c8* path_cstr = sp_str_to_cstr(path);

  CXErrorCode error = clang_parseTranslationUnit2(
    index, path_cstr, NULL, 0, NULL, 0,
    0, &unit
  );

  if (error != CXError_Success) {
    SP_LOG("Failed to parse {}: error {}", SP_FMT_STR(path), SP_FMT_S32(error));
    goto cleanup;
  }

  if (!process_ast(unit)) {
    SP_LOG("AST processing failed");
    goto cleanup;
  }

  success = true;

cleanup:
  if (unit) clang_disposeTranslationUnit(unit);
  if (index) clang_disposeIndex(index);
  sp_free(path_cstr);
  return success;
}

int main(void) {
  sp_example_init();
  process_file(SP_LIT("example.c"));
  sp_example_shutdown();
  return 0;
}
