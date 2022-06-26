idf_component_register(
    SRCS "webserver.c" "wifi.c"
    INCLUDE_DIRS "."
    REQUIRES mdns nvs_flash
)