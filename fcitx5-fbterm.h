/*
* SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
*/

#ifndef FCITX5_FBTERM_FCITX5_FBTERM_H
#define FCITX5_FBTERM_FCITX5_FBTERM_H

#include "immessage.h"
#include <istream>

class GlobalValues {
public:
    uint fontWidth;
    uint fontHeight;
    uint halfFontHeight;
    uint screenWidth;
    uint screenHeight;
    unsigned cursorx, cursory;
};

extern GlobalValues globalValues;

#endif//FCITX5_FBTERM_FCITX5_FBTERM_H
