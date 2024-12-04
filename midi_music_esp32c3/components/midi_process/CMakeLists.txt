idf_component_register(
    SRCS "midi_process.c"
    INCLUDE_DIRS include
    REQUIRES midi_command file_system
    PRIV_REQUIRES
)