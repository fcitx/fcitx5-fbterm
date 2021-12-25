cmake_minimum_required(VERSION 3.0)
project(fcitx5-fbterm)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5GClient REQUIRED)

pkg_check_modules(GIO "gio-2.0" REQUIRED)

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

add_executable(fcitx5-fbterm fcitx5-fbterm.cpp imapi.c imapi.c immessage.h keycode.cpp keycode.h input_key.h keymap.cpp keymap.h utils.h utils.cpp fcitx5-fbterm.h)

target_include_directories(fcitx5-fbterm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FCITX5_CORE_INCLUDE_DIRS}
        ${FCITX5_UTILS_INCLUDE_DIRS}
        ${FCITX5_GCLIENT_INCLUDE_DIRS}
)

target_link_libraries(fcitx5-fbterm Fcitx5::Core Fcitx5::GClient ${GIO_LIBRARIES})
