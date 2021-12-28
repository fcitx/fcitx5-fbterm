/*
 * SPDX-FileCopyrightText: 2008~2010 dragchan <zgchan317@gmail.com>
 * SPDX-FileCopyrightText: 2021~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef _FCITX5_FBTERM_INPUT_KEY_H_
#define _FCITX5_FBTERM_INPUT_KEY_H_

#include <linux/keyboard.h>

enum Keys {
    AC_START = K(KT_LATIN, 0x80),
    SHIFT_PAGEUP = AC_START,
    SHIFT_PAGEDOWN,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    CTRL_SPACE,
    CTRL_ALT_1,
    CTRL_ALT_2,
    CTRL_ALT_3,
    CTRL_ALT_4,
    CTRL_ALT_5,
    CTRL_ALT_6,
    CTRL_ALT_7,
    CTRL_ALT_8,
    CTRL_ALT_9,
    CTRL_ALT_0,
    CTRL_ALT_C,
    CTRL_ALT_D,
    CTRL_ALT_E,
    CTRL_ALT_F1,
    CTRL_ALT_F2,
    CTRL_ALT_F3,
    CTRL_ALT_F4,
    CTRL_ALT_F5,
    CTRL_ALT_F6,
    CTRL_ALT_K,
    AC_END = CTRL_ALT_K
};

#endif // _FCITX5_FBTERM_INPUT_KEY_H_
