/*
 * SPDX-FileCopyrightText: 2008~2010 dragchan <zgchan317@gmail.com>
 * SPDX-FileCopyrightText: 2021~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef _FCITX5_FBTERM_KEYCODE_H_
#define _FCITX5_FBTERM_KEYCODE_H_

void init_keycode_state();

void update_term_mode(char crlf, char appkey, char curo);

unsigned short keycode_to_keysym(unsigned short keycode, char down);

unsigned short keypad_keysym_redirect(unsigned short keysym);

char *keysym_to_term_string(unsigned short keysym, char down);

#endif // _FCITX5_FBTERM_KEYCODE_H_
