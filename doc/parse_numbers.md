Use sp_parse_* functions for safe string to number conversions.

## Good
```c
sp_str_t port_str = SP_LIT("8080");
u32 port;
sp_parse_result_t result = sp_parse_u32_ex(port_str, &port);
if (result == SP_PARSE_OK) {
    server_listen(port);
}

sp_str_t hex_color = SP_LIT("ff00aa");
u32 color;
if (sp_parse_hex(hex_color, &color)) {
    set_background_color(color);
}

sp_str_t fps_str = SP_LIT("60.5");
f32 fps;
if (sp_parse_f32(fps_str, &fps)) {
    set_target_fps(fps);
}
```

## Bad
```c
char* port_str = "8080";
int port = atoi(port_str);
server_listen(port);

char* hex_color = "ff00aa";
unsigned int color;
sscanf(hex_color, "%x", &color);

char* fps_str = "60.5";
float fps = atof(fps_str);
```