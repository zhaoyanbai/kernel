/*
 *--------------------------------------------------------------------------
 *   File Name: keyboard.h
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Thu Jul 16 18:39:57 2009
 * Last Update: Thu Jul 16 18:39:57 2009
 *
 *--------------------------------------------------------------------------
 */
#include <console.h>
#include <io.h>
#include <printk.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <system.h>
#include <tty.h>
#include <vt.h>
#include <irq.h>

void reboot();
void poweroff();
void ide_debug();
void ide_status();

void kbd_debug(uint8_t scan_code);

char kbd_char_tbl[] = {
    0,   0,   '1', '2', '3', '4', '5',  '6', '7', '8', '9', '0', '-', '=', '\b', 0,   'q', 'w', 'e',  'r', 't', 'y',
    'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a', 's', 'd', 'f', 'g', 'h', 'j',  'k', 'l', ';', '\'', '`', 0,   '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm',  ',', '.', '/', 0,   0,   0,   ' ', 0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,
};

uint8_t kbd_scan_code;
void kbd_bh_handler(void* arg) {
    kbd_debug(kbd_scan_code);
    if (0x80 & kbd_scan_code) {  // break code
        return;
    }

    uint8_t inx = kbd_scan_code & 0xFF;
    char ch = kbd_char_tbl[inx];

    vt_keyboard_input(ch);
}

uint64_t kbd_irq_cnt = 0;
void kbd_handler(unsigned int irq, pt_regs_t* regs, void* dev_id) {
    kbd_scan_code = inb(0x60);
    kbd_irq_cnt++;
    add_irq_bh_handler(kbd_bh_handler, NULL);
}

void kbd_debug(uint8_t scan_code) {
    switch (scan_code) {
    case 0x01:  // Esc
        break;
    case 0x3B:  // F1
        vt_switch(0);
        break;
    case 0x3C:  // F2
        vt_switch(1);
        break;
    case 0x3D:  // F3
        vt_switch(2);
        break;
    default:
        break;
    }
}
