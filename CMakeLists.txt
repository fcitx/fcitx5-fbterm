cmake_minimum_required(VERSION 3.6)
project(fcitx5-fbterm)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Utils REQUIRED)
find_package(Fcitx5GClient REQUIRED)
include(GNUInstallDirs)

pkg_check_modules(Gio2 REQUIRED IMPORTED_TARGET "gio-2.0")

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

add_subdirectory(src)
