idf_component_register(
    SRCS "Launcher.c" "editbin.c" "editbmp.c" "editform.c" "editreg.c" "editstr.c" "editsurf.c"
    REQUIRES "driver" "libpumpkin" "libpit" "fatfs" "esp_timer" "esp_wifi" "libmath"
)
