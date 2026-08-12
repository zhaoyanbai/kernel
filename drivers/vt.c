/*
 * ------------------------------------------------------------------------
 *   File Name: vt.c
 *      Author: Zhao Yanbai
 *              2026-08-09 18:38:52 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include "vt.h"
#include <vga.h>

static vc_t vcs[VC_COUNT];
static vc_t* fg_vc = &vcs[0];
static vc_t* view_vc = &vcs[0];  // 当前显示的vC，因为AP的存在，当前显示的VC不一定是fg_vc,
                                 // fg_vc是前台VC，AP VC不参与这个概念
static const vc_t* ap_vc = &vcs[VC_ID_FOR_AP_ID];

bool vc_is_fg_vc(vc_t* vc) {
    return vc == fg_vc;
}

bool vc_is_view_vc(vc_t* vc) {
    return vc == view_vc;
}

int vt_write(vc_t* vc, const char* buf, size_t size) {
    return vga_write(vc, buf, size);
}

// VT CONSOLE: 仅桥接console用
int vt_console_write(const char* buf, size_t size) {
    // 找到前台vt调用其write
    assert(fg_vc != NULL);
    assert(fg_vc->id < VC_FOR_TTY_COUNT);
    return vt_write(fg_vc, buf, size);
}

int vt_console_setup(console_t* console) {
    return 0;
}

static console_t vt_console = {
    .name = "VT",
    .write = vt_console_write,
    .setup = vt_console_setup,
};

// INIT VT

void vt_init_vc(vc_t* vc) {
    assert(vc != NULL);

    vga_init_vc(vc);
}

vc_t* vt_get_vc(int id) {
    assert(id >= 0);
    assert(id < VC_COUNT);
    return &vcs[id];
}

vc_t* vt_get_ap_vc() {
    return &vcs[VC_ID_FOR_AP_ID];
}

void vt_switch(int id) {
    assert(id >= 0);
    assert(id < VC_COUNT);

    view_vc = &vcs[id];

    if (view_vc != ap_vc) {
        fg_vc = view_vc;
    }

    vga_switch(view_vc);
}

uint8_t vt_make_attr(uint8_t fg, uint8_t bg, bool highlight, bool blink) {
    return vga_make_attr(fg, bg, highlight, blink);
}

void init_vt() {
    vga_set_cursor_style(false);

    for (int i = 0; i < VC_COUNT; i++) {
        vc_t* vc = vcs + i;

        vc->id = i;

        if (i == VC_ID_FOR_AP_ID) {
            vc->default_color = vt_make_attr(VT_WHITE, VT_BLUE, true, false);
            vc->show_cursor = false;
        } else {
            vc->default_color = vt_make_attr(VT_GREEN, VT_BLACK, true, false);
            vc->show_cursor = true;
        }

        vt_init_vc(vc);
    }

    const int view_vc_id = 0;

    fg_vc = vcs + view_vc_id;
    view_vc = fg_vc;

    vt_switch(view_vc_id);

    register_console(&vt_console);
}

void vt_keyboard_input(uint8_t c) {
    assert(fg_vc != NULL);
    assert(fg_vc->id < VC_FOR_TTY_COUNT);

    tty_t* tty = fg_vc->tty;
    assert(tty != NULL);

    tty_input(tty, c);
}

void print_kernel_version(const char* version) {
    vc_t* vc = vt_get_vc(0);
    uint8_t color = vt_make_attr(VT_WHITE, VT_CYAN, true, false);

    // 清理第一行
    for (int i = 0; i < vc->cols; i++) {
        uint16_t c = (color << 8) | ' ';
        vc->vram_vaddr[i] = c;
    }

    // 打印版本号
    for (int i = 0; i < strlen(version); i++) {
        uint16_t c = (color << 8) | version[i];
        vc->vram_vaddr[i] = c;
    }
}

int vt_tty_write(tty_t* tty, const char* buf, size_t size) {
    assert(tty != NULL);

    vc_t* vc = tty->private;
    assert(vc != NULL);

    return vt_write(vc, buf, size);

    return 0;
}

tty_ops_t vt_tty_ops = {
    .write = vt_tty_write,
};