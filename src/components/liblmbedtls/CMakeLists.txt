idf_component_register(
    SRCS "liblmbedtls.c"
    INCLUDE_DIRS "." "../../libpit"
    REQUIRES mbedtls esp_wifi libpit
)

target_compile_options(${COMPONENT_LIB} PRIVATE -DSYS_CPU=3 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="pit" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="ESP32" -DSOEXT=".so" -DLOGTRAP_SYS -DESP32 -Wno-multichar -Wno-format)
