/*
* SPDX-FileCopyrightText: 2021 duzhaokun123 <duzhaokun2@outlook.com>
*
* SPDX-License-Identifier: GPL-3.0-or-later
*
*/

#include "fcitx5-fbterm.h"
#include "imapi.h"
#include "keycode.h"
#include "keymap.h"
#include "utils.h"
#include <cstring>
#include <fcitx-gclient/fcitxgclient.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/keysymgen.h>
#include <fcitx-utils/fs.h>
#include <gio/gio.h>
#include <iconv.h>

#define eprintf(args...) fprintf(stderr, args)
#define getArg1(i) getArg3(argc, argv, i)
#define clearWin(id) set_im_window(id, rect0)
#define BUF_SIZE 8192
#define WINID_IM 0
#define WINID_ERROR 1

using namespace std;

static char raw_mode = 1;
static FcitxGClient* client;
static GMainLoop* main_loop;
static bool active = false;
static fcitx::KeyState state;
static char textup[BUF_SIZE];
static char textdown[BUF_SIZE];
static char imname[BUF_SIZE];
static iconv_t iconvW;
static int cursorPos = -1;
static uint auxUpWidth;
ColorType fc = Black;
ColorType bc = Gray;
static const auto rect0 = Rectangle {0, 0, 0, 0};
unsigned cursorx, cursory;
uint fw;
uint fh;
uint hfh;
uint sw;
uint sh;

struct interval {
    unsigned first;
    unsigned last;
};

char* getArg3(int argc, char* argv[], int i) {
    if (i >= argc) return "";
    return argv[i];
}

void* fcitx_utils_malloc0(size_t bytes) {
    void* p = malloc(bytes);
    if (!p)
        return nullptr;

    memset(p, 0, bytes);
    return p;
}

static int bisearch(unsigned ucs, const struct interval* table, unsigned max) {
    unsigned min = 0;
    unsigned mid;

    if (ucs < table[0].first || ucs > table[max].last)
        return 0;
    while (max >= min) {
        mid = (min + max) / 2;
        if (ucs > table[mid].last)
            min = mid + 1;
        else if (ucs < table[mid].first)
            max = mid - 1;
        else
            return 1;
    }
    return 0;
}

static int is_double_width(unsigned ucs) {
    ucs = be32toh(ucs);
    static const struct interval double_width[] = {
            {0x1100, 0x115F},
            {0x2329, 0x232A},
            {0x2E80, 0x303E},
            {0x3040, 0xA4CF},
            {0xAC00, 0xD7A3},
            {0xF900, 0xFAFF},
            {0xFE10, 0xFE19},
            {0xFE30, 0xFE6F},
            {0xFF00, 0xFF60},
            {0xFFE0, 0xFFE6},
            {0x20000, 0x2FFFD},
            {0x30000, 0x3FFFD}};
    return bisearch(ucs, double_width, sizeof(double_width) / sizeof(struct interval) - 1);
}

static unsigned int text_width(char* str) {
    if (iconvW == nullptr)
        iconvW = iconv_open("ucs-4be", "utf-8");
    if (iconvW == (iconv_t)-1) {
        return g_utf8_strlen(str, INT_MAX);
    } else {
        size_t len = strlen(str);
        size_t charlen = g_utf8_strlen(str, INT_MAX);
        unsigned* wmessage;
        size_t wlen = (len + 1) * sizeof(unsigned);
        wmessage = (unsigned*)fcitx_utils_malloc0((len + 1) * sizeof(unsigned));

        char* inp = str;
        char* outp = (char*)wmessage;

        iconv(iconvW, &inp, &len, &outp, &wlen);

        int i = 0;
        int width = 0;
        for (i = 0; i < charlen; i++) {
            if (is_double_width(wmessage[i]))
                width += 2;
            else
                width += 1;
        }


        free(wmessage);
        return width;
    }
}

static void show_cannot_connect_error() {
    auto msg = "ERROR: Can't connect to fcitx5! Is daemon running?";
    Rectangle rect;
    rect.w = (text_width((char*)msg) + 2) * fw;
    rect.h = fh * 2;
    moveRectInScreen(rect);
    set_im_window(WINID_ERROR, rect);
    fill_rect(rect, Red);
    draw_text(rect.x + fw, rect.y + hfh, White, Red, msg, strlen(msg));
}

static void im_active() {
    if (raw_mode) {
        init_keycode_state();
    }
    // don't focus in here, not connected yet
    // fcitx_g_client_focus_in(client);
    active = true;
}

static void im_deactive() {
    clearWin(WINID_IM);
    clearWin(WINID_ERROR);
    active = false;
}

static void im_show(unsigned) {
    clearWin(WINID_ERROR);
    if (strlen(textup) == 0) {
        clearWin(WINID_IM);
        return;
    }

    auto width = (max(text_width(textup), text_width(textdown)) + 2) * fw;
    auto height = fh * (strlen(textdown) != 0 ? 3 : 2);
    Rectangle rect;
    rect.w = width;
    rect.h = height;
    moveRectInScreen(rect);
    set_im_window(WINID_IM, rect);
    fill_rect(rect, bc);
    draw_text(rect.x + fw, rect.y + hfh, fc, bc, textup, strlen(textup));
    draw_text(rect.x + fw, rect.y + hfh * 3, fc, bc, textdown, strlen(textdown));
    if (cursorPos != -1) {
        draw_text(rect.x + fw * (1 + cursorPos + auxUpWidth), rect.y + hfh, fc, bc,
                  textup[cursorPos + auxUpWidth] ? &textup[cursorPos + auxUpWidth] : " ", 1);
    }
}

static void im_hide() {}

static void process_raw_key(char* buf, unsigned int len) {
    auto notConnected = !fcitx_g_client_is_valid(client);
    if (notConnected) {
        show_cannot_connect_error();
        clearWin(WINID_IM);
    }
    unsigned i;
    for (i = 0; i < len; i++) {
        char down = !(buf[i] & 0x80);
        short code = buf[i] & 0x7f;

        if (!code) {
            if (i + 2 >= len) break;

            code = (buf[++i] & 0x7f) << 7;
            code |= buf[++i] & 0x7f;
            if (!(buf[i] & 0x80) || !(buf[i - 1] & 0x80)) continue;
        }

        ushort linux_keysym = keycode_to_keysym(code, down);
        if (notConnected) {
            char *str = keysym_to_term_string(linux_keysym, down);
            put_im_text(str, strlen(str));
            return;
        }
        FcitxKeySym keysym = linux_keysym_to_fcitx_keysym(linux_keysym, code);

        fcitx_g_client_focus_in(client);

        if (keysym == FcitxKey_None || fcitx_g_client_process_key_sync(client, keysym, code,
                                                                       static_cast<guint32>(state), !down, 0) <= 0) {
            char* str = keysym_to_term_string(linux_keysym, down);
            if (str)
                put_im_text(str, strlen(str));
        }

        state = calculate_modifiers(state, keysym, down);
    }
}

static void cursor_pos_changed(unsigned x, unsigned y) {
    cursorx = x;
    cursory = y;
    im_show(-1);
}

static void update_fbterm_info(Info* info) {
    fh = info->fontHeight;
    fw = info->fontWidth;
    sh = info->screenHeight;
    sw = info->screenWidth;
    hfh = (uint)(fh * 0.5);
}

static ImCallbacks cbs = {
        im_active,  // .active
        im_deactive,// .deactive
        im_show,
        im_hide,
        process_raw_key,   // .send_key
        cursor_pos_changed,// .cursor_position
        update_fbterm_info,// .fbterm_info
        update_term_mode   // .term_mode
};

void fcitx_fbterm_connect_cb(FcitxGClient*, void*) {
    g_assert(fcitx_g_client_is_valid(client));
    fcitx_g_client_set_capability(client, static_cast<guint64>(fcitx::CapabilityFlag::ClientSideInputPanel));
}

void fcitx_fbterm_commit_string_cb(FcitxGClient*, char* str, void*) {
    put_im_text(str, strlen(str));
}

void fcitx_fbterm_current_im_cb(FcitxGClient*, char* name, char* uniqe_name, char* lang_code, gint type, void*) {
    state = fcitx::KeyState::NoState;
    //    snprintf(imname, BUF_SIZE, "%s", name);
    //    im_show(-1);
}

void fcitx_fbterm_update_client_side_ui_cb_parsed(const char* preedit, int _cursorPos, const char* auxUp,
                                                  const char* auxDown, const vector<FcitxGCandidateItem*>& candidates,
                                                  int highlight, bool hasPrev, bool hasNext) {
    cursorPos = _cursorPos;
    snprintf(textup, BUF_SIZE, "%s%s", auxUp, preedit);
    auto down = string();
    down.append(auxDown);
    for (int i = 0; i < candidates.size(); i++) {
        down.append(i == highlight ? "*" : " ");
        auto idx = to_string(i <= 8 ? i + 1 : i - 9);
        down.append(idx + ".");
        down.append(candidates[i]->candidate);
    }
    snprintf(textdown, BUF_SIZE, "%s", down.c_str());
    textup[BUF_SIZE - 1] = textdown[BUF_SIZE - 1] = '\0';
    auxUpWidth = text_width((char*)auxUp);
    im_show(-1);
}

void fcitx_fbterm_update_client_side_ui_cb(FcitxGClient*, GPtrArray* preedit, int _cursorPos, GPtrArray* auxUp,
                                           GPtrArray* auxDown, GPtrArray* candidates, int highlight,
                                           int /* layoutHint */, gboolean hasPrev, gboolean hasNext,
                                           void*) {
    auto _preedit = preedit->len > 0 ? ((FcitxGPreeditItem*)g_ptr_array_index(preedit, 0))->string : "";
    auto _auxUp = auxUp->len > 0 ? ((FcitxGPreeditItem*)g_ptr_array_index(auxUp, 0))->string : "";
    auto _auxDown = auxDown->len > 0 ? ((FcitxGPreeditItem*)g_ptr_array_index(auxDown, 0))->string : "";
    auto _candidates = vector<FcitxGCandidateItem*>();
    for (guint i = 0; i < candidates->len; i++) {
        _candidates.push_back((FcitxGCandidateItem*)g_ptr_array_index(candidates, i));
    }
    fcitx_fbterm_update_client_side_ui_cb_parsed(_preedit, _cursorPos, _auxUp, _auxDown, _candidates, highlight, hasPrev, hasNext);
}

static gboolean iochannel_fbterm_callback(GIOChannel*, GIOCondition, gpointer) {
    if (!check_im_message()) {
        g_main_loop_quit(main_loop);
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (strcmp(getArg1(1), "--help") == 0) {
        printf("Usage: %s [options]\n", getArg1(0));
        printf("Options:\n"
               "  --help        show this message\n"
               "  --skip-fbterm skip fbterm connect check\n");
        printf("You can create a link (not alias) to set theme:\n"
               "  fcitx5-fbterm-<ForegroundColor>-<BackgroundColor>\n"
               "  foreground color default: Black\n"
               "  background color default: Gray\n");
        printf("Color:\n"
               "  Black, DarkRed, DarkGreen, DarkYellow, DarkBlue, DarkMagenta, DarkCyan, Gray,\n"
               "  DarkGray, Red, Green, Yellow, Blue, Magenta, Cyan, White\n");
        return 0;
    }
    if (get_im_socket() == -1 && strcmp(getArg1(1), "--skip-fbterm") != 0) {
        eprintf("can't not connect to fbterm, make sure start using `fbterm -i %s` or in config file.\n", getArg1(0));
        return -1;
    }
    auto s = vector<string>();
    split(fcitx::fs::baseName(argv[0]), s, '-');
    if (s.size() != 2 && s.size() != 4) {
        eprintf("Warning: unknown name %s", getArg1(0));
        sleep(1);
    }
    if (s.size() == 4) {
        fc = stringToColorType(s[2]);
        bc = stringToColorType(s[3]);
    }

    GIOChannel* iochannel_fbterm = g_io_channel_unix_new(get_im_socket());
    g_io_add_watch(iochannel_fbterm, (GIOCondition)(G_IO_IN | G_IO_HUP | G_IO_ERR),
                   (GIOFunc)iochannel_fbterm_callback,
                   nullptr);

    client = fcitx_g_client_new();
    fcitx_g_client_set_program(client, "fbterm");
    fcitx_g_client_set_display(client, "fbterm:");

    g_signal_connect(client, "connected", G_CALLBACK(fcitx_fbterm_connect_cb), nullptr);
    g_signal_connect(client, "commit-string", G_CALLBACK(fcitx_fbterm_commit_string_cb), nullptr);
    g_signal_connect(client, "current-im", G_CALLBACK(fcitx_fbterm_current_im_cb), nullptr);
    g_signal_connect(client, "update-client-side-ui", G_CALLBACK(fcitx_fbterm_update_client_side_ui_cb), nullptr);

    register_im_callbacks(cbs);
    connect_fbterm(raw_mode);

    main_loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(main_loop);

    g_object_unref(client);
    g_io_channel_unref(iochannel_fbterm);
    g_main_loop_unref(main_loop);

    return 0;
}
