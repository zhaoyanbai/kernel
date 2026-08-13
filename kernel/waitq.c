/*
 * ------------------------------------------------------------------------
 *   File Name: waitq.c
 *      Author: Zhao Yanbai
 *              2026-08-13 06:39:43 Thursday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <waitq.h>
#include <sched.h>

void waitq_init(waitq_t* waitq) {
    assert(waitq != NULL);
    list_init(&waitq->list);
}

void waitq_sleep(waitq_t* waitq) {
    task_t* task = current;

    assert(waitq != NULL);
    assert(task != NULL);
    assert(task->state != TASK_WAIT);

    list_add_tail(&task->waitq_list, &waitq->list);
    task_set_wait(task);

    schedule();
}

void waitq_wakeup(waitq_t* waitq, int cnt) {
    assert(waitq != NULL);
    assert(cnt >= 0);

    for (int i = 0; ((i < cnt) || (cnt == 0)); i++) {
        if (list_empty(&waitq->list)) {
            break;
        }

        task_t* task = list_first_entry(&waitq->list, task_t, waitq_list);
        assert(task != NULL);
        assert(task->state == TASK_WAIT);

        list_del_init(&task->waitq_list);

        task_set_ready(task);
    }
}

void waitq_wakeup_one(waitq_t* waitq) {
    assert(waitq != NULL);
    waitq_wakeup(waitq, 1);
}

void waitq_wakeup_all(waitq_t* waitq) {
    assert(waitq != NULL);
    waitq_wakeup(waitq, 0);
}