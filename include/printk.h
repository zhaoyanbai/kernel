/*
 *--------------------------------------------------------------------------
 *   File Name: printk.h
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Wed Mar  4 21:49:19 2009
 * Last Update: Wed Mar  4 21:49:19 2009
 *
 *--------------------------------------------------------------------------
 */

#pragma once

int printk(const char* fmtstr, ...);
int printd(const char* fmtstr, ...);
int ap_print(unsigned int xpos, unsigned int ypos, const char* fmtstr, ...);

#define ap_printl(line, fmt, args...) ap_print(1, line, fmt, ##args)
#define ap_printll(line, fmt, args...) ap_print(0, line, fmt, ##args)
#define ap_printlr(line, fmt, args...) ap_print(40, line, fmt, ##args)
#define ap_printlxy(line, offset, fmt, args...) ap_print(offset, line, fmt, ##args)


// monitor print line
enum {
    MPL_TITLE,
    MPL_IRQ,
    MPL_IDE0,
    MPL_IDE1,
    MPL_CURRENT,
    MPL_TEST,
    MPL_DEBUG,
    MPL_TASK_TITLE,
    MPL_TASK_0,
    MPL_TASK_1,
    MPL_TASK_2,
    MPL_TASK_3,
    MPL_TASK_4,
    MPL_TASK_5,
    MPL_TASK_6,
    MPL_TASK_7,
    MPL_TASK_8,
    MPL_END
};

// monitor print offset
enum {
    MPO_HPET = 1,
    MPO_AP_CLOCK = 20,
    MPO_DISK = 36,
    MPO_KEYBOARD = 60,
    MPO_IDE = 1,
};

int sprintf(char* str, const char* fmtstr, ...);

#include <types.h>
typedef int (*printk_t)(const char* fmtstr, va_list args);
void set_printk(printk_t pk);
int _printk(const char* fmtstr, va_list args);
