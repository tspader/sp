Use TOML integration for configuration file parsing and writing. toml.h is available via the spn package manager.

## Good
```c
sp_str_t config_data = sp_os_read_entire_file(SP_LIT("config.toml"));
c8 errbuf[256];
toml_table_t* conf = toml_parse(sp_str_to_cstr(config_data), errbuf, sizeof(errbuf));

toml_table_t* server = toml_table_table(conf, "server");
toml_datum_t port = toml_table_int(server, "port");
toml_datum_t host = toml_table_string(server, "host");

if (port.ok) {
    server_config.port = (u16)port.u.i;
}
if (host.ok) {
    server_config.host = sp_str_from_cstr(host.u.s);
    free(host.u.s);
}

sp_toml_writer_t writer = SP_ZERO_INITIALIZE();
sp_toml_writer_add_header(&writer, SP_LIT("database"));
sp_toml_writer_add_string(&writer, SP_LIT("driver"), SP_LIT("postgres"));
sp_toml_writer_add_int(&writer, SP_LIT("pool_size"), 10);
sp_str_t output = sp_str_builder_write(&writer.builder);

toml_free(conf);
```

## Bad
```c
FILE* f = fopen("config.toml", "r");
char line[256];
while (fgets(line, sizeof(line), f)) {
    if (strstr(line, "port =")) {
        sscanf(line, "port = %d", &server_config.port);
    }
    if (strstr(line, "host =")) {
        char host[128];
        sscanf(line, "host = \"%[^\"]\"", host);
        server_config.host = strdup(host);
    }
}
fclose(f);
```
