/*
 * ------------------------------------------------------------------------
 *   File Name: vt.c
 *      Author: Zhao Yanbai
 *              2026-08-09 18:38:52 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include "vt.h"


static vc_t vcs[VC_COUNT];
static vc_t *fg_vc = &vcs[0];

bool vc_is_fg(vc_t *vc) {
    return vc == fg_vc;
}

// // VT
// static vt_t vt = {
//     .vcs = {{0},},
//     .fg_vc = &vt.vcs[0],
// };



// VT CONSOLE: 仅桥接console用
int vt_console_write(const char *buf, size_t size) {
    //找到前台vt调用其write
    return 0;
}

int vt_console_setup(console_t *console) {
    return 0;
}


static console_t vt_console = {
    .name = "VT",
    .write = vt_console_write,
    .setup = vt_console_setup,
};


// INIT VT

void init_vt() {
    vc_backend_t *backend = &vga_backend;

    for (int i=0; i<VC_COUNT; i++) {
        memset(vcs+i, 0, sizeof(vc_t));
        vc_t *vc = vcs+i;

        backend->init(vc);
        
        
        vc->id = i;
        vc->backend = backend;

        vc->vram_size = VGA_WIDTH * VGA_HEIGHT * 2;
        vc->vram = kmalloc(PAGE_UP(vc->vram_size), 0);

        memset(vc->vram, 0, vc->vram_size);


        // int width = 0;
        // int height = 0;
        // assert(backend->get_size != NULL);
        // backend->get_size(&width, &height);

        // assert(width > 0);
        // assert(height > 0);

        // vc->cols = width;
        // vc->rows = height;
        // vc->x = 0;
        // vc->y = 0;
        // vc->fg_color = 0;
        // vc->bg_color = 0;
        // vc->vram = NULL;
    }

    vt.fg_vc = &vt.vcs[0];


    register_console(&vt_console);
}