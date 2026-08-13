/*
 * ------------------------------------------------------------------------
 *   File Name: timer.h
 *      Author: Zhao Yanbai
 *              2026-08-13 16:02:25 Thursday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <list.h>
#include <types.h>

typedef struct timer timer_t;

typedef void (*timer_callback_t)(void* arg);

struct timer {
    list_head_t list;
    uint64_t expires;

    timer_callback_t callback;
    void* callback_arg;
};

void timer_init(timer_t* timer, uint64_t expires, timer_callback_t callback, void* callback_arg);

void timer_add(timer_t* timer);

void timer_del(timer_t* timer);

void timer_run_expired_timers();