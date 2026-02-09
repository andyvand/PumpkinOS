cmake_minimum_required(VERSION 3.13)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(PumpkinOS)

include(elf_loader)

get_filename_component(configName "${CMAKE_BINARY_DIR}" NAME)

list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/components/esp_littlefs")

littlefs_create_partition_image(storage data FLASH_IN_PROJECT)
