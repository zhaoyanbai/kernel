/*
 * ------------------------------------------------------------------------
 *   File Name: console.h
 *      Author: Zhao Yanbai
 *              Sun Jun 22 22:06:53 2014
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

typedef struct console console_t;

struct console {
    const char* name;
    int (*setup)(console_t* console);
    int (*write)(const char* buf, size_t size);
};

void register_console(console_t* console);
int console_write(const char* buf, size_t size);
