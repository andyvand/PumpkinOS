cmake_minimum_required(VERSION 3.13)

set(SO_PROJECT 1)
set(EXTRA_COMPONENT_DIRS "../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Launcher)

include(elf_loader)

project_so(Launcher)
