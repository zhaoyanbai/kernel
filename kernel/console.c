/*
 * ------------------------------------------------------------------------
 *   File Name: console.c
 *      Author: Zhao Yanbai
 *              Sun Jun 22 18:50:13 2014
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <console.h>
#include <sched.h>
#include <string.h>
#include <tty.h>

#define MAX_CONSOLE_CNT 16

static console_t* console_list[MAX_CONSOLE_CNT] = {NULL};

void register_console(console_t* console) {
    assert(console != NULL);
    assert(console->name != NULL);
    assert(console->write != NULL);
    // assert(console->setup != NULL);

    for (int i = 0; i < MAX_CONSOLE_CNT; i++) {
        if (console_list[i] == console) {
            return;
        }
    }

    for (int i = 0; i < MAX_CONSOLE_CNT; i++) {
        if (console_list[i] == NULL) {
            console_list[i] = console;
            if (console->setup != NULL) {
                console->setup(console);
            }
            printk("console %s registered\n", console->name);
            return;
        }
    }
    panic("console list full\n");
    return;
}

int console_write(const char* buf, size_t size) {
    assert(buf != NULL);
    for (int i = 0; i < MAX_CONSOLE_CNT; i++) {
        if (console_list[i] == NULL) {
            break;
        }

        console_list[i]->write(buf, size);
    }
    return 0;
}