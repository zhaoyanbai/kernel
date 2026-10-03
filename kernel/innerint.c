/*
 *--------------------------------------------------------------------------
 *   File Name: innerint.c
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Mon Nov 10 15:58:46 2008
 * Last Update: Tue Feb 10 22:39:24 2009
 *     Version:    2.0
 * Last Update: Fri Jul 10 11:55:39 2009
 *
 *--------------------------------------------------------------------------
 */

#include <system.h>

#define DIE_MSG()                                                                                          \
    do {                                                                                                   \
        printk("Unsupport Now...[%s]\n", __FUNCTION__);                                                    \
        printk("EFLAGS:%08x CS:%02x EIP:%08x ERRCODE:%x\n", regs.eflags, regs.cs, regs.eip, regs.errcode); \
        asm("cli;hlt;");                                                                                   \
    } while (0);

void doDivideError(pt_regs_t regs) {
    DIE_MSG();
}
void doDebug(pt_regs_t regs) {
    DIE_MSG();
}
void doNMI(pt_regs_t regs) {
    DIE_MSG();
}
void doBreakPoint(pt_regs_t regs) {
    DIE_MSG();
}
void doOverFlow(pt_regs_t regs) {
    DIE_MSG();
}
void doBoundsCheck(pt_regs_t regs) {
    DIE_MSG();
}
void doInvalidOpcode(pt_regs_t regs) {
    DIE_MSG();
}
void doDeviceNotAvailable(pt_regs_t regs) {
    DIE_MSG();
}
void doDoubleFault(pt_regs_t regs) {
    DIE_MSG();
}
void doCoprocSegOverRun(pt_regs_t regs) {
    DIE_MSG();
}
void doInvalidTss(pt_regs_t regs) {
    DIE_MSG();
}
void doSegNotPresent(pt_regs_t regs) {
    DIE_MSG();
}
void doStackFault(pt_regs_t regs) {
    DIE_MSG();
}
void doGeneralProtection(pt_regs_t regs) {
    DIE_MSG();
}

void do_page_fault(pt_regs_t regs) {
    void do_page_fault_handler(pt_regs_t);
    do_page_fault_handler(regs);
}

void doCoprocError(pt_regs_t regs) {
    DIE_MSG();
}

// TODO 把printk换掉
#define AP_DIE_MSG()                                                                                       \
    do {                                                                                                   \
        printk("AP Unsupport Now...[%s]\n", __FUNCTION__);                                                 \
        printk("EFLAGS:%08x CS:%02x EIP:%08x ERRCODE:%x\n", regs.eflags, regs.cs, regs.eip, regs.errcode); \
        asm("cli;hlt;");                                                                                   \
    } while (0);

void doAPDivideError(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPDebug(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPNMI(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPBreakPoint(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPOverFlow(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPBoundsCheck(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPInvalidOpcode(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPDeviceNotAvailable(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPDoubleFault(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPCoprocSegOverRun(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPInvalidTss(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPSegNotPresent(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPStackFault(pt_regs_t regs) {
    AP_DIE_MSG();
}
void doAPGeneralProtection(pt_regs_t regs) {
    AP_DIE_MSG();
}

void doAPPageFault(pt_regs_t regs) {
    AP_DIE_MSG();
}

void doAPCoprocError(pt_regs_t regs) {
    AP_DIE_MSG();
}
