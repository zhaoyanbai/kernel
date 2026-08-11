/*
 * ------------------------------------------------------------------------
 *   File Name: console.h
 *      Author: Zhao Yanbai
 *              Sun Jun 22 22:06:53 2014
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <wait.h>

typedef struct console console_t;

struct console {
    const char* name;
    int (*setup)(console_t* console);
    int (*write)(const char* buf, size_t size);
};

void register_console(console_t* console);
int console_write(const char* buf, size_t size);

#define CNSL_QUEUE_SIZE 1024

typedef struct cnsl_queue {
    unsigned int head;
    unsigned int tail;
    wait_queue_head_t wait;
    char data[CNSL_QUEUE_SIZE];
} cnsl_queue_t;

typedef struct cnsl {
    cnsl_queue_t rd_q;
    cnsl_queue_t wr_q;
    cnsl_queue_t sc_q;
} cnsl_t;

int cnsl_kbd_write(char c);
