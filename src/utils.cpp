/*
 * SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "utils.h"
#include <memory>
#include <string>
#include <fcitx-utils/log.h>
#include <glib.h>

ColorType stringToColorType(std::string_view s, ColorType fallback) {
#define COLOR_NAME(NAME)                                                       \
    { FCITX_STRINGIFY(NAME), NAME }
    static std::tuple<std::string, ColorType> colorNames[] = {
        COLOR_NAME(Black),      COLOR_NAME(DarkRed),  COLOR_NAME(DarkGreen),
        COLOR_NAME(DarkYellow), COLOR_NAME(DarkBlue), COLOR_NAME(DarkMagenta),
        COLOR_NAME(DarkCyan),   COLOR_NAME(Gray),     COLOR_NAME(DarkGray),
        COLOR_NAME(Red),        COLOR_NAME(Green),    COLOR_NAME(Yellow),
        COLOR_NAME(Blue),       COLOR_NAME(Magenta),  COLOR_NAME(Cyan),
        COLOR_NAME(White),
    };
#undef COLOR_NAME
    if (auto iter = std::find_if(
            std::begin(colorNames), std::end(colorNames),
            [s](const auto &item) { return std::get<0>(item) == s; });
        iter != std::end(colorNames)) {
        return std::get<1>(*iter);
    }
    return fallback;
}
