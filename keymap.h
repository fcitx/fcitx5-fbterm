/*
 * original from ibus-fbterm
 */

#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>

FcitxKeySym linux_keysym_to_fcitx_keysym(unsigned short keysym, unsigned short keycode);

fcitx::KeyState calculate_modifiers(fcitx::KeyState state, FcitxKeySym keyval, char down);