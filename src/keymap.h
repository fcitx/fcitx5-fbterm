/*
 * SPDX-FileCopyrightText: 2008~2010 dragchan <zgchan317@gmail.com>
 * SPDX-FileCopyrightText: 2021~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef _FCITX5_FBTERM_KEYMAP_H_
#define _FCITX5_FBTERM_KEYMAP_H_

#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>

FcitxKeySym linux_keysym_to_fcitx_keysym(unsigned short keysym,
                                         unsigned short keycode);

fcitx::KeyState calculate_modifiers(fcitx::KeyState state, FcitxKeySym keyval,
                                    char down);

#endif // _FCITX5_FBTERM_KEYMAP_H_
