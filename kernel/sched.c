/*
 *--------------------------------------------------------------------------
 *   File Name: sched.c
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Tue Feb 10 11:53:21 2009
 * Last Update: Tue Feb 10 11:53:21 2009
 *
 *--------------------------------------------------------------------------
 */

#include "sched.h"

#include "assert.h"
#include "linkage.h"
#include "mm.h"
#include "msr.h"

task_t root_task __attribute__((__aligned__(PAGE_SIZE)));

// 暂时不考虑pid回绕问题
pid_t get_next_pid() {
    static pid_t g_pid = ROOT_TSK_PID;

    unsigned long iflags;
    irq_save(iflags);

    pid_t pid = g_pid;
    g_pid++;

    irq_restore(iflags);

    return pid;
}

void load_cr3(task_t* tsk) {
    write_cr3(tsk->cr3);
}

extern pde_t __initdata init_pgd[PDECNT_PER_PAGE] __attribute__((__aligned__(PAGE_SIZE)));

LIST_HEAD(all_tasks);
// LIST_HEAD(ready_tasks);

void init_root_task() {
    int i;

    root_task.pid = get_next_pid();
    root_task.ppid = 0;
    root_task.state = TASK_RUN;
    root_task.reason = "root";
    root_task.priority = TASK_PRIORITY_MAX;
    root_task.ticks_left = 1;
    root_task.vma_list = NULL;
    root_task.magic = TASK_MAGIC;
    strcpy(root_task.name, "root");

    task_init_lists(&root_task);
    task_init_stats(&root_task);
    // root_task.st_last_exec_tsc = rdtsc();

    list_add(&root_task.list, &all_tasks);

    //  TODO
    // for(i=0; i<NR_OPENS; i++)
    //    root_task.fps[i] = 0;

    root_task.esp0 = ((unsigned long)&root_task) + sizeof(root_task);
    root_task.cr3 = va2pa((unsigned long)(init_pgd));

    tss.esp0 = root_task.esp0;  // sysenter会把MSR_SYSENTER_ESP设置为&tss.esp0

    for (i = 0; i < NR_TASK_OPEN_CNT; i++) {
        root_task.files.fds[i] = NULL;
    }

    printk("init_root_task tss.esp0 %08x\n", tss.esp0);
}

kmem_cache_t* task_t_cache;

static priority_readyq_t g_priority_readyq;
void priority_readyq_init(priority_readyq_t* readyq) {
    for (int i = 0; i < TASK_PRIORITY_CNT; i++) {
        list_init(&readyq->lists[i]);
    }

    for (int i = 0; i < READYQ_BITMAP_WORD_CNT; i++) {
        readyq->bitmap[i] = 0;
    }
}

static void priority_readyq_set_bit(int priority) {
    int item_index = priority / READYQ_BITS_PER_WORD;
    int bit_index = priority % READYQ_BITS_PER_WORD;
    g_priority_readyq.bitmap[item_index] |= (1U << bit_index);
}

static void priority_readyq_clear_bit(int priority) {
    int item_index = priority / READYQ_BITS_PER_WORD;
    int bit_index = priority % READYQ_BITS_PER_WORD;
    g_priority_readyq.bitmap[item_index] &= ~(1U << bit_index);
}

void task_reset_priority(int priority) {
    assert(priority >= TASK_PRIORITY_MIN);
    assert(priority <= TASK_PRIORITY_MAX);

    task_t* task = current;

    if (task->priority == priority) {
        return;
    }

    // unsigned long eflags;
    // irq_save(eflags);

    // 只有当前运行的Task才能调整priority
    // 而它在调度器调度运行时已经从队列上取下了
    // 运行时不在任何队列上，所以只需要直接调整

    int old_priority = current->priority;

    current->priority = priority;

    // irq_restore(eflags);

    // 降低优先级应该触发调度
    if ((old_priority < priority) && priority_readyq_has_higher_priority_task(priority)) {
        set_need_schedule();
    }
}

static void _priority_readyq_enqueue(task_t* task, bool at_head) {
    assert(task != NULL);

    int priority = task->priority;

    assert(priority >= TASK_PRIORITY_MIN);
    assert(priority <= TASK_PRIORITY_MAX);

    if (at_head) {
        list_add(&task->ready_list, g_priority_readyq.lists + priority);
    } else {
        list_add_tail(&task->ready_list, g_priority_readyq.lists + priority);
    }

    priority_readyq_set_bit(priority);
}

void priority_readyq_enqueue_head(task_t* task) {
    _priority_readyq_enqueue(task, true);
}

void priority_readyq_enqueue_tail(task_t* task) {
    _priority_readyq_enqueue(task, false);
}

void priority_readyq_unlink(task_t* task) {
    assert(task != NULL);

    int priority = task->priority;

    assert(priority >= TASK_PRIORITY_MIN);
    assert(priority <= TASK_PRIORITY_MAX);

    assert(!list_empty(&task->ready_list));

    list_del_init(&task->ready_list);

    if (list_empty(g_priority_readyq.lists + priority)) {
        priority_readyq_clear_bit(priority);
    }
}

bool priority_readyq_has_higher_priority_task(int priority) {
    assert(priority >= TASK_PRIORITY_MIN);
    assert(priority <= TASK_PRIORITY_MAX);

    int end_index = priority / READYQ_BITS_PER_WORD;
    int bit_index = priority % READYQ_BITS_PER_WORD;

    for (int i = 0; i < end_index; i++) {
        if (g_priority_readyq.bitmap[i] != 0) {
            return true;
        }
    }

    // 虽然bit_index为0的情况下这个if必不满足，但不影响逻辑正确性
    if (g_priority_readyq.bitmap[end_index] & ((1U << bit_index) - 1)) {
        return true;
    }

    return false;
}

void setup_tasks() {
    INIT_LIST_HEAD(&all_tasks);
    // INIT_LIST_HEAD(&ready_tasks);

    priority_readyq_init(&g_priority_readyq);

    init_root_task();

    task_t_cache = kmem_cache_create("task_t", sizeof(task_t), PAGE_SIZE);
    if (0 == task_t_cache) {
        panic("setup tasks failed. out of memory");
    }
}

task_t* alloc_task() {
    task_t* task;
    task = (task_t*)kmem_cache_alloc(task_t_cache, 0);
    return task;
}

void switch_to() {
    write_cr3(current->cr3);
    tss.esp0 = current->esp0;
}

void context_switch(task_t* prev, task_t* next) {
    unsigned long /*eax,*/ ebx, ecx, edx, esi, edi;
    asm volatile(
        "pushfl;"
        "pushl  %%ebp;"
        "movl   %%esp,%[prev_esp];"
        "movl   %[next_esp],%%esp;"
        "movl   $1f,%[prev_eip];"
        "pushl  %[next_eip];"
        "jmp    switch_to;"
        "1:"
        "popl   %%ebp;"
        "popfl;"
        : [prev_esp] "=m"(prev->esp), [prev_eip] "=m"(prev->eip), "=a"(prev), "=b"(ebx), "=c"(ecx), "=d"(edx),
          "=S"(esi), "=D"(edi)
        : [next_esp] "m"(next->esp), [next_eip] "m"(next->eip), [prev] "a"(prev), [next] "d"(next)
        : "memory");
}

task_t* pick_next_task() {
    int index = -1;  // 代表所有READY队列都为空
    for (int i = 0; i < READYQ_BITMAP_WORD_CNT; i++) {
        if (g_priority_readyq.bitmap[i] != 0) {
            index = __builtin_ctz(g_priority_readyq.bitmap[i]);

            index += i * READYQ_BITS_PER_WORD;
            break;
        }
    }

    if (index == -1) {
        return NULL;
    }

    assert(index >= 0);
    assert(index < TASK_PRIORITY_CNT);

    // 对应的优先级队列必定不为空
    list_head_t* list = g_priority_readyq.lists + index;
    assert(!list_empty(list));

    // 返回队列头部第一个任务
    task_t* task = list_entry(list->next, task_t, ready_list);
    assert(task != NULL);
    assert(task->priority == index);
    assert(task != &root_task);
    assert(task->priority >= TASK_PRIORITY_MIN);
    assert(task->priority <= TASK_PRIORITY_MAX);
    assert(task->state == TASK_READY);

    return task;
}

void schedule() {
    task_t* prev = current;
    task_t* next = NULL;

    unsigned long eflags;
    irq_save(eflags);

    // prev->state == TASK_WAIT 的进程也可能走到这里来，因为这还是切换前
    if (prev->state == TASK_READY || prev->state == TASK_RUN) {
        prev->state = TASK_READY;
        assert(list_empty(&prev->ready_list));
        if (prev != &root_task) {
            // 如果时间片没有耗尽(例如：被高优先级的进程抢占了，或者主动让出CPU)，还是把自己挂队列头部，下次还是该优先级第一个被调度，以消耗完全余下的时间片
            // 如果时间片耗尽了，就把自己挂到就绪队列尾部，下次调度时会重新分配时间片
            if (prev->ticks_left > 0) {
                priority_readyq_enqueue_head(prev);
            } else {
                priority_readyq_enqueue_tail(prev);
            }
        }
    }

    next = pick_next_task();

    if (next == NULL) {
        next = &root_task;
        next->ticks_left = 1;
    } else {
        priority_readyq_unlink(next);
        next->state = TASK_RUN;
        if (next->ticks_left <= 0) {
            next->ticks_left = TASK_TICKS_PER_QUANTUM;
        }
    }

    clear_need_schedule();

    if (prev != next) {
        uint64_t tsc = rdtsc();

        prev->st_runtime_tsc += tsc - prev->st_last_exec_tsc;
        prev->st_last_exec_tsc = 0;

        assert(next->st_last_exec_tsc == 0);
        next->st_last_exec_tsc = tsc;

        next->sched_cnt++;

        context_switch(prev, next);
    } else {
        next->sched_keep_cnt++;
    }

    irq_restore(eflags);
}

task_t* monitor_tasks[1024] = {&root_task, 0};
void add_task_for_monitor(task_t* tsk) {
    assert(tsk != NULL);
    int id = tsk->pid;
    monitor_tasks[id] = tsk;
}

void task_set_ready(task_t* t) {
    assert(t != NULL);
    if (t == &root_task) {
        t->state = TASK_READY;
        return;
    }

    unsigned long eflags;
    irq_save(eflags);

    // 不应该出现重复设置ready的情况
    assert(list_empty(&t->ready_list));

    //
    priority_readyq_enqueue_tail(t);
    t->state = TASK_READY;

    if (t->priority < current->priority || current == &root_task) {
        set_need_schedule();
    }

    irq_restore(eflags);
}

void task_set_wait() {
    task_t* task = current;
    assert(task != NULL);
    assert(task != &root_task);
    assert(task->state == TASK_RUN);

    task->state = TASK_WAIT;
}

void task_init_lists(task_t* t) {
    assert(t != NULL);

    INIT_LIST_HEAD(&t->list);
    INIT_LIST_HEAD(&t->ready_list);
    INIT_LIST_HEAD(&t->waitq_list);
}

void task_init_stats(task_t* t) {
    assert(t != NULL);
    t->st_ticks = 0;
    t->st_last_exec_tsc = 0;
    t->st_runtime_tsc = 0;
    t->sched_cnt = 0;
    t->sched_keep_cnt = 0;
}

///

static volatile bool _need_schedule = false;

void set_need_schedule() {
    _need_schedule = true;
}

void clear_need_schedule() {
    _need_schedule = false;
}

bool need_schedule() {
    return _need_schedule;
}
