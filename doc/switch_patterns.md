Use braces around case statement bodies and SP_UNREACHABLE_CASE() for unreachable defaults.

## Good
```c
switch (token.type) {
  case TOKEN_NUMBER: {
    parse_number(&token);
    break;
  }
  
  case TOKEN_STRING: {
    parse_string(&token);
    break;
  }
  
  case TOKEN_PLUS:
  case TOKEN_MINUS: {
    parse_operator(&token);
    break;
  }
  
  case TOKEN_SPACE: {
    break;
  }
  
  default: {
    SP_UNREACHABLE_CASE();
  }
}

switch (build_status) {
  case BUILD_PENDING: {
    msg = sp_format("{:fg brightblack} {}", SP_FMT_CSTR("[ ]"), SP_FMT_STR(target));
    break;
  }
  case BUILD_RUNNING: {
    msg = sp_format("{:fg yellow} {}", SP_FMT_CSTR("[.]"), SP_FMT_STR(target));
    break;
  }
  case BUILD_SUCCESS: {
    msg = sp_format("{:fg green} {}", SP_FMT_CSTR("[+]"), SP_FMT_STR(target));
    break;
  }
  case BUILD_FAILED: {
    msg = sp_format("{:fg red} {}", SP_FMT_CSTR("[-]"), SP_FMT_STR(target));
    break;
  }
}
```

## Bad
```c
switch (token.type) {
  case TOKEN_NUMBER:
    parse_number(&token);
    break;
  
  case TOKEN_PLUS:
  case TOKEN_MINUS:
    parse_operator(&token);
    break;
  
  default:
    assert(0);
}
```

# Tags
- api.logging
- api.os.formatting
- usage.general