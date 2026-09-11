set (CMAKE_C_FLAGS "-DIS_LITTLE_ENDIAN=1 ${CMAKE_C_FLAGS}")

idf_component_register(
    SRCS "sys/sdl3/sdl3.c" "sys/sdl3/sdl-audio.c" "sys/sdl3/sdl-joystick.c" "sys/sdl3/keymap.c" "cpu.c" "debug.c" "emu.c" "events.c" "exports.c" "fastmem.c" "hw.c" "inflate.c" "keytable.c" "lcd.c" "lcdc.c" "loader.c" "main.c" "mem.c" "menu.c" "miniz_tinfl.c" "newsound.c" "palette.c" "path.c" "rccmds.c" "rcfile.c" "rckeys.c" "rcvars.c" "refresh.c" "rtc.c" "save.c" "sound.c" "split.c" "xz/xz_crc32.c" "xz/xz_crc64.c" "xz/xz_dec_lzma2.c" "xz/xz_dec_stream.c" "xz/xz_dec_bcj.c" "sys/esp/map.c" "sys/esp/nix.c"
    INCLUDE_DIRS "."
    REQUIRES "esp_bsp_generic" "nvs_flash" "spi_flash" "SDL"
)
