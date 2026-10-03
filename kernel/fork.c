/*
 *--------------------------------------------------------------------------
 *   File Name: fork.c
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *              Sun Feb  7 13:25:28 2010
 *
 * Description: none
 *
 *--------------------------------------------------------------------------
 */

#include <page.h>
#include <sched.h>

extern pid_t get_next_pid();
extern list_head_t all_tasks;

void copy_page_tables(pde_t* pds_vaddr, pde_t* pdd_vaddr);

int do_fork(pt_regs_t* regs, unsigned long flags) {
    task_t* tsk;
    tsk = alloc_task();

    printd("fork task %08x flags %08x\n", tsk, flags);
    if (tsk == NULL) {
        panic("can not malloc PCB");
    }

    memcpy(tsk, current, sizeof(task_t));

    assert(tsk->magic == TASK_MAGIC);

    tsk->state = TASK_INITING;

    task_init_lists(tsk);
    task_init_stats(tsk);

    unsigned long eflags;
    irq_save(eflags);
    list_add(&tsk->list, &all_tasks);
    irq_restore(eflags);

    tsk->cr3 = (uint32_t)page2pa(alloc_one_page(0));
    assert(tsk->cr3 != 0);

    pde_t* pde_src = (pde_t*)pa2va(current->cr3);
    pde_t* pde_dst = (pde_t*)pa2va(tsk->cr3);

    memcpy((void*)pa2va(tsk->cr3), (void*)pa2va(current->cr3), PAGE_SIZE);

    irq_save(eflags);
    copy_page_tables(pde_src, pde_dst);
    irq_restore(eflags);

    pt_regs_t* child_regs = ((pt_regs_t*)(TASK_SIZE + (unsigned long)tsk)) - 1;

    // printd("child regs: %x %x\n", child_regs, regs);
    memcpy(child_regs, regs, sizeof(*regs));

    // child_regs->eflags |= 0x200;

    if (flags & FORK_KRNL) {
        strcpy(tsk->name, (char*)(child_regs->eax));
        child_regs->eax = 0;
    } else {
        child_regs->eip = *((unsigned long*) && fork_child);
    }
    printk("%s fork %s EFLAGS %08x\n", current->name, tsk->name, regs->eflags);

    // 这一句已经不需要了，通过fork_child已经能给子进程返回0了
    // child_regs->eax = 0;

    tsk->pid = get_next_pid();
    tsk->ppid = current->pid;
    tsk->priority = current->priority;

    tsk->ticks_left = TASK_TICKS_PER_QUANTUM;

    // for switch_to
    tsk->eip = child_regs->eip;
    tsk->esp = (unsigned long)child_regs;
    tsk->esp0 = TASK_SIZE + (unsigned long)tsk;

    printd("task %08x child_regs esp %08x esp0 %08x\n", tsk, tsk->esp, tsk->esp0);

    task_set_ready(tsk);

    void add_task_for_monitor(task_t * tsk);
    add_task_for_monitor(tsk);

    return (int)tsk->pid;

fork_child:
    return 0;
}

void copy_page_tables(pde_t* pds_vaddr, pde_t* pdd_vaddr) {
    assert(pds_vaddr != NULL);
    assert(pdd_vaddr != NULL);
    assert(PAGE_ALIGN(pds_vaddr) == (vaddr_t)pds_vaddr);
    assert(PAGE_ALIGN(pdd_vaddr) == (vaddr_t)pdd_vaddr);
    assert((vaddr_t)pds_vaddr >= KERNEL_VADDR_BASE);
    assert((vaddr_t)pdd_vaddr >= KERNEL_VADDR_BASE);

    const int knpde = get_npde(KERNEL_VADDR_BASE);
    for (int i = 0; i < PAGE_PDE_CNT; i++) {
        if (i < knpde) {
            // 把所有可写页设置为只读
            pds_vaddr[i] = pds_vaddr[i] & (~PDE_RW);
        }

        pdd_vaddr[i] = pds_vaddr[i];
    }
}

int sysc_fork(pt_regs_t regs) {
    return do_fork(&regs, 0);
}
