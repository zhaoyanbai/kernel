/*
 * ------------------------------------------------------------------------
 *   File Name: page_fault.c
 *      Author: Zhao Yanbai
 *              2026-10-02 09:16:33 Friday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <sched.h>
#include <system.h>

#if 0
// #PF error code

    31                      16   15           7    6    5    4    3    2    1    0
+----+-----------------------+----+--//--+----+----+----+----+----+----+----+----+
| RMP|        reserved       | SGX|      |HLAT| SS | PK | I/D|RSVD|U/S |W/R | P  |
+----+-----------------------+----+--//--+----+----+----+----+----+----+----+----+

bit 0: P    0 - 页不存在; 1 - 页存在(权限/其它违例)
bit 1: W/R  0 - 读/取指; 1 - 写
bit 2: U/S  0 - 内核态(CPL<3); 1 - 用户态(CPL=3)
bit 3: RSVD 保留位冲突(写了必需为0的页表位)
bit 4: I/D  指令fetch导致的缺页异常。取指令(需PAE+NXE/SMEP支持)(开了NX想区分取指才用)
bit 5: PK   保护密钥违例(需要CR4.PKE=1)
bit 6: SS   影子栈访问(需要CET支持)
bit 7: HLAT HLAT分页找不到翻译(仅VMX+HLAT开启)
bit15: SGX  SGX访问违例(需要CR4.SGXE=1)
bit31: RMP  SEV-SNP的RMP访问违例(仅SNP开启)
32位非PAE只需关心低4位(P、W/R、U/S、RSVD)


1. U/S位永远反映的是触发时的CPL，不反映页表项的属性。用户态触发任何缺页，U/S一定为1
2. W/R们永远反映的是本次访问的动作，不反映页表项的R/W属性。做写操作就置1，做读/取指就是0
3. P=1时硬件就已经完成了页表遍历，所有级别的PTE都找到了且P=1。拒绝发生在最后一步权限检查时 -- MMU发现CPL和页表项的U/S不匹配，或发现写操作但页表项的R/W=0
4. 内核仅靠错误码无法区分"COW写保护"还是"真违规" 必须检查软件元数据
#endif

#if 0
US RW  P - Description
0  0  0 - Supervisory process tried to read a non-present page entry
0  0  1 - Supervisory process tried to read a page and caused a protection fault
0  1  0 - Supervisory process tried to write to a non-present page entry
0  1  1 - Supervisory process tried to write a page and caused a protection fault
1  0  0 - User process tried to read a non-present page entry
1  0  1 - User process tried to read a page and caused a protection fault
1  1  0 - User process tried to write to a non-present page entry
1  1  1 - User process tried to write a page and caused a protection fault
#endif

#if 0
    bit 0: 0 non-present page entry; 1 protection fault
    bit 1: 0 read; 1 write
    bit 2: 0 supervisor mode; 1 user mode
#endif

#if 0
核心判断逻辑:

STEP 1: P位判定 - 是缺页，还是权限违例
    if (errorcode & PF_P) == 0 :
        -> 页面不存在(P == 0)，是正常缺页
        -> 需要分配/映射物理页
    else :
        -> 页面存在(P == 1)，是权限违例
        -> 需要检查PDE/PTE的权限位，是否是写保护导致的
            -> 如果是写保护导致的，则需要进行写时复制(COW)处理
            -> 如果不是写保护导致的，则是非法访问，直接杀死进程

STEP 2: 对于缺页的情况 (P == 0)，看是谁、怎么触发的
    A: P=0, W/R=0, U/S=0:   内核读不存在的页
                            内核BUG或内核动态映射未建立
    B: P=0, W/R=0, U/S=1:   用户读不存在的页
                            正常缺页，可能是用户访问了未映射的页，需要分配物理页，建立映射
    C: P=0, W/R=1, U/S=0:   内核写不存在的页
                            同上，内核缺页处理
    D: P=0, W/R=1, U/S=1:   用户写不存在的页
                            正常缺页(写时分配)，建立映射


STEP 3: 对于权限违例的情况 (P == 1), 细分原因
    A: P=1, W/R=0, U/S=0:   内核读了被保护的页，如不可执行或特殊保护
                            内核BUG, panic
    B: P=1, W/R=0, U/S=1:   用户读内核页或NX页
                            段错误
    C: P=1, W/R=1, U/S=0:   内核写只读页
                            内核BUG，或COW场景，内核代表进程写
    D: P=1, W/R=1, U/S=1:   用户写只读页
                            COW缺页 或 段错误 (mprotect只读区)


STEP 4: 特殊位
    RSVD=1:     页表项保留位被写了1
                说明页表项被破坏了，内核BUG，直接panic
    I/D=1, P=1: 尝试执行不可执行页
                NX违规(PAE下)，段错误
    I/D=1, P=0: 取指令时页面不存在
                代码段缺页，按需加载
#endif

#define PF_P (1U << 0)
#define PF_WR (1U << 1)
#define PF_US (1U << 2)
#define PF_RSVD (1U << 3)
#define PF_ID (1U << 4)
#define PF_PK (1U << 5)
#define PF_SS (1U << 6)
#define PF_HLAT (1U << 7)
#define PF_SGX (1U << 15)
#define PF_RMP (1U << 31)

void do_no_page(vaddr_t vaddr);
void do_wp_page(void*);
void do_page_fault_handler(pt_regs_t regs) {
    vaddr_t pf_vaddr;
    u32 errcode = regs.errcode;

    asm("movl %%cr2,%%eax" : "=a"(pf_vaddr));

    printk("#PF: errcode 0x%08x addr 0x%08x [0x%08x] %s\n", errcode, pf_vaddr, current, current->name);

    if (errcode & PF_RSVD) {
        panic("#PF: reserved bit set, addr 0x%08x errcode 0xmake%08x\n", pf_vaddr, errcode);
    }

    // assert(errcode != 2 && errcode != 6);
    if ((errcode & PF_P) == 0) {
        printk("#PF: page not present\n");
        do_no_page(pf_vaddr);
    } else {
        printk("#PF: page protection violation\n");
        do_wp_page((void*)pf_vaddr);
    }
}
