/*
 *--------------------------------------------------------------------------
 *   File Name: sched.h
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Sat Feb  7 21:43:49 2009
 * Last Update: Sat Feb  7 21:43:49 2009
 *
 *--------------------------------------------------------------------------
 */

#pragma once

#include <irq.h>
#include <system.h>
#include <task.h>

#define FORK_USER 0
#define FORK_KRNL 1

#define TASK_PRIORITY_LEVEL_KERNEL 0
#define TASK_PRIORITY_LEVEL_SYSTEM 10
#define TASK_PRIORITY_LEVEL_DRIVER 20
#define TASK_PRIORITY_LEVEL_USER 80

#define TASK_PRIORITY_CNT 100
#define TASK_PRIORITY_MIN 0
#define TASK_PRIORITY_MAX (TASK_PRIORITY_CNT - 1)
#define READYQ_BITS_PER_WORD 32
#define READYQ_BITMAP_WORD_CNT ((TASK_PRIORITY_CNT + READYQ_BITS_PER_WORD - 1) / READYQ_BITS_PER_WORD)
typedef struct priority_readyq {
    list_head_t lists[TASK_PRIORITY_CNT];
    uint32_t bitmap[READYQ_BITMAP_WORD_CNT];
} priority_readyq_t;

void task_reset_priority(int priority);

void schedule();

void set_need_schedule();
void clear_need_schedule();
bool need_schedule();

extern task_t root_task;

extern void load_cr3(task_t* tsk);

extern list_head_t all_tasks;
