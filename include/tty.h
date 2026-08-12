/*
 * ------------------------------------------------------------------------
 *   File Name: tty.h
 *      Author: Zhao Yanbai
 *              2021-11-07 17:17:23 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

#define TTY_MAX_NAME_LEN 32
#define TTY_MAX_COUNT 8
#define TTY_MAX_IN_BUF_SIZE 512

typedef struct tty tty_t;
typedef struct tty_ops tty_ops_t;

struct tty_ops {
    int (*write)(tty_t* tty, const char* buf, size_t size);
};

struct tty {
    char name[TTY_MAX_NAME_LEN];

    uint8_t in_buf[TTY_MAX_IN_BUF_SIZE];
    int ib_head;
    int ib_tail;

    tty_ops_t* ops;
    void* private;
};

void init_ttys();

// 进程 -> read -> tty_read -> TTY (buf)
// 键盘： 键盘中断 -> vt_keyboard_input -> tty_input -> TTY (in_buf)
//                                          └─echo(回显) -> tty_write -> TTY ops->write -> 键盘 TX
// 串口 RX： 串口中断 -> serial_input -> tty_input -> TTY (in_buf)
//                                         └─echo(回显) -> tty_write -> TTY ops->write -> 串口 TX
// 进程 -> write -> tty_write -> TTY ops->write -> [Serial, VT] write -> 硬件

int tty_write(tty_t* tty, const char* buf, size_t size);
int tty_read(tty_t* tty, char* buf, size_t size);

// 返回 1 表示成功，0 表示缓冲区满，丢弃
int tty_input(tty_t* tty, uint8_t c);
