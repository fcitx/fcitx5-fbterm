cmake_minimum_required(VERSION 3.0)
project(fcitx5-fbterm)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5GClient REQUIRED)

pkg_check_modules(Gio2 REQUIRED IMPORTED_TARGET "gio-2.0")

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

add_executable(fcitx5-fbterm fcitx5-fbterm.cpp imapi.cpp imapi.cpp immessage.h keycode.cpp keycode.h input_key.h keymap.cpp keymap.h utils.h utils.cpp fcitx5-fbterm.h)

target_link_libraries(fcitx5-fbterm Fcitx5::Core Fcitx5::GClient PkgConfig::Gio2)
