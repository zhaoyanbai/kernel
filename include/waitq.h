/*
 * ------------------------------------------------------------------------
 *   File Name: waitq.h
 *      Author: Zhao Yanbai
 *              2026-08-13 06:39:38 Thursday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <list.h>

typedef struct waitq waitq_t;

struct waitq {
    list_head_t list;
};

void waitq_init(waitq_t* waitq);

// 以下接口
// 在调用前需要保证已经关闭中断
// 在返回时中断状态仍然处于关闭状态
void waitq_sleep(waitq_t* waitq);

void waitq_wakeup(waitq_t* waitq, int cnt);

void waitq_wakeup_one(waitq_t* waitq);

void waitq_wakeup_all(waitq_t* waitq);