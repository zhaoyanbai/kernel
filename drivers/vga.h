/*
 * ------------------------------------------------------------------------
 *   File Name: vga.h
 *      Author: Zhao Yanbai
 *              2026-08-11 19:37:11 Tuesday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

static const paddr_t vga_vram_base_paddr = 0xB8000;
static const size_t vga_cols = 80;
static const size_t vga_rows = 25;
static const size_t vga_vram_size = vga_cols * vga_rows;
static const size_t vga_vram_byte_size = vga_vram_size * sizeof(uint16_t);
static const size_t TAB_SPACE = 4;

#define VGA_FG_HIGHLIGHT 0b1000
#define VGA_BG_BLINK 0b1000

#define VGA_BLACK 0b0000
#define VGA_BLUE 0b0001
#define VGA_GREEN 0b0010
#define VGA_CYAN 0b0011
#define VGA_RED 0b0100
#define VGA_PURPLE 0b0101
#define VGA_YELLOW 0b0110
#define VGA_WHITE 0b0111

void vga_init_vc(vc_t* vc);
void vga_switch(vc_t* vc);
int vga_write(vc_t* vc, const char* buf, size_t size);
uint8_t vga_make_attr(uint8_t fg, uint8_t bg, bool fg_highlight, bool bg_blink);
void vga_set_cursor_style(bool block);