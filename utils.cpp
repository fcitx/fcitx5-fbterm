/*
* SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
*/

#include "utils.h"
#include "fcitx5-fbterm.h"
#include <sstream>

void split(const std::string& s, std::vector<std::string>& sv, char delim) {
    sv.clear();
    std::istringstream iss(s);
    std::string temp;
    while (std::getline(iss, temp, delim)) {
        sv.push_back(std::move(temp));
    }
}

ColorType stringToColorType(std::string& s) {
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
    throw std::runtime_error("unknown color: " + s);
}

void moveRectInScreen(Rectangle& rect) {
    auto width = rect.w;
    auto height = rect.h;
    rect.x = cursorx + fw + width > sw ? cursorx - width - fw : cursorx + fw;
    rect.y = cursory + hfh + height > sh ? cursory - height - hfh * 3 : cursory + hfh;
}
