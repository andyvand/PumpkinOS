file(GLOB_RECURSE SRCS src/*.c)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver spiffs esp_littlefs fatfs esp_driver_i2s
    PRIV_REQUIRES esp_lcd
)
