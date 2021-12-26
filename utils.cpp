/*
* SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
*/

#include "utils.h"
#include "fcitx5-fbterm.h"
#include <glib.h>
#include <memory>
#include <string>

ColorType stringToColorType(std::string &s) {
    if (s == "Black") return Black;
    if (s == "DarkRed") return DarkRed;
    if (s == "DarkGreen") return DarkGreen;
    if (s == "DarkYellow") return DarkYellow;
    if (s == "DarkBlue") return DarkBlue;
    if (s == "DarkMagenta") return DarkMagenta;
    if (s == "DarkCyan") return DarkCyan;
    if (s == "Gray") return Gray;
    if (s == "DarkGray") return DarkGray;
    if (s == "Red") return Red;
    if (s == "Green") return Green;
    if (s == "Yellow") return Yellow;
    if (s == "Blue") return Blue;
    if (s == "Magenta") return Magenta;
    if (s == "Cyan") return Cyan;
    if (s == "White") return White;
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "unknown color: %s\n", s.c_str());
    exit(-1);
}

void moveRectInScreen(Rectangle &rect) {
    auto width = rect.w;
    auto height = rect.h;
    rect.x = globalValues.cursorx + globalValues.fontWidth + width > globalValues.screenWidth ?
             globalValues.cursorx - width - globalValues.fontWidth :
             globalValues.cursorx + globalValues.fontWidth;
    rect.y = globalValues.cursory + globalValues.halfFontHeight + height > globalValues.screenHeight ?
             globalValues.cursory - height - globalValues.halfFontHeight * 3 :
             globalValues.cursory + globalValues.halfFontHeight;
}
