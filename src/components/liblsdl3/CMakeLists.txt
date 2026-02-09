idf_component_register(
    SRCS "liblsdl3.c"
    INCLUDE_DIRS "." "../../liblsdl3" "../../libpit" "../../libpumpkin" "../SDL/include" "../SDL/SDL/include"
    REQUIRES driver fatfs esp_timer esp_wifi
)

target_compile_options(${COMPONENT_LIB} PRIVATE -DSYS_CPU=3 -DSYS_SIZE=1 -DSYS_OS=1 -DSYS_ENDIAN=1 -DSYSTEM_NAME="pit" -DSYSTEM_VERSION="1.0" -DSYSTEM_OS="ESP32" -DSOEXT=".so" -DLOGTRAP_SYS -DESP32 -Wno-multichar -Wno-format)
