idf_component_register(
    SRCS "command.c" "deploy.c"
    REQUIRES "driver" "libpumpkin" "libpit" "fatfs" "esp_timer" "esp_wifi" "libmath"
)
