/*
 * ------------------------------------------------------------------------
 *   File Name: semaphore.h
 *      Author: Zhao Yanbai
 *              Sun Jun 22 13:53:18 2014
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <list.h>

typedef struct deprecated_semaphore {
    volatile unsigned int cnt;
    list_head_t wait_list;
} deprecated_semaphore_t;

#define SEMAPHORE_INITIALIZER(name, n) {.cnt = (n), .wait_list = LIST_HEAD_INIT((name).wait_list)}

void deprecated_semaphore_init(deprecated_semaphore_t* s, unsigned int v);

// down
// 如果s->cnt > 0不会立即重新调度进程
// 如果s->cnt == 0 会重新调度进程
volatile void down(deprecated_semaphore_t* s);

// up
// 只会唤醒进程，但不会立即重新调度进程
volatile void up(deprecated_semaphore_t* s);

typedef deprecated_semaphore_t deprecated_mutex_t;

#define MUTEX_INITIALIZER(name) {.cnt = (1), .wait_list = LIST_HEAD_INIT((name).wait_list)}

#define DECLARE_MUTEX(name) mutex_t name = MUTEX_INITIALIZER(name)

#define INIT_MUTEX(ptr)                      \
    do {                                     \
        (ptr)->cnt = 1;                      \
        INIT_LIST_HEAD(&((ptr)->wait_list)); \
    } while (0)
void deprecated_mutex_init(deprecated_mutex_t*);
void deprecated_mutex_lock(deprecated_mutex_t*);
void deprecated_mutex_unlock(deprecated_mutex_t*);
