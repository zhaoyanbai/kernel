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
        mutex_init(&tty->ib_mutex);
        init_wait_queue_head(&tty->ib_wait);

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

bool _tty_ib_empty(tty_t* tty) {
    return tty->ib_head == tty->ib_tail;
}

int tty_read(tty_t* tty, char* buf, size_t size) {
    assert(tty != NULL);

    if (0 == size) {
        return 0;
    }

    if (NULL == buf) {
        return -EINVAL;
    }

    size_t n = 0;

    // 防多进程同时读取输入缓冲区
    mutex_lock(&tty->ib_mutex);
    while (true) {
        unsigned long eflags;
        irq_save(eflags);

        // 如果输入缓冲区为空，则继续等待
        if (_tty_ib_empty(tty)) {
            irq_restore(eflags);
            mutex_unlock(&tty->ib_mutex);
            wait_event(&tty->ib_wait, !_tty_ib_empty(tty));
            mutex_lock(&tty->ib_mutex);
            continue;
        }

        irq_restore(eflags);

        // 如果输入缓冲区不为空，则读取输入缓冲区
        for (n = 0; n < size; n++) {
            irq_save(eflags);

            if (_tty_ib_empty(tty)) {
                break;
            }

            buf[n] = tty->in_buf[tty->ib_tail];
            tty->ib_tail += 1;
            tty->ib_tail %= TTY_MAX_IN_BUF_SIZE;

            irq_restore(eflags);
        }

        break;
    }

    mutex_unlock(&tty->ib_mutex);

    return n;
}

int tty_input(tty_t* tty, uint8_t c) {
    assert(tty != NULL);

    unsigned long eflags;
    irq_save(eflags);

    int next = (tty->ib_head + 1) % TTY_MAX_IN_BUF_SIZE;
    if (next == tty->ib_tail) {
        irq_restore(eflags);
        return 0;
    }

    tty->in_buf[tty->ib_head] = c;
    tty->ib_head = next;

    irq_restore(eflags);

    // 唤醒读进程
    wake_up(&tty->ib_wait);

    // 目前先总是回显
    tty_write(tty, (const char*)&c, 1);

    return 1;
}