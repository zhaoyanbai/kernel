/*
 * ------------------------------------------------------------------------
 *   File Name: vmalloc.h
 *      Author: Zhao Yanbai
 *              2026-01-02 12:21:21 Friday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <kdef.h>

#define VMAP_ALLOC 0x01
#define VMAP_IOREMAP 0x02

// virtual map region
typedef struct vmap_region {
    vaddr_t vm_vaddr;
    size_t vm_size;
    paddr_t vm_paddr;
    uint32_t vm_flags;
    struct vmap_region* vm_next;
} vmap_region_t;

vmap_region_t* alloc_vmap_region(size_t size, uint32_t flags);
vmap_region_t* find_vmap_region(vaddr_t vaddr);

void free_vmap_region(vmap_region_t* region);
