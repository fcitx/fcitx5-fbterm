/*
* SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
*/

#ifndef FCITX5_FBTERM_UTILS_H
#define FCITX5_FBTERM_UTILS_H

#include "imapi.h"
#include <iostream>
#include <vector>

void split(const std::string& s, std::vector<std::string>& sv, char delim);
ColorType stringToColorType(std::string& s);
void moveRectInScreen(Rectangle& rect);

#endif//FCITX5_FBTERM_UTILS_H
