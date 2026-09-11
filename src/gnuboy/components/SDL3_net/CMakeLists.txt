idf_component_register(
    SRCS "SDLnet.c" "SDLnetselect.c" "SDLnetTCP.c" "SDLnetUDP.c"
    INCLUDE_DIRS "." "../gnuboy" "../SDL/include" "../SDL/SDL/include"
    REQUIRES driver fatfs esp_timer esp_wifi
)
