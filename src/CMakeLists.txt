add_executable(fcitx5-fbterm fcitx5-fbterm.cpp imapi.cpp keycode.cpp keymap.cpp utils.cpp)

target_link_libraries(fcitx5-fbterm Fcitx5::Utils Fcitx5::GClient PkgConfig::Gio2)

install(TARGETS fcitx5-fbterm DESTINATION ${CMAKE_INSTALL_BINDIR})
