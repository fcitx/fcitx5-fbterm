cmake_minimum_required(VERSION 3.0)
project(fcitx5-fbterm)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FCITX5_CORE "Fcitx5Core" REQUIRED)
pkg_check_modules(FCITX5_UTILS "Fcitx5Utils" REQUIRED)
pkg_check_modules(FCITX5_GCLIENT "Fcitx5GClient" REQUIRED)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FCITX5_CORE_INCLUDE_DIRS}
        ${FCITX5_UTILS_INCLUDE_DIRS}
        ${FCITX5_GCLIENT_INCLUDE_DIRS}
)

link_libraries(
        ${FCITX5_CORE_LIBRARIES}
        ${FCITX5_UTILS_LIBRARIES}
        ${FCITX5_GCLIENT_LIBRARIES}
)

add_executable(fcitx5-fbterm fcitx5-fbterm.cpp imapi.c imapi.c immessage.h keycode.c keycode.h input_key.h keymap.cpp keymap.h utils.h utils.cpp fcitx5-fbterm.h)
