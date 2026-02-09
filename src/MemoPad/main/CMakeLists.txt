idf_component_register(
    SRCS "MemoDB.c" "MemoMain.c" "MemoRsc.c" "MemoTransfer.c"
    REQUIRES "driver" "libpumpkin" "libpit" "fatfs" "esp_timer" "esp_wifi" "libmath"
)
