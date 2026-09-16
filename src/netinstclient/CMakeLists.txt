# netinstclient - desktop (Qt) client that installs applications on a
# PumpkinOS device over the network.  The server side is the "Net Install"
# PumpkinOS application in src/netinstserver.
#
#   cmake -S . -B build
#   cmake --build build
#
# With a vcpkg Qt add
#   -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.16)

project(netinstclient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(SOURCES
  src/main.cpp
  src/PrcInfo.cpp
  src/PrcInfo.h
  src/NetInstClient.cpp
  src/NetInstClient.h
  src/Discovery.cpp
  src/Discovery.h
  src/MainWindow.cpp
  src/MainWindow.h
  src/NetInst.rc
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
  qt_add_executable(netinstclient ${SOURCES})
else()
  add_executable(netinstclient ${SOURCES})
endif()

target_link_libraries(netinstclient PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Network
)

target_compile_definitions(netinstclient PRIVATE
  NETINSTCLIENT_VERSION="${PROJECT_VERSION}"
  QT_NO_CAST_FROM_ASCII_UNUSED
)

install(TARGETS netinstclient RUNTIME DESTINATION bin BUNDLE DESTINATION .)
