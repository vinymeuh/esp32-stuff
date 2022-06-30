idf_component_register(
    SRCS "dht.cpp"
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer
)