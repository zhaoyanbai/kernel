/*
 * ------------------------------------------------------------------------
 *   File Name: task_disk.c
 *      Author: Zhao Yanbai
 *              2026-01-25 09:53:51 Sunday CST
 * Description: none
 * ------------------------------------------------------------------------
 */

#include <disk.h>
#include <sync.h>
#include <sata.h>
// #include

disk_request_queue_t disk_request_queue;

void init_disk_request_queue() {
    INIT_LIST_HEAD(&disk_request_queue.list);
    mutex_init(&disk_request_queue.mutex);
    semaphore_init(&disk_request_queue.sem, 0);
}

int send_disk_request(disk_request_t* req) {
    assert(req != NULL);
    assert(req->count != 0);
    assert(req->buf != NULL);

    {
        const uint32_t size = req->count * 512;
        const uint32_t _64K = 1 << 16;
        assert(((((uint32_t)req->buf) & (_64K - 1)) + size) <= _64K);
    }

    completion_init(&req->completion);

    //
    mutex_lock(&disk_request_queue.mutex);
    list_add_tail(&req->list, &disk_request_queue.list);
    disk_request_queue.req_count++;
    disk_request_queue.pending_count++;
    mutex_unlock(&disk_request_queue.mutex);

    //
    semaphore_up(&disk_request_queue.sem);

    //
    completion_wait(&req->completion);

    return 0;
}

void disk_request(disk_request_t* req) {
    int minor = DEV_MINOR(req->dev);
    printk("disk_request dev %x minor %d\n", req->dev, minor);
    assert(minor == 0);  // 先简单支持
    assert(req->command == DISK_REQ_READ);

    sata_device_t* sata = &sata_devices[0];

    //
    completion_init(&sata->completion);

    sata_dma_read(sata, req->pos, req->count, (vaddr_t)req->buf);

    completion_wait(&sata->completion);
}

void disk_task_entry() {
    while (1) {
        semaphore_down(&disk_request_queue.sem);

        for (int i = 0; i < 123; i++) {
            asm("hlt");
        }

        mutex_lock(&disk_request_queue.mutex);
        assert(!list_empty(&disk_request_queue.list));
        disk_request_t* req = list_first_entry(&disk_request_queue.list, disk_request_t, list);
        list_del(&req->list);
        disk_request_queue.pending_count--;
        mutex_unlock(&disk_request_queue.mutex);

        //
        disk_request(req);

        //
        completion_complete(&req->completion);

        disk_request_queue.completed_count++;  // 不需要保护

        // for (int i = 0; i < 123; i++) {
        //     asm("hlt");
        // }
    }
}
