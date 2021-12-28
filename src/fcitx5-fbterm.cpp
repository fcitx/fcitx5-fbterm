/*
 * SPDX-FileCopyrightText: 2021~2021 duzhaokun123 <duzhaokun2@outlook.com>
 * SPDX-FileCopyrightText: 2010~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cstring>
#include <fcitx-gclient/fcitxgclient.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/keysymgen.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/stringutils.h>
#include <fcitx-utils/utf8.h>
#include <getopt.h>
#include <gio/gio.h>
#include "imapi.h"
#include "keycode.h"
#include "keymap.h"
#include "utils.h"

using namespace std;
using namespace fcitx;

namespace {

std::string preeditItemsToString(const GPtrArray *preedit) {
    std::string preeditString;
    for (guint i = 0; i < preedit->len; i++) {
        preeditString +=
            static_cast<FcitxGPreeditItem *>(g_ptr_array_index(preedit, 0))
                ->string;
    }
    return preeditString;
}

int is_double_width(uint32_t ucs) {
    static const std::tuple<uint32_t, uint32_t> double_width[] = {
        {0x1100, 0x115F}, {0x2329, 0x232A},   {0x2E80, 0x303E},
        {0x3040, 0xA4CF}, {0xAC00, 0xD7A3},   {0xF900, 0xFAFF},
        {0xFE10, 0xFE19}, {0xFE30, 0xFE6F},   {0xFF00, 0xFF60},
        {0xFFE0, 0xFFE6}, {0x20000, 0x2FFFD}, {0x30000, 0x3FFFD}};
    // this is tricky, upper_bound means, first item that larger than value.
    auto iter =
        std::upper_bound(std::begin(double_width), std::end(double_width), ucs,
                         [](uint32_t ucs, const auto &item) {
                             return ucs < std::get<1>(item) + 1;
                         });
    if (iter == std::end(double_width)) {
        return false;
    }
    return (ucs >= std::get<0>(*iter));
}

unsigned int text_width(std::string_view str) {
    int width = 0;
    for (auto c : utf8::MakeUTF8CharRange(str)) {
        if (is_double_width(c))
            width += 2;
        else
            width += 1;
    }
    return width;
}

void printUsage(std::string_view arg0) {
    std::cout << "Usage: " << arg0 << " [options]" << std::endl
              << "Options:" << std::endl
              << "  --help        show this message" << std::endl
              << "Envrionment variables:" << std::endl
              << "  FCITX5_FBTERM_FOREGROUND=<color> set text color"
              << std::endl
              << "  FCITX5_FBTERM_BACKGROUND=<color> set window color"
              << std::endl
              << "Color:" << std::endl
              << "  Black, DarkRed, DarkGreen, DarkYellow, DarkBlue, "
                 "DarkMagenta, DarkCyan, Gray,"
              << std::endl
              << "  DarkGray, Red, Green, Yellow, Blue, Magenta, Cyan, White"
              << std::endl;
}

} // namespace

class FcitxFbterm {

    enum {
        WINID_IM = 0,
        WINID_ERROR = 1,
    };

public:
    FcitxFbterm(int argc, char *argv[]);

    int exec();

private:
    void clearWin(int winid) {
        constexpr Rectangle rect0{0, 0, 0, 0};
        set_im_window(winid, rect0);
    }

    void moveRectInScreen(Rectangle &rect);

    void im_active();

    void im_deactive();

    void im_show();

    void im_hide();

    void process_raw_key(char *buf, unsigned int len);

    void cursor_pos_changed(unsigned x, unsigned y);

    void update_fbterm_info(::Info *info);

    void show_cannot_connect_error();

    gboolean socketCallback();

    void fcitx_fbterm_connect_cb();

    void fcitx_fbterm_commit_string_cb(const char *str);

    void fcitx_fbterm_current_im_cb();

    void fcitx_fbterm_update_client_side_ui_cb(
        GPtrArray *preedit, int cursorPos, GPtrArray *auxUp, GPtrArray *auxDown,
        GPtrArray *candidates, int highlight, int layoutHint, gboolean hasPrev,
        gboolean hasNext);

    UniqueCPtr<FcitxGClient, &g_object_unref> client_;
    UniqueCPtr<GIOChannel, &g_io_channel_unref> iochannel_;
    UniqueCPtr<GMainLoop, &g_main_loop_unref> mainloop_;

    unsigned fontWidth_;
    unsigned fontHeight_;
    unsigned halfFontHeight_;
    unsigned screenWidth_;
    unsigned screenHeight_;
    unsigned cursorx_, cursory_;

    static constexpr char useRawMode = 1;
    bool active_ = false;
    fcitx::KeyState state_;
    string textUp_;
    string textDown_;
    int cursorPos_ = -1;
    ColorType foreground_ = Black;
    ColorType background_ = Gray;
    bool quit_ = false;
};

FcitxFbterm::FcitxFbterm(int argc, char *argv[]) {

    const struct option longOptions[] = {{"help", no_argument, nullptr, 'h'}};
    int r;
    while ((r = getopt_long_only(argc, argv, "", longOptions, nullptr)) != -1) {
        switch (r) {
        case 'h':
        default:
            printUsage(argv[0]);
            quit_ = true;
            return;
        }
    }

    std::string_view background;
    if (auto *env = getenv("FCITX5_FBTERM_BACKGROUND")) {
        background = env;
    }
    std::string_view foreground;
    if (auto *env = getenv("FCITX5_FBTERM_FOREGROUND")) {
        foreground = env;
    }
    foreground_ = stringToColorType(foreground, Black);
    background_ = stringToColorType(background, Gray);

    auto imSocket = get_im_socket();
    if (imSocket == -1) {
        FCITX_ERROR()
            << "Can't not connect to fbterm, make sure start using `fbterm -i "
            << argv[0] << "` or in config file";
        quit_ = true;
        return;
    }

    iochannel_.reset(g_io_channel_unix_new(imSocket));
    g_io_add_watch(
        iochannel_.get(),
        static_cast<GIOCondition>(G_IO_IN | G_IO_HUP | G_IO_ERR),
        +[](GIOChannel *, GIOCondition, gpointer user_data) {
            return static_cast<FcitxFbterm *>(user_data)->socketCallback();
        },
        this);

    client_.reset(fcitx_g_client_new());
    fcitx_g_client_set_program(client_.get(), "fbterm");
    fcitx_g_client_set_display(client_.get(), "fbterm");

    g_signal_connect(
        client_.get(), "connected",
        G_CALLBACK(+[](FcitxGClient *, void *user_data) {
            static_cast<FcitxFbterm *>(user_data)->fcitx_fbterm_connect_cb();
        }),
        this);
    g_signal_connect(
        client_.get(), "commit-string",
        G_CALLBACK(+[](FcitxGClient *, char *str, void *user_data) {
            static_cast<FcitxFbterm *>(user_data)
                ->fcitx_fbterm_commit_string_cb(str);
        }),
        this);
    g_signal_connect(
        client_.get(), "current-im",
        G_CALLBACK(+[](FcitxGClient *, char *, char *, char *,
                       void *user_data) {
            static_cast<FcitxFbterm *>(user_data)->fcitx_fbterm_current_im_cb();
        }),
        this);
    g_signal_connect(
        client_.get(), "update-client-side-ui",
        G_CALLBACK(+[](FcitxGClient *, GPtrArray *preedit, int _cursorPos,
                       GPtrArray *auxUp, GPtrArray *auxDown,
                       GPtrArray *candidates, int highlight, int layoutHint,
                       gboolean hasPrev, gboolean hasNext, void *user_data) {
            static_cast<FcitxFbterm *>(user_data)
                ->fcitx_fbterm_update_client_side_ui_cb(
                    preedit, _cursorPos, auxUp, auxDown, candidates, highlight,
                    layoutHint, hasPrev, hasNext);
        }),
        this);

    ImCallbacks cbs = {
        [this]() { im_active(); }, // .active

        [this]() { im_deactive(); }, // .deactive
        [this](unsigned) { im_show(); },
        [this]() { im_hide(); },
        [this](char *keys, unsigned len) {
            process_raw_key(keys, len);
        }, // .send_key
        [this](unsigned x, unsigned y) {
            cursor_pos_changed(x, y);
        },                                                  // .cursor_position
        [this](::Info *info) { update_fbterm_info(info); }, // .fbterm_info
        [](char crlf, char appkey, char curo) {
            update_term_mode(crlf, appkey, curo);
        } // .term_mode
    };

    register_im_callbacks(cbs);
    connect_fbterm(useRawMode);

    mainloop_.reset(g_main_loop_new(nullptr, false));
}

int FcitxFbterm::exec() {
    if (quit_) {
        return 1;
    }
    g_main_loop_run(mainloop_.get());
    return 0;
}

void FcitxFbterm::moveRectInScreen(Rectangle &rect) {
    auto width = rect.w;
    auto height = rect.h;
    rect.x = cursorx_ + fontWidth_ + width > screenWidth_
                 ? cursorx_ - width - fontWidth_
                 : cursorx_ + fontWidth_;
    rect.y = cursory_ + halfFontHeight_ + height > screenHeight_
                 ? cursory_ - height - halfFontHeight_ * 3
                 : cursory_ + halfFontHeight_;
}

void FcitxFbterm::im_active() {
    if (useRawMode) {
        init_keycode_state();
    }
    active_ = true;
    if (fcitx_g_client_is_valid(client_.get())) {
        fcitx_g_client_focus_in(client_.get());
    }
}

void FcitxFbterm::im_deactive() {
    clearWin(WINID_IM);
    clearWin(WINID_ERROR);
    active_ = false;
    if (fcitx_g_client_is_valid(client_.get())) {
        fcitx_g_client_focus_out(client_.get());
    }
}

void FcitxFbterm::im_show() {
    clearWin(WINID_ERROR);
    if (textUp_.empty() && textDown_.empty()) {
        clearWin(WINID_IM);
        return;
    }

    auto width =
        (max(text_width(textUp_.c_str()), text_width(textDown_.c_str())) + 2) *
        fontWidth_;
    auto height = fontHeight_ * (textDown_.empty() ? 2 : 3);
    Rectangle rect;
    rect.w = width;
    rect.h = height;
    moveRectInScreen(rect);
    set_im_window(WINID_IM, rect);
    fill_rect(rect, background_);
    draw_text(rect.x + fontWidth_, rect.y + halfFontHeight_, foreground_,
              background_, textUp_.c_str(), textUp_.length());
    draw_text(rect.x + fontWidth_, rect.y + halfFontHeight_ * 3, foreground_,
              background_, textDown_.c_str(), textDown_.length());
    if (cursorPos_ >= 0) {
        auto charOffset =
            text_width(std::string_view(textUp_).substr(0, cursorPos_));
        Rectangle cursorRect = {rect.x + fontWidth_ * (1 + charOffset),
                                rect.y + halfFontHeight_, 1, fontHeight_};
        fill_rect(cursorRect, foreground_);
    }
}

void FcitxFbterm::im_hide() {}

void FcitxFbterm::process_raw_key(char *buf, unsigned int len) {
    auto notConnected = !fcitx_g_client_is_valid(client_.get());
    if (notConnected) {
        show_cannot_connect_error();
        clearWin(WINID_IM);
    }
    for (unsigned int i = 0; i < len; i++) {
        char down = !(buf[i] & 0x80);
        short code = buf[i] & 0x7f;

        if (!code) {
            if (i + 2 >= len)
                break;

            code = (buf[++i] & 0x7f) << 7;
            code |= buf[++i] & 0x7f;
            if (!(buf[i] & 0x80) || !(buf[i - 1] & 0x80))
                continue;
        }

        ushort linux_keysym = keycode_to_keysym(code, down);
        if (notConnected) {
            char *str = keysym_to_term_string(linux_keysym, down);
            put_im_text(str, strlen(str));
            return;
        }
        FcitxKeySym keysym = linux_keysym_to_fcitx_keysym(linux_keysym, code);

        fcitx_g_client_focus_in(client_.get());

        if (keysym == FcitxKey_None ||
            fcitx_g_client_process_key_sync(client_.get(), keysym, code,
                                            static_cast<guint32>(state_), !down,
                                            0) <= 0) {
            char *str = keysym_to_term_string(linux_keysym, down);
            if (str)
                put_im_text(str, strlen(str));
        }

        state_ = calculate_modifiers(state_, keysym, down);
    }
}

void FcitxFbterm::cursor_pos_changed(unsigned x, unsigned y) {
    cursorx_ = x;
    cursory_ = y;
    im_show();
}

void FcitxFbterm::update_fbterm_info(::Info *info) {
    fontWidth_ = info->fontWidth;
    fontHeight_ = info->fontHeight;
    halfFontHeight_ = info->fontHeight * 0.5;
    screenHeight_ = info->screenHeight;
    screenWidth_ = info->screenWidth;
    cursorx_ = 0;
    cursory_ = 0;
}

void FcitxFbterm::show_cannot_connect_error() {
    constexpr std::string_view msg =
        "ERROR: Can't connect to fcitx5! Is daemon running?";
    Rectangle rect = {0, 0, 0, 0};
    rect.w = text_width(msg.data() + 2) * fontWidth_;
    rect.h = fontHeight_ * 2;
    moveRectInScreen(rect);
    set_im_window(WINID_ERROR, rect);
    fill_rect(rect, Red);
    draw_text(rect.x + fontWidth_, rect.y + halfFontHeight_, White, Red,
              msg.data(), msg.size());
}

gboolean FcitxFbterm::socketCallback() {
    if (!check_im_message()) {
        g_main_loop_quit(mainloop_.get());
        return false;
    }
    return true;
}

void FcitxFbterm::fcitx_fbterm_connect_cb() {
    g_assert(fcitx_g_client_is_valid(client_.get()));
    fcitx_g_client_set_capability(
        client_.get(),
        static_cast<guint64>(fcitx::CapabilityFlag::ClientSideInputPanel));
    if (active_) {
        fcitx_g_client_focus_in(client_.get());
    }
}

void FcitxFbterm::fcitx_fbterm_commit_string_cb(const char *str) {
    put_im_text(str, strlen(str));
}

void FcitxFbterm::fcitx_fbterm_current_im_cb() {
    state_ = fcitx::KeyState::NoState;
}

void FcitxFbterm::fcitx_fbterm_update_client_side_ui_cb(
    GPtrArray *preedit, int cursorPos, GPtrArray *auxUp, GPtrArray *auxDown,
    GPtrArray *candidates, int highlight, int layoutHint, gboolean hasPrev,
    gboolean hasNext) {
    FCITX_UNUSED(hasPrev);
    FCITX_UNUSED(hasNext);
    FCITX_UNUSED(layoutHint);
    textUp_ = preeditItemsToString(auxUp);
    cursorPos_ = cursorPos;
    if (cursorPos_ >= 0) {
        cursorPos_ += textUp_.size();
    }
    textUp_ += preeditItemsToString(preedit);
    textDown_ = preeditItemsToString(auxDown);
    for (guint i = 0; i < candidates->len; i++) {
        const auto *item = static_cast<FcitxGCandidateItem *>(
            g_ptr_array_index(candidates, i));
        textDown_.append(static_cast<int>(i) == highlight ? "*" : " ");
        textDown_.append(item->label);
        textDown_.append(item->candidate);
    }
    im_show();
}

int main(int argc, char *argv[]) {
    FcitxFbterm fbterm(argc, argv);
    return fbterm.exec();
}
