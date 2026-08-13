/*
 * ------------------------------------------------------------------------
 *   File Name: completion.h
 *      Author: Zhao Yanbai
 *              2021-11-27 10:58:33 Saturday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <wait.h>

typedef struct deprecated_completion {
    unsigned int done;
    wait_queue_head_t wait;

    // 仅用于调试
    char* name;
} deprecated_completion_t;

#define DEPRECATED_COMPLETION_INITIALIZER(x) {0, WAIT_QUEUE_HEAD_INITIALIZER(x.wait)}

void init_deprecated_completion(deprecated_completion_t* x);

void wait_deprecated_completion(deprecated_completion_t* x);

// 一次只唤醒一个进程
void complete(deprecated_completion_t* x);
