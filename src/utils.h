/*
 * SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef FCITX5_FBTERM_UTILS_H
#define FCITX5_FBTERM_UTILS_H

#include <iostream>
#include <vector>
#include "imapi.h"

ColorType stringToColorType(std::string_view s, ColorType fallback);

#endif // FCITX5_FBTERM_UTILS_H
