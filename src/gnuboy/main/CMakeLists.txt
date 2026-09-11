idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES "esp_bsp_generic" "gnuboy" "SDL"
    PRIV_REQUIRES "app_update" "nvs_flash" "esp_adc"
)
