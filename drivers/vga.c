/*
 * ------------------------------------------------------------------------
 *   File Name: vga.c
 *      Author: Zhao Yanbai
 *              2026-08-10 09:01:44 Monday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include "vga.h"

static uint16_t *vga_vram_vaddr = (uint16_t *)pa2va(0xB8000);

// VGA VC backend

// static int vga_write(const char *buf, size_t size) {
//     return 0;
// }

// static int vga_get_size(int *width, int *height) {
//     *width = VGA_WIDTH;
//     *height = VGA_HEIGHT;
//     return 0;
// }

static int vga_init(vc_t *vc) {
    vc->cols = VGA_WIDTH;
    vc->rows = VGA_HEIGHT;
    return 0;
}

static int vga_switch(vc_t *vc) {

    memcpy(vc->vram, vga_vram_vaddr, vc->vram_size);

    return 0;
}

static int vga_set_vram_addr(vc_t *vc) {

    vc->vram_addr = vga_vram_vaddr;

    return 0;
}

static int vga_save_screen(vc_t *vc) {

    memcpy(vc->vram, vga_vram_vaddr, vc->vram_size);

    return 0;
}

static int vga_clear(vc_t *vc) {
 //   memset(vc->vram, 0, vc->vram_size);
    for (int i = 0; i < vc->vram_size; i++) {
        vc->vram[i] = 0 | ;
    }
    return 0;
}

static uint8_t vga_build_attr(vc_t *vc, uint8_t bg_color, uint8_t fg_color, bool blink, bool highlight) {
    uint8_t bgc = bg_color;
    uint8_t fgc = fg_color;
    return (bgc << 4) | fgc | (blink ? 0b1000 : 0) | (highlight ? 0b10000 : 0);
}

static vc_backend_t vga_backend = {
    .init = vga_init,
    // .write = vga_write,
    // .get_size = vga_get_size,
    .switch = vga_switch,
    .set_vram_addr = vga_set_vram_addr,
    .save_screen = vga_save_screen,
};
