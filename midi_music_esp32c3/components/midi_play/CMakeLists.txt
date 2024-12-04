set(COMPONENT_REQUIRES fatfs vfs)

idf_component_register(
    SRCS "midi_play.c"
    INCLUDE_DIRS include
    REQUIRES nvs_flash fatfs vfs
    PRIV_REQUIRES midi_command midi_process file_system
)