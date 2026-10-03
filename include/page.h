/*
 *--------------------------------------------------------------------------
 *   File Name: page.h
 *
 * Description: none
 *
 *
 *      Author: Zhao Yanbai [zhaoyanbai@126.com]
 *
 *     Version:    1.0
 * Create Date: Sat Feb  7 21:47:42 2009
 * Last Update: Sat Feb  7 21:47:42 2009
 *
 *--------------------------------------------------------------------------
 */

#ifndef _PAGE_H
#define _PAGE_H

#include <kdef.h>
#include <processor.h>

/*

| CR0.PG  | CR4.PAE  | CR4.PSE  | PDE.PS  | PAGE SIZE | 物理地址规模 |
| ------- | -------- | -------- | ------- | --------- | ---------- |
| 0       | ×        | ×        | ×       | ―         | disabled   |
| 1       | 0        | 0        | ×       | 4KB       | 32bit      |
| 1       | 0        | 1        | 0       | 4KB       | 32bit      |
| 1       | 0        | 1        | 1       | 4MB       | 32bit      |
| 1       | 1        | ×        | 0       | 4KB       | 36bit      |
| 1       | 1        | ×        | 1       | 2MB       | 36bit      |

*/

#define PAGE_P 0x001   // 在内存中
#define PAGE_WR 0x002  // 表示可读写
#define PAGE_US 0x004  // 用户级
#define PAGE_G 0x100   // 全局页表项

#define PAGE_SHIFT (12)
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_FLAG_MASK ((1UL << PAGE_SHIFT) - 1)
#define PAGE_MASK (~PAGE_FLAG_MASK)
#define PAGE_OFFSET (KERNEL_VADDR_BASE)
#define PAGE_PDE_CNT 1024
#define PAGE_PTE_CNT 1024

// P：  有效位
//      0
//      表示当前表项无效，访问该虚拟地址直接#PF缺页异常，CPU不翻译，也不看其它位，此时余下31位CPU完全不管，内核可以随便用。
//      修改必须刷对应TLB
// R/W: 0 表示只读写就#PF; 1表示可读写。PDE和PTE要同时看，逻辑与。比如PDE.RW=0 && PTE.RW=1 最终还是只读。
//      另外还需要注意CR0.WP，如果CR0.WP=0(默认)，那么只限制用户态，内核写只读页也不缺页;
//      CR0.WP=1，那么内核写只读页也#PF。
// U/S: 0 表示只能0、1、2特权级可访问; 3 表示只有特权级程序可访问。
//      同样是PDE与PTE按逻辑与。
// PWT: 0 表示写进缓存，迟早刷回内存; 1 表示写缓存同时写内存，慢但内存总是新的。
//      普通内存映射一般PWT=0,对页表自身、设备内存特殊映射才考虑设置为1。
//      CR0.CD=1时这个位被忽略。
// PCD: 0 表示可以进cache; 1表示这一页不进CPU cache。
//      映射设备寄存器、MMIO、帧缓冲时用。普通RAM页不要开，开了性能会崩。
//      CR0.CD=1时这个位也被忽略。
// A:   0 表示该页未被访问; 1表示已被访问。
//      改此位从1清0时，如果P=1，建议顺手刷TLB，避免硬件状态不一致。
// D:   脏位。0表示该页未写过; 1表示该页被写过。只有PTE用这位，PDE这个位忽略或保留。
//      用于换出页面时判断要不要写回磁盘。D=0的页换出可直接丢弃，磁盘上已经有相同内容; D=1才需要写回。
//      清D位也要刷新TLB。
// PS:  PAGE SIZE.
// 只存在于页目录表项。在CR4中的PSE打开的情况下，0表示这是4KB页，指向一个页表。1表示这是4MB大页，直接指向物理页。 PAT:
// PAGE ATTRIBUTE TABLE. 只存在于页表项。 Pentium III后引入的机制。 G:   GLOBAL 全局位
//      4K分页下PDE.G被忽略，如果用4MB大页映射内核空间，PDE.G有效
//      内核空间映射在所有进程页表里都一样，设为全局，切换进程不刷新这些TLB项，减少内核映射的TLB miss。
//      用户页不能随便设G，会跨进程泄露TLB翻译。改G相关属性也需要注意TLB一致性。
// AVL: 内核可用
//      常见用法:
//          1. COW标记（软件位，不是硬件COW）
//          2. 页类型标记: 匿名页、文件页、设备页、共享页
//          3. 写时复制里记录"这是共享只读页"
//          4. 页面锁标记，禁止换出

// x

// |<------ 31~12------>|<------ 11~0 --------->| 比特
//                      |b a 9 8 7 6 5 4 3 2 1 0|
// |--------------------|-|-|-|-|-|-|-|-|-|-|-|-| 占位
// |<-------index------>| AVL |G|P|0|A|P|P|U|R|P| 属性
//                              |S|   |C|W|/|/|
//                                    |D|T|S|W|

#define PDE_P 0x001
#define PDE_RW 0x002
#define PDE_US 0x004
#define PDE_PWT 0x008
#define PDE_PCD 0X010
#define PDE_A 0x020
#define PDE_PS 0x080
#define PDE_G 0x100
#define PDE_AVL 0xE00

// |<------ 31~12------>|<------ 11~0 --------->| 比特
//                      |b a 9 8 7 6 5 4 3 2 1 0|
// |--------------------|-|-|-|-|-|-|-|-|-|-|-|-| 占位
// |<-------index------>| AVL |G|P|D|A|P|P|U|R|P| 属性
//                              |A|   |C|W|/|/|
//                              |T|   |D|T|S|W|
#define PTE_P 0x001
#define PTE_RW 0x002
#define PTE_US 0x004
#define PTE_PWT 0x008
#define PTE_PCD 0X010
#define PTE_A 0x020
#define PTE_D 0x040
#define PTE_PAT 0x080
#define PTE_G 0x100
#define PTE_AVL 0xE00

#ifndef ASM
#include <bits.h>
#include <types.h>
#define get_npde(addr) (((u32)(addr)) >> 22)
#define get_npte(addr) ((((u32)(addr)) >> 12) & 0x3FF)

#include <list.h>

typedef unsigned long pde_t;
typedef unsigned long pte_t;

#define PDECNT_PER_PAGE (PAGE_SIZE / sizeof(pde_t))
#define PTECNT_PER_PAGE (PAGE_SIZE / sizeof(pte_t))

#define PAGE_ITEMS (PAGE_SIZE / sizeof(unsigned long))
#define PAGE_ALIGN(page) (((unsigned long)(page)) & PAGE_MASK)
#define PAGE_UP(page) (((unsigned long)page + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_DOWN PAGE_ALIGN

#define PAGE_FLAGS(addr) ((addr) - PAGE_ALIGN(addr))

#define va2pa(x) (((unsigned long)(x)) - PAGE_OFFSET)
#define pa2va(x) ((void*)(((unsigned long)(x)) + PAGE_OFFSET))

// pfn: page frame number
#define pa2pfn(addr) ((addr) >> PAGE_SHIFT)
#define pfn2pa(pfn) ((void*)((pfn) << PAGE_SHIFT))

#define va2pfn(addr) pa2pfn(va2pa(addr))
#define pfn2va(pfn) pa2va(pfn2pa(pfn))

#define valid_va(addr) ((addr) >= PAGE_OFFSET)

#define PFN_UP(addr) (((addr) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DW(addr) ((addr) >> PAGE_SHIFT)

#define MAX_ORDER (11)

static inline pde_t* get_pgd() {
    return (pde_t*)pa2va((read_cr3() & PAGE_MASK));
}

enum page_flags {
    PG_Private,
};

struct kmem_cache;
typedef struct kmem_cache kmem_cache_t;

struct blk_buffer;
struct address_space;
typedef struct page {
    unsigned long count;
    unsigned long flags;
    unsigned long private;

    list_head_t lru;

    list_head_t list;

    struct page* head_page;  // buddy system
    unsigned int order;

    void** freelist;  // for slub

    unsigned long index;
    struct page* hash_next;
    struct address_space* mapping;

    kmem_cache_t* cache;

    unsigned long inuse;

    //
    struct blk_buffer* buffers;
} page_t;

void* page2va(page_t* page);
void* page2pa(page_t* page);
page_t* _va2page(unsigned long addr);
page_t* _pa2page(unsigned long addr);

#define va2page(addr) _va2page(PAGE_ALIGN(addr))
#define pa2page(addr) _pa2page(PAGE_ALIGN(addr))

static inline page_t* get_head_page(page_t* page) {
    return page->head_page;
}

#define __GETPAGEFLAG(name)                                \
    static inline int Page##name(page_t* page) {           \
        return constant_test_bit(PG_##name, &page->flags); \
    }

#define __SETPAGEFLAG(name)                               \
    static inline int SetPage##name(page_t* page) {       \
        return test_and_set_bit(PG_##name, &page->flags); \
    }

#define __CLEARPAGEFLAG(name)                               \
    static inline int ClearPage##name(page_t* page) {       \
        return test_and_clear_bit(PG_##name, &page->flags); \
    }

__GETPAGEFLAG(Private)
__SETPAGEFLAG(Private)
__CLEARPAGEFLAG(Private)

typedef struct free_area {
    unsigned long free_count;
    list_head_t free_list;
} free_area_t;

typedef enum {
    GFP_NONE = 0,
    GFP_ATOMIC = 1,
    GFP_KERNEL = 2,
    GFP_REWRITE = 3,  // 后续需要重新处理的接口,暂时写个值在这里提醒
} gfp_t;

page_t* alloc_pages(unsigned int order, gfp_t gfpflags);
void free_pages(unsigned long addr);

#define alloc_one_page(gfpflags) alloc_pages(0, gfpflags)

struct kmem_cache {
    const char* name;

    // 预期分配的大小
    unsigned long objsize;

    // 实际分配的大小
    unsigned long size;

    // 对齐
    unsigned long align;

    // 从buddy system批发的页的幂数
    unsigned long order;

    //
    unsigned long objects;

    // 没有完全分配的页的链表及数量
    unsigned int partial_cnt;
    list_head_t partial;

    // 正在分配的页
    page_t* page;

    list_head_t list;
};

void page_map(vaddr_t vaddr, paddr_t paddr, uint32_t flags);
void set_pte_paddr(vaddr_t vaddr, paddr_t paddr, uint32_t flags);

#endif  // ASM

#endif  //_PAGE_H
