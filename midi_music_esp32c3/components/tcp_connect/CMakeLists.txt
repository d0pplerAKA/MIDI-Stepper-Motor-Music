idf_component_register(
    SRCS "tcp_connect.c"
    INCLUDE_DIRS include
    REQUIRES nvs_flash esp_wifi esp_event
    PRIV_REQUIRES midi_process file_system midi_play
)