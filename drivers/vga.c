/*
 * ------------------------------------------------------------------------
 *   File Name: vga.c
 *      Author: Zhao Yanbai
 *              2026-08-10 09:01:44 Monday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include "vt.h"
#include <vga.h>
#include <io.h>
#include <irq.h>

static uint16_t* vga_vram_base_vaddr = (uint16_t*)pa2va(vga_vram_base_paddr);

uint8_t vga_make_attr(uint8_t fg, uint8_t bg, bool fg_highlight, bool bg_blink) {
    assert((fg & 0x7) == fg);
    assert((bg & 0x7) == bg);

    // 目前VT_COLOR和VGA_COLOR是一一对应的
    // 所以直接处理

    uint8_t attr = 0;
    attr |= bg << 4;
    attr |= fg << 0;

    attr |= (bg_blink ? VGA_BG_BLINK : 0) << 4;
    attr |= (fg_highlight ? VGA_FG_HIGHLIGHT : 0) << 0;

    return attr;
}

static uint16_t* _vga_get_vram_vaddr(vc_t* vc) {
    unsigned long addr = (unsigned long)vga_vram_base_vaddr;
    addr += (vc->id * PAGE_UP(vga_vram_byte_size));
    return (uint16_t*)addr;
}

static void _vga_clear(vc_t* vc, int x, int y, size_t size) {
    assert(x >= 0);
    assert(y >= 0);
    assert(x < vc->cols);
    assert(y < vc->rows);

    const size_t begin_offset = y * vc->cols + x;
    const size_t end_offset = begin_offset + size;
    assert(begin_offset <= vga_vram_size);
    assert(end_offset <= vga_vram_size);

    uint16_t* vram_vaddr = vc->vram_vaddr;

    for (size_t i = begin_offset; i < end_offset; i++) {
        vram_vaddr[i] = (vc->default_color << 8) | 0;
    }
}

void vga_clear(vc_t* vc) {
    _vga_clear(vc, 0, 0, vga_vram_size);
}

void vga_init_vc(vc_t* vc) {
    assert(vc != NULL);
    assert(vc->id >= 0);

    vc->cols = vga_cols;
    vc->rows = vga_rows;
    vc->x = 0;
    vc->y = 0;
    vc->vram_vaddr = _vga_get_vram_vaddr(vc);

    printk("vga_init_vc: id=%d, vram_vaddr=%08x\n", vc->id, vc->vram_vaddr);

    vga_clear(vc);
}

void vga_clear_line(vc_t* vc, int y) {
    assert(y >= 0);
    assert(y < vc->rows);
    _vga_clear(vc, 0, y, vc->cols);
}

void vga_scroll_up(vc_t* vc) {
    // assert(vc->y == vc->rows);
    assert(vc->x < vc->cols);
    assert(vc->x >= 0);

    uint16_t* vram_vaddr = vc->vram_vaddr;

    // 如果是VC0则最顶上一行保留用来显示内核版本及编译时间信息
    const int keep = vc->id == 0 ? vc->cols : 0;

    for (int i = keep; i < ((vc->rows - 1) * vc->cols); i++) {
        vram_vaddr[i] = vram_vaddr[i + vc->cols];
    }

    _vga_clear(vc, 0, (vc->rows - 1), vc->cols);

    vc->y = vc->rows - 1;
}

off_t vga_get_offset(vc_t* vc) {
    return vc->y * vc->cols + vc->x;
}

#define VGA_CRTC_ADDR 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_CRTC_CURSOR_START 0x0A
#define VGA_CRTC_CURSOR_END 0x0B
#define VGA_CRTC_START_ADDR_H 0x0C
#define VGA_CRTC_START_ADDR_L 0x0D
#define VGA_CRTC_CURSOR_H 0x0E
#define VGA_CRTC_CURSOR_L 0x0F

#define VGA_CURSOR_DISABLE (1 << 5)

void _vga_cursor(bool show) {
    unsigned long eflags;
    irq_save(eflags);

    outb(VGA_CRTC_CURSOR_START, VGA_CRTC_ADDR);
    uint8_t start = inb(VGA_CRTC_DATA);

    if (show) {
        start &= ~VGA_CURSOR_DISABLE;
    } else {
        start |= VGA_CURSOR_DISABLE;
    }
    outb(VGA_CRTC_CURSOR_START, VGA_CRTC_ADDR);
    outb(start, VGA_CRTC_DATA);

    irq_restore(eflags);
}

void vga_set_cursor_style(bool block) {
    unsigned long eflags;
    irq_save(eflags);

    outb(VGA_CRTC_CURSOR_START, VGA_CRTC_ADDR);
    uint8_t start = inb(VGA_CRTC_DATA);

    start &= 0xF0;
    if (block) {
        start |= 0x00;
    } else {
        start |= 0x0E;
    }

    outb(VGA_CRTC_CURSOR_START, VGA_CRTC_ADDR);
    outb(start, VGA_CRTC_DATA);

    outb(VGA_CRTC_CURSOR_END, VGA_CRTC_ADDR);
    uint8_t end = inb(VGA_CRTC_DATA);

    end &= 0xF0;
    if (block) {
        end |= 0x0F;
    } else {
        end |= 0x0F;
    }

    outb(VGA_CRTC_CURSOR_END, VGA_CRTC_ADDR);
    outb(end, VGA_CRTC_DATA);

    irq_restore(eflags);
}

void vga_set_cursor(vc_t* vc) {
    assert(vc->x < vc->cols);
    assert(vc->y < vc->rows);
    assert(vc->x >= 0);
    assert(vc->y >= 0);

    if (!vc_is_view_vc(vc)) {
        return;
    }

    if (!vc->show_cursor) {
        return;
    }

    off_t offset = vga_get_offset(vc);
    // offset += 1;
    assert(offset < vga_vram_size);

    offset += (vc->vram_vaddr - vga_vram_base_vaddr);

    unsigned long eflags;
    irq_save(eflags);

    outb(VGA_CRTC_CURSOR_H, VGA_CRTC_ADDR);
    outb((offset >> 8) & 0xFF, VGA_CRTC_DATA);
    outb(VGA_CRTC_CURSOR_L, VGA_CRTC_ADDR);
    outb(offset & 0xFF, VGA_CRTC_DATA);

    irq_restore(eflags);
}

void _vga_set_start_addr(vc_t* vc) {
    uint16_t* vram_vaddr = vc->vram_vaddr;

    off_t offset = vram_vaddr - vga_vram_base_vaddr;

    unsigned long eflags;
    irq_save(eflags);

    outb(VGA_CRTC_START_ADDR_H, VGA_CRTC_ADDR);
    outb((offset >> 8) & 0xFF, VGA_CRTC_DATA);
    outb(VGA_CRTC_START_ADDR_L, VGA_CRTC_ADDR);
    outb(offset & 0xFF, VGA_CRTC_DATA);

    irq_restore(eflags);
}

void vga_switch(vc_t* vc) {
    _vga_set_start_addr(vc);

    if (vc->show_cursor) {
        _vga_cursor(true);
        vga_set_cursor(vc);
    } else {
        _vga_cursor(false);
    }
}

void vga_set_xy(vc_t* vc, off_t offset) {
    // 最后一个字符写完是可能自动移动下一个的，所以是可能超过vga_vram_size的
    // 所以这里用的是 <= 而不是 <
    assert(offset <= vga_vram_size);
    vc->x = offset % vc->cols;
    vc->y = offset / vc->cols;

    if (offset >= vga_vram_size) {
        vga_scroll_up(vc);
    }

    vga_set_cursor(vc);
}

void vga_color_putc(vc_t* vc, uint8_t c, uint8_t color) {
    assert(vc != NULL);
    // assert(0);

    bool display = false;
    off_t offset = vga_get_offset(vc);

    switch (c) {
    case '\r':
        offset = vc->y * vc->cols;
        break;
    case '\n':
        offset = (vc->y + 1) * vc->cols;
        break;
    case '\t':
        offset += TAB_SPACE;
        offset &= ~(TAB_SPACE - 1);
        break;
    case '\b':
        if (vc->x == 0 && vc->y == 0) {
            break;
        } else {
            assert(vc->x < vc->cols);
            assert(vc->y < vc->rows);
            assert(offset < vga_vram_size);
            offset -= 1;
        }
        break;
    default:
        display = true;
        break;
    }

    vga_set_xy(vc, offset);

    offset = vga_get_offset(vc);

    if (display) {
        assert(offset <= vga_vram_size);
        uint16_t* vram_vaddr = vc->vram_vaddr;
        vram_vaddr[offset] = (color << 8) | c;

        offset += 1;
    }

    vga_set_xy(vc, offset);
}

void vga_putc(vc_t* vc, uint8_t c) {
    vga_color_putc(vc, c, vc->default_color);
}

int vga_write(vc_t* vc, const char* buf, size_t size) {
    assert(buf != NULL);
    for (size_t i = 0; i < size; i++) {
        vga_putc(vc, buf[i]);
    }

    return size;
}

void vga_ap_clear() {
    vc_t* vc = vt_get_ap_vc();
    assert(vc != NULL);
    vga_clear(vc);
}

void vga_ap_write(int x, int y, char* buf, size_t size) {
    assert(buf != NULL);
    vc_t* vc = vt_get_ap_vc();
    assert(vc != NULL);
    assert(x >= 0);
    assert(y >= 0);
    assert(x < vc->cols);
    assert(y < vc->rows);

    vc->x = x;
    vc->y = y;

    for (size_t i = 0; i < size; i++) {
        vga_color_putc(vc, buf[i], vc->default_color);
    }
}
