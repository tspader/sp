Prefer to return early on failure instead of using nested if statements. Push code up scopes whenever possible.

## Good
```c
bool process_file(sp_str_t path) {
  CXIndex index = clang_createIndex(0, 1);
  CXTranslationUnit unit = NULL;
  bool success = false;

  enum CXErrorCode error = clang_parseTranslationUnit2(
    index, sp_str_to_cstr(path), NULL, 0, NULL, 0,
    CXTranslationUnit_None, &unit
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
  return success;
}
```

## Bad
```c
bool process_file(sp_str_t path) {
  CXIndex index = clang_createIndex(0, 1);
  CXTranslationUnit unit = NULL;
  bool success = false;

  enum CXErrorCode error = clang_parseTranslationUnit2(
    index, sp_str_to_cstr(path), NULL, 0, NULL, 0,
    CXTranslationUnit_None, &unit
  );

  success = false;
  if (error == CXError_Success) {
    if (process_ast(unit)) {
      success = true;
    }
  }

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return success;
}
```
