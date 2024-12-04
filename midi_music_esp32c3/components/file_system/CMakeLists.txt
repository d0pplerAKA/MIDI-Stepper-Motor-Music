set(COMPONENT_REQUIRES fatfs vfs)

idf_component_register(
    SRCS "file_system.c"
    INCLUDE_DIRS include
    REQUIRES esp_system fatfs vfs nvs_flash
    PRIV_REQUIRES 
)