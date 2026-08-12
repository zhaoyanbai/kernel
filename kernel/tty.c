/*
 * ------------------------------------------------------------------------
 *   File Name: tty.c
 *      Author: Zhao Yanbai
 *              2021-11-07 17:17:18 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <assert.h>
#include <io.h>
#include <irq.h>
#include <page.h>
#include <string.h>
#include <tty.h>
#include <vt.h>
#include <errno.h>

#define MAX_NR_TTYS 8
tty_t ttys[NR_TTYS];

void init_ttys() {
    for (int i = 0; i < VC_FOR_TTY_COUNT; i++) {
        tty_t* tty = ttys + i;
        char name[TTY_MAX_NAME_LEN];
        sprintf(name, "tty%d", i);

        strcpy(tty->name, name);

        tty->ib_head = 0;
        tty->ib_tail = 0;

        tty->ops = &vt_tty_ops;

        vc_t* vc = vt_get_vc(i);
        assert(vc != NULL);

        tty->private = vc;
        vc->tty = tty;

        printk("init_ttys: tty %x vcid %d\n", tty, i);
    }
}

void tty_putc(tty_t* tty, char c) {
}

int tty_write(tty_t* tty, const char* buf, size_t size) {
    assert(tty != NULL);
    assert(buf != NULL);

    tty_ops_t* ops = tty->ops;
    assert(ops != NULL);

    return ops->write(tty, buf, size);

    return 0;
}

int tty_read(tty_t* tty, char* buf, size_t size) {
    assert(tty != NULL);

    if (0 == size) {
        return 0;
    }

    if (NULL == buf) {
        return -EINVAL;
    }

    // TODO

    return 0;
}

int tty_input(tty_t* tty, uint8_t c) {
    assert(tty != NULL);

    int next = (tty->ib_head + 1) % TTY_MAX_IN_BUF_SIZE;
    if (next == tty->ib_tail) {
        return 0;
    }

    tty->in_buf[tty->ib_head] = c;
    tty->ib_head = next;

    // 目前先总是回显
    tty_write(tty, (const char*)&c, 1);

    // TODO: 唤醒读进程

    return 1;
}