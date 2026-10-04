/*
 * ------------------------------------------------------------------------
 *   File Name: ioremap.c
 *      Author: Zhao Yanbai
 *              2026-01-02 13:38:43 Friday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <ioremap.h>
#include <page.h>
#include <assert.h>
#include <printk.h>

void ioremap_page_range(vaddr_t vaddr, paddr_t paddr, size_t size) {
    assert(PAGE_DOWN(size) == size);

    for (int i = 0; i < size / PAGE_SIZE; i++) {
        page_map(vaddr + i * PAGE_SIZE, paddr + i * PAGE_SIZE, PAGE_P | PAGE_WR);
    }
}

void ioremap_page_range_clear(vaddr_t vaddr, size_t size) {
    assert(PAGE_DOWN(size) == size);

    for (int i = 0; i < size / PAGE_SIZE; i++) {
        page_map(vaddr + i * PAGE_SIZE, 0, 0);
    }
}

void* ioremap(paddr_t phys_addr, size_t size) {
    size = PAGE_UP(size);

    vmap_region_t* vmap_region = alloc_vmap_region(size, VMAP_IOREMAP);
    if (!vmap_region) {
        return NULL;
    }

    // 映射物理地址到虚拟地址
    vmap_region->vm_paddr = phys_addr;
    vmap_region->vm_flags |= VMAP_IOREMAP;

    printk("ioremap: 0x%08x -> 0x%08x size %u\n", phys_addr, vmap_region->vm_vaddr, vmap_region->vm_size);

    ioremap_page_range(vmap_region->vm_vaddr, vmap_region->vm_paddr, vmap_region->vm_size);

    return (void*)vmap_region->vm_vaddr;
}

void iounmap(vaddr_t vaddr) {
    vmap_region_t* vmap_region = find_vmap_region(vaddr);
    assert(vmap_region != NULL);
    assert(vmap_region->vm_flags & VMAP_IOREMAP);

    printk("iounmap: 0x%08x size %u\n", vmap_region->vm_vaddr, vmap_region->vm_size);

    free_vmap_region(vmap_region);

    ioremap_page_range_clear(vmap_region->vm_vaddr, vmap_region->vm_size);
}
