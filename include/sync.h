/*
 * ------------------------------------------------------------------------
 *   File Name: sync.h
 *      Author: Zhao Yanbai
 *              2026-08-13 06:39:47 Thursday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <waitq.h>

// WAIT EVENT

#define wait_event(waitq, condition) \
    do {                             \
        assert(waitq != NULL);       \
        unsigned long __we_eflags;   \
        irq_save(__we_eflags);       \
        while (!(condition)) {       \
            waitq_sleep(waitq);      \
        }                            \
        irq_restore(__we_eflags);    \
    } while (0)

// SEMAPHORE
typedef struct semaphore semaphore_t;

struct semaphore {
    volatile int cnt;
    waitq_t waitq;
};

void semaphore_init(semaphore_t* semaphore, int cnt);

void semaphore_down(semaphore_t* semaphore);

void semaphore_up(semaphore_t* semaphore);

// MUTEX
typedef semaphore_t mutex_t;

void mutex_init(mutex_t* mutex);

void mutex_lock(mutex_t* mutex);

void mutex_unlock(mutex_t* mutex);

// COMPLETION
typedef struct completion completion_t;

struct completion {
    volatile int done;
    waitq_t waitq;

    // 仅用于调试
    char* name;
};

void completion_init(completion_t* completion);

void completion_wait(completion_t* completion);

void completion_complete(completion_t* completion);
