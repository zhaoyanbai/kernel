/*
 * ------------------------------------------------------------------------
 *   File Name: vmalloc.c
 *      Author: Zhao Yanbai
 *              2026-01-02 12:21:28 Friday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <vmalloc.h>
#include <assert.h>
#include <irq.h>
#include <page.h>
#include <assert.h>
#include <string.h>
#include <mm.h>

static vmap_region_t* vmap_region_list = NULL;

static const int vmap_guard_size = 4 * 1024 * 1024;  // 4MB

static const vaddr_t vmap_vaddr_base = VMALLOC_VADDR_BASE + vmap_guard_size;
static const vaddr_t vmap_vaddr_end = VMALLOC_VADDR_END - vmap_guard_size;

vmap_region_t* alloc_vmap_region(size_t size, uint32_t flags) {
    // assert size是页对齐
    assert(PAGE_DOWN(size) == size);
    assert(vmap_vaddr_base < vmap_vaddr_end);

    size += PAGE_SIZE;  // 尾部添加一个空页

    vaddr_t vmap_region_gap_bgn = 0;
    vaddr_t vmap_region_gap_end = 0;

    uint32_t eflags;
    irq_save(eflags);

    vmap_region_t* prev = NULL;
    vmap_region_t* curt = vmap_region_list;

    while (curt != NULL) {
        assert(curt->vm_vaddr >= vmap_vaddr_base);
        assert(curt->vm_vaddr + curt->vm_size <= vmap_vaddr_end);

        //
        vmap_region_gap_bgn = prev == NULL ? vmap_vaddr_base : prev->vm_vaddr + prev->vm_size;
        vmap_region_gap_end = curt->vm_vaddr;

        assert(vmap_region_gap_bgn <= vmap_region_gap_end);

        // 找到了
        if (vmap_region_gap_end - vmap_region_gap_bgn >= size) {
            break;
        }

        prev = curt;
        curt = curt->vm_next;
    }

    // 如果找到最后了也还没找到
    if (curt == NULL) {
        vmap_region_gap_bgn = prev == NULL ? vmap_vaddr_base : prev->vm_vaddr + prev->vm_size;
        vmap_region_gap_end = vmap_vaddr_end;

        assert(vmap_region_gap_bgn <= vmap_region_gap_end);

        if (vmap_region_gap_end - vmap_region_gap_bgn < size) {
            irq_restore(eflags);
            return NULL;
        }
    }

    // 创建一个vmap_region
    vmap_region_t* vmap_region = kmalloc(sizeof(vmap_region_t), 0);
    if (vmap_region == NULL) {
        irq_restore(eflags);
        return NULL;
    }

    vmap_region->vm_vaddr = vmap_region_gap_bgn;
    vmap_region->vm_size = size;
    vmap_region->vm_flags = flags;

    // add to list
    vmap_region->vm_next = curt;
    if (prev == NULL) {
        vmap_region_list = vmap_region;
    } else {
        prev->vm_next = vmap_region;
    }

    irq_restore(eflags);

    return vmap_region;
}

vmap_region_t* find_vmap_region(vaddr_t vaddr) {
    assert(PAGE_DOWN(vaddr) == vaddr);
    assert(vaddr >= vmap_vaddr_base);
    assert(vaddr < vmap_vaddr_end);
    assert(vmap_region_list != NULL);

    uint32_t eflags;
    irq_save(eflags);

    vmap_region_t* curt = vmap_region_list;

    while (curt != NULL) {
        assert(curt->vm_size != 0);
        assert(curt->vm_vaddr >= vmap_vaddr_base);
        assert(curt->vm_vaddr + curt->vm_size <= vmap_vaddr_end);

        if (vaddr == curt->vm_vaddr) {
            irq_restore(eflags);
            return curt;
        }

        assert(vaddr >= curt->vm_vaddr + curt->vm_size);

        curt = curt->vm_next;
    }

    irq_restore(eflags);

    return NULL;
}

void free_vmap_region(vmap_region_t* region) {
    assert(region != NULL);
    assert(region->vm_size != 0);
    assert(region->vm_size % PAGE_SIZE == 0);
    assert(region->vm_size >= (2 * PAGE_SIZE));
    assert(region->vm_vaddr >= vmap_vaddr_base);
    assert(region->vm_vaddr + region->vm_size <= vmap_vaddr_end);
    assert(vmap_region_list != NULL);
    // TODO

    uint32_t eflags;
    irq_save(eflags);

    vmap_region_t* prev = NULL;
    vmap_region_t* curt = vmap_region_list;

    while (curt != NULL) {
        if (curt == region) {
            if (prev == NULL) {
                // 只有链表上仅有一个节点，且这个节点就是要删除的节点的情况下prev才可能为NULL
                assert(vmap_region_list == region);
                assert(region->vm_next == NULL);

                //
                vmap_region_list = curt->vm_next;
            } else {
                prev->vm_next = curt->vm_next;
            }

            // 释放前先清空vmap_region避免连续二次释放
            memset(curt, 0, sizeof(vmap_region_t));

            kfree(curt);

            irq_restore(eflags);
            return;
        }

        prev = curt;
        curt = curt->vm_next;
    }

    // 不应该出现这种情况
    assert(0 && "vmap_region not found");
    irq_restore(eflags);
}

void* vmalloc(size_t size) {
    return NULL;
}

void vfree(void* addr) {
}
