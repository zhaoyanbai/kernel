/*
 *--------------------------------------------------------------------------
 *   File Name: syscall.c
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Fri Jan  2 19:49:20 2009
 * Last Update: Fri Jan  2 19:49:20 2009
 *
 *--------------------------------------------------------------------------
 */
#include "syscall.h"

#include "msr.h"
#include "sched.h"
#include "system.h"
#include "timer.h"
#include "clock.h"
#include "cpuid.h"

extern void syscall_entry();
extern void init_sysc_handler_table();

unsigned long sysc_handler_table[SYSC_NUM];

void setup_sysc() {
    wrmsr(MSR_SYSENTER_CS, SELECTOR_KRNL_CS, 0);
    wrmsr(MSR_SYSENTER_EIP, syscall_entry, 0);
    wrmsr(MSR_SYSENTER_ESP, &(tss.esp0), 0);
    init_sysc_handler_table();
}

int sysc_none() {
    int sysc_nr;
    asm("" : "=a"(sysc_nr));
    printk("unsupport syscall:%d\n", sysc_nr);

    return 0;
}

static void timer_waitq_wakeup_one_cb(void* arg) {
    waitq_t* waitq = (waitq_t*)arg;
    waitq_wakeup_one(waitq);
}

// 特别说明：如果想把这个函数的参数ticks改为int64_t
// 那么就需要在编写用户级的系统调用库函数的时候注意
// 不仅需要填写 ebx，还要填写 ecx
// 不然就可能出现诡异的一直WAIT，不会调度到该任务的问题
int sysc_wait(int ticks) {
    if (ticks < 0) {
        return -EINVAL;
    } else {
        unsigned long flags;
        irq_save(flags);
        assert(current->state != TASK_WAIT);

        waitq_t waitq;
        waitq_init(&waitq);

        timer_t timer;
        timer_init(&timer, jiffies + ticks, timer_waitq_wakeup_one_cb, &waitq);
        timer_add(&timer);

        waitq_sleep(&waitq);

        timer_del(&timer);

        irq_restore(flags);
    }

    return 0;
}

int sysc_test() {
    // 测试修改优先级
    int priority = current->priority;
    priority += 1;
    priority = TASK_PRIORITY_LEVEL_USER + (priority % 10);
    task_reset_priority(priority);
    return 0;
}
int sysc_pause() {
    return 0;
}

int sysc_debug(unsigned int v) {
    static unsigned int cnt = 0;
    cnt++;
    return 0;
}

int sysc_rand() {
    bool has_rdrand = false;
    cpuid_regs_t cpuid_regs = cpuid(1);
    if (cpuid_regs.ecx & CPUID_FEAT_ECX_RDRAND) {
        has_rdrand = true;
    }

    uint32_t rand = jiffies;
    static uint32_t rand_seed = 0;

    if (has_rdrand) {
        asm volatile("rdrand %0" : "=r"(rand) : : "cc");
    } else {
        if (0 == rand_seed) {
            rand_seed = jiffies;
            int cnt = (jiffies % 32);
            for (int i = 0; i < cnt; i++) {
                uint32_t bit = rand_seed & 1;
                rand_seed >>= 1;
                rand_seed |= (bit << 31);
            }
            rand_seed ^= ((uint32_t)rdtsc());
        }

        rand_seed = rand_seed * 1664525u + 1013904223u;
        rand = rand_seed;
    }

    return (int)rand;
}

void init_sysc_handler_table() {
    int i;
    for (i = 0; i < SYSC_NUM; i++)
        sysc_handler_table[i] = (unsigned long)sysc_none;

#define _sysc_(nr, sym)                              \
    do {                                             \
        extern int sym();                            \
        sysc_handler_table[nr] = (unsigned long)sym; \
    } while (0);

    _sysc_(SYSC_WRITE, sysc_write);
    _sysc_(SYSC_REBOOT, sysc_reboot);
    _sysc_(SYSC_FORK, sysc_fork);
    _sysc_(SYSC_EXEC, sysc_exec);
    _sysc_(SYSC_WAIT, sysc_wait);
    _sysc_(SYSC_OPEN, sysc_open);
    _sysc_(SYSC_READ, sysc_read);
    _sysc_(SYSC_STAT, sysc_stat);
    _sysc_(SYSC_EXIT, sysc_exit);
    _sysc_(SYSC_PAUSE, sysc_pause);
    _sysc_(SYSC_TEST, sysc_test);
    _sysc_(SYSC_DEBUG, sysc_debug);
    _sysc_(SYSC_RAND, sysc_rand);
    _sysc_(SYSC_BAD_NR, sysc_bad_nr);
}

int sysc_bad_nr() {
    int sysc_nr;

    asm("" : "=a"(sysc_nr));

    printk("bad syscall nr:%d\n", sysc_nr);

    return -1;
}

void sysc_check_resched() {
    unsigned long eflags;
    irq_save(eflags);
    if (need_schedule()) {
        if (!IN_CRITICAL_ZONE()) {
            schedule();
        }
    }
    irq_restore(eflags);
}