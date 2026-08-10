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

typedef struct vc vc_t;
typedef struct vt vt_t;
typedef struct vc_backend_t vc_backend_t;


#define VC_COUNT 4


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

// typedef uint8_t vt_color_t;

struct vc_backend_t {
    int (*init)(vc_t *vc);
    // int (*write)(const char *buf, size_t size);
    int (*switch)(vc_t *vc);
    int (*set_vram_addr)(vc_t *vc);
    int (*save_screen)(vc_t *vc);
    int (*clear)(vc_t *vc);
    uint8_t (*build_attr)(vc_t *vc, uint8_t bg_color, uint8_t fg_color, bool blink, bool highlight);

};
 
struct vc {
    int id;

    int x;
    int y;

    int cols;
    int rows;

    // vt_color_t fg_color;
    // vt_color_t bg_color;

    vc_backend_t *backend;

    uint16_t *vram_addr;
    uint16_t *vram;
    size_t vram_size;

    size_t bytes_per_row;
};




// struct vt {
//     vc_t vcs[VC_COUNT];
//     vc_t *fg_vc;
// };

bool vc_is_fg(vc_t *vc);