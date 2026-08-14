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

    // 是否唤醒了优先级更高的任务，如果唤醒就将当前任务标记为需要调度，以便新的高优先级任务更快可以被调度
    bool woken_higher_priority_task = false;

    bool woken = false;

    for (int i = 0; ((i < cnt) || (cnt == 0)); i++) {
        if (list_empty(&waitq->list)) {
            break;
        }

        task_t* task = list_first_entry(&waitq->list, task_t, waitq_list);
        assert(task != NULL);
        assert(task->state == TASK_WAIT);

        list_del_init(&task->waitq_list);

        task_set_ready(task);

        woken = true;

        // 当前任务优先级低于唤醒的任务
        if (current->priority > task->priority) {
            woken_higher_priority_task = true;
        }
    }

    if (woken_higher_priority_task || (woken && (current == &root_task))) {
        set_need_schedule();
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