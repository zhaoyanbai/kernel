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
    set_cr3(tsk->cr3);
}

extern pde_t __initdata init_pgd[PDECNT_PER_PAGE] __attribute__((__aligned__(PAGE_SIZE)));

LIST_HEAD(all_tasks);
LIST_HEAD(ready_tasks);

void init_root_task() {
    int i;

    root_task.pid = get_next_pid();
    root_task.ppid = 0;
    root_task.state = TASK_RUN;
    root_task.reason = "root";
    root_task.priority = 7;
    root_task.ticks = root_task.priority;
    root_task.vma_list = NULL;
    root_task.sched_cnt = 0;
    root_task.sched_keep_cnt = 0;
    root_task.magic = TASK_MAGIC;
    strcpy(root_task.name, "root");

    task_init_lists(&root_task);

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

void setup_tasks() {
    INIT_LIST_HEAD(&all_tasks);
    INIT_LIST_HEAD(&ready_tasks);

    init_root_task();

    task_t_cache = kmem_cache_create("task_t", sizeof(task_t), PAGE_SIZE);
    if (0 == task_t_cache) {
        panic("setup tasks failed. out of memory");
    }
}

task_t* alloc_task_t() {
    task_t* task;
    task = (task_t*)kmem_cache_alloc(task_t_cache, 0);
    return task;
}

void switch_to() {
    set_cr3(current->cr3);
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

void schedule() {
    task_t* prev = current;
    task_t* next = NULL;

    unsigned long eflags;
    irq_save(eflags);

    // 把自己挂到就绪队列尾部
    if (prev->state == TASK_READY || prev->state == TASK_RUN) {
        task_set_ready(prev);
    }

    if (list_empty(&ready_tasks)) {
        next = &root_task;
        goto end;
    }

    // 从ready_tasks中选择第一个
    next = list_entry(ready_tasks.next, task_t, ready_list);

    assert(next->state == TASK_READY);
    assert(next != &root_task);

end:
    task_set_run(next);

    if (prev->ticks <= 0) {
        prev->ticks = prev->priority;
    }

    clear_need_schedule();

    if (prev != next) {
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

void task_set_run(task_t* t) {
    assert(t != NULL);

    // if (t == &root_task) {
    //     t->state = TASK_RUN;
    //     return;
    // }

    assert(t->state == TASK_READY);

    unsigned long eflags;
    irq_save(eflags);

    list_del_init(&t->ready_list);
    t->state = TASK_RUN;

    irq_restore(eflags);
}

void task_set_ready(task_t* t) {
    assert(t != NULL);
    if (t == &root_task) {
        t->state = TASK_READY;
        return;
    }

    unsigned long eflags;
    irq_save(eflags);
    if (!list_empty(&t->ready_list)) {
        list_del_init(&t->ready_list);
    }
    list_add_tail(&t->ready_list, &ready_tasks);
    t->state = TASK_READY;
    irq_restore(eflags);
}

void task_set_wait(task_t* t) {
    assert(t != NULL);
    // printk("task_set_wait %s %d\n", t->name, t->state);
    assert(t != &root_task);

    unsigned long eflags;
    irq_save(eflags);

    list_del_init(&t->ready_list);
    t->state = TASK_WAIT;

    irq_restore(eflags);
}

void task_init_lists(task_t* t) {
    assert(t != NULL);

    INIT_LIST_HEAD(&t->list);
    INIT_LIST_HEAD(&t->ready_list);
    INIT_LIST_HEAD(&t->waitq_list);
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