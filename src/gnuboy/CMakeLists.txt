cmake_minimum_required(VERSION 3.13)

if(ESP_PLATFORM)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(gnuboy)

get_filename_component(configName "${CMAKE_BINARY_DIR}" NAME)

list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/components/esp_littlefs")

littlefs_create_partition_image(storage data FLASH_IN_PROJECT)
else()
project(sdl3gnuboy)

set(TARGET_NAME sdl3gnuboy)

#If Linux / UNIX / BSD / APPLE / etc...
if(UNIX OR APPLE) 
        set(PLATFORM_IMPL sys/nix/map.c sys/nix/nix.c)
#If not unix probably windows?
else() 
        set(PLATFORM_IMPL sys/windows/windows.c GNUBoy.rc)
        add_definitions(-DALT_PATH_SEP)
endif()

find_package(SDL3 REQUIRED)

set(CMAKE_C_STANDARD 99)

if(UNIX OR APPLE) 
set(CMAKE_C_FLAGS "-Wall")

set(CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS_DEBUG "-g3")
endif()

include_directories(./include)
include_directories(./lib)
include_directories(./)
include_directories("/usr/local/include/")

include_directories(${SDL2_INCLUDE_DIR})

add_definitions(-DIS_LITTLE_ENDIAN -D_GNU_SOURCE=1 -D_REENTRANT)

add_executable(${TARGET_NAME} sys/sdl3/sdl3.c sys/sdl3/sdl-audio.c sys/sdl3/sdl-joystick.c sys/sdl3/keymap.c cpu.c debug.c emu.c events.c exports.c fastmem.c hw.c inflate.c keytable.c lcd.c lcdc.c loader.c main.c mem.c menu.c miniz_tinfl.c newsound.c palette.c path.c rccmds.c rcfile.c rckeys.c rcvars.c refresh.c rtc.c save.c sound.c split.c xz/xz_crc32.c xz/xz_crc64.c xz/xz_dec_lzma2.c xz/xz_dec_stream.c xz/xz_dec_bcj.c ${PLATFORM_IMPL})

target_link_libraries(${TARGET_NAME} ${SDL3_LIBRARIES})
endif()
