/*
 * ------------------------------------------------------------------------
 *   File Name: timer.c
 *      Author: Zhao Yanbai
 *              2026-08-13 16:11:37 Thursday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <timer.h>
#include <irq.h>
#include <assert.h>
#include <clock.h>

void timer_init(timer_t* timer, uint64_t expires, timer_callback_t callback, void* callback_arg) {
    assert(timer != NULL);

    list_init(&timer->list);
    timer->expires = expires;
    timer->callback = callback;
    timer->callback_arg = callback_arg;
}

list_head_t g_timer_list = LIST_HEAD_INIT(g_timer_list);

void timer_add(timer_t* timer) {
    assert(timer != NULL);
    assert(list_empty(&timer->list));

    unsigned long eflags;
    irq_save(eflags);

    // 按expires升序添加到g_timer_list中
    list_head_t* prev = &g_timer_list;
    list_head_t* pos = NULL;
    list_for_each(pos, &g_timer_list) {
        timer_t* current_timer = list_entry(pos, timer_t, list);
        if (timer->expires < current_timer->expires) {
            break;
        }

        prev = pos;
    }

    assert(prev != NULL);
    list_add(&timer->list, prev);

    irq_restore(eflags);
}

void timer_del(timer_t* timer) {
    assert(timer != NULL);

    unsigned long eflags;
    irq_save(eflags);

    list_del_init(&timer->list);

    irq_restore(eflags);
}

void timer_run_expired_timers() {
    list_head_t expire_timers = LIST_HEAD_INIT(expire_timers);

    // 关中断将到期的定时器添加到临时的expire_timers链表中
    unsigned long eflags;
    irq_save(eflags);

    list_head_t* pos = NULL;
    list_head_t* tmp = NULL;
    list_for_each_safe(pos, tmp, &g_timer_list) {
        timer_t* timer = list_entry(pos, timer_t, list);
        if (timer->expires <= jiffies) {
            list_del_init(pos);

            list_add_tail(pos, &expire_timers);
        } else {
            break;
        }
    }

    irq_restore(eflags);

    // 执行到期的定时器回调函数
    pos = NULL;
    tmp = NULL;
    list_for_each_safe(pos, tmp, &expire_timers) {
        timer_t* timer = list_entry(pos, timer_t, list);
        timer_callback_t callback = timer->callback;
        void* callback_arg = timer->callback_arg;
        assert(callback != NULL);

        list_del_init(pos);

        callback(callback_arg);
    }
}