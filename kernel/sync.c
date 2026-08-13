/*
 * ------------------------------------------------------------------------
 *   File Name: sync.c
 *      Author: Zhao Yanbai
 *              2026-08-13 06:39:52 Thursday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <sync.h>
#include <irq.h>
#include <system.h>

// SEMAPHORE
void semaphore_init(semaphore_t* semaphore, int cnt) {
    assert(semaphore != NULL);
    semaphore->cnt = cnt;
    waitq_init(&semaphore->waitq);
}

void semaphore_down(semaphore_t* semaphore) {
    assert(semaphore != NULL);

    unsigned long eflags;
    irq_save(eflags);

    // 此处semaphore->cnt其实是有可能为负数的，因为可能直接被初始化为负数
    while (true) {
        if (semaphore->cnt > 0) {
            semaphore->cnt--;
            break;
        }

        waitq_sleep(&semaphore->waitq);
    }

    irq_restore(eflags);
}

void semaphore_up(semaphore_t* semaphore) {
    assert(semaphore != NULL);
    unsigned long eflags;
    irq_save(eflags);

    semaphore->cnt++;

    waitq_wakeup_one(&semaphore->waitq);

    irq_restore(eflags);
}

// MUTEX

void mutex_init(mutex_t* mutex) {
    assert(mutex != NULL);
    semaphore_init(mutex, 1);
}

void mutex_lock(mutex_t* mutex) {
    assert(mutex != NULL);
    semaphore_down(mutex);
}

void mutex_unlock(mutex_t* mutex) {
    assert(mutex != NULL);
    semaphore_up(mutex);
}

// COMPLETION

void completion_init(completion_t* completion) {
    assert(completion != NULL);
    completion->done = 0;
    waitq_init(&completion->waitq);
    completion->name = NULL;
}

void completion_wait(completion_t* completion) {
    assert(completion != NULL);

    unsigned long eflags;
    irq_save(eflags);

    while (completion->done == 0) {
        waitq_sleep(&completion->waitq);
    }

    assert(completion->done > 0);
    completion->done--;

    irq_restore(eflags);
}

void completion_complete(completion_t* completion) {
    assert(completion != NULL);

    unsigned long eflags;
    irq_save(eflags);

    completion->done++;
    waitq_wakeup_all(&completion->waitq);

    irq_restore(eflags);
}