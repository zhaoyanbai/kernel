/*
 * ------------------------------------------------------------------------
 *   File Name: vt.c
 *      Author: Zhao Yanbai
 *              2026-08-09 18:38:52 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include "vt.h"

void vga_init_vc(vc_t* vc);
void vga_switch(vc_t* vc);
void vga_write(vc_t* vc, const char* buf, size_t size);
uint8_t vga_make_attr(uint8_t fg, uint8_t bg, bool fg_highlight, bool bg_blink);
void vga_set_cursor_style(bool block);

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

// VT CONSOLE: 仅桥接console用
int vt_console_write(const char* buf, size_t size) {
    // 找到前台vt调用其write
    assert(fg_vc != NULL);
    assert(fg_vc->id < VC_FOR_TTY_COUNT);
    vga_write(fg_vc, buf, size);
    return 0;
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