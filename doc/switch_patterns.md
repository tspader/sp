Use SP_UNREACHABLE_CASE() and SP_FALLTHROUGH() for explicit switch control flow.

## Good
```c
switch (token.type) {
    case TOKEN_NUMBER:
        parse_number(&token);
        break;
    
    case TOKEN_STRING:
        parse_string(&token);
        break;
    
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        parse_operator(&token);
        break;
    
    case TOKEN_NEWLINE:
        if (ignore_whitespace) {
            SP_FALLTHROUGH();
        }
        handle_newline();
        break;
    
    case TOKEN_SPACE:
        break;
    
    default: {
        SP_UNREACHABLE_CASE();
    }
}

switch (event.type) {
    case EVENT_MOUSE_DOWN:
        start_drag();
        SP_FALLTHROUGH();
    
    case EVENT_MOUSE_MOVE:
        update_position(event.x, event.y);
        break;
    
    default: {
        SP_FATAL("Unknown event type: {}", SP_FMT_U32(event.type));
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
    
    case TOKEN_NEWLINE:
        if (ignore_whitespace) {
        }
        handle_newline();
        break;
    
    default:
        assert(0);
}
```