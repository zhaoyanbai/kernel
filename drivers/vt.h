/*
 * ------------------------------------------------------------------------
 *   File Name: vt.h
 *      Author: Zhao Yanbai
 *              2026-08-09 18:39:06 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <console.h>

// 暂不考虑别的类型的VC，直接按VGA来写

typedef struct vc vc_t;

// 前面几个给TTY用的VC，后面跟一个给AP用的VC
#define VC_FOR_TTY_COUNT 2
#define VC_ID_FOR_AP_ID (VC_FOR_TTY_COUNT)
#define VC_COUNT (VC_FOR_TTY_COUNT + 1)

enum {
    VT_BLACK = 0,
    VT_BLUE = 1,
    VT_GREEN = 2,
    VT_CYAN = 3,
    VT_RED = 4,
    VT_PURPLE = 5,
    VT_YELLOW = 6,
    VT_WHITE = 7,
};

struct vc {
    int id;

    int x;
    int y;

    int cols;
    int rows;

    uint8_t default_color;

    bool show_cursor;

    uint16_t* vram_vaddr;
};

bool vc_is_fg_vc(vc_t* vc);
bool vc_is_view_vc(vc_t* vc);
vc_t* vt_get_vc(int id);
vc_t* vt_get_ap_vc();
void vt_switch(int id);