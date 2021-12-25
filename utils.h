//
// Created by duzhaokun on 2021/12/24.
//

#ifndef FCITX5_FBTERM_UTILS_H
#define FCITX5_FBTERM_UTILS_H

#include "imapi.h"
#include <iostream>
#include <vector>

#define fw (fbtermInfo.fontWidth)
#define fh (fbtermInfo.fontHeight)
#define hfh ((uint)(fh * 0.5))

void split(const std::string& s, std::vector<std::string>& sv, char delim);
ColorType stringToColorType(std::string& s);
std::string getAppName(const char* arg0);
void moveRectInScreen(Rectangle& rect);

#endif//FCITX5_FBTERM_UTILS_H
