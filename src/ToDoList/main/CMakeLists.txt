idf_component_register(
    SRCS "ToDo.c" "ToDoDB.c" "ToDoRsc.c" "ToDoTransfer.c"
    REQUIRES "driver" "libpumpkin" "libpit" "fatfs" "esp_timer" "esp_wifi" "libmath"
)
