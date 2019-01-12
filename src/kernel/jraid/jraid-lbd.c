/* 
 * jraid-lbd.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/blkdev.h>
#include <linux/slab.h>

#include "ioctl_us.h"
#include "jraid-lbd.h"
#include "jraid-io.h"
#include "jraid-pool.h"

#define BLKDEV_DISKNAME "lbd0"
#define BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define BLKDEV_SECTORS (200*1024*1024)

extern struct local_block_device *lbd;

static int lbd_open(struct block_device *bdev, fmode_t mode)
{
        return 0;
}

static void lbd_release(struct gendisk *disk, fmode_t mode)
{
        return;
}

static inline bool lbd_ioctl_valid(unsigned int cmd)
{
        switch (cmd) {
        case TEST_0:
        case TEST_1:
                return true;
        default:
                return false;
        }
}

static int lbd_ioctl(struct block_device *bdev, fmode_t mode,                    
                        unsigned int cmd, unsigned long arg)                    
{ 

        if(!lbd_ioctl_valid(cmd))
                return -ENOTTY;

        switch (cmd) {
        case TEST_0:
                goto out_deal;
        case TEST_1:
                break;
        default:
                ;
        }

out_deal:
        printk("do something ...\n");

        return 0;
}

static const struct block_device_operations lbd_blkdev_fops = {
        .owner          = THIS_MODULE,
        .open           = lbd_open,
        .release        = lbd_release,
        .ioctl          = lbd_ioctl, 
};

struct local_block_device *lbd_alloc(void)
{
        struct local_block_device *lbd;
        lbd = kmalloc(sizeof(struct local_block_device), GFP_KERNEL);
        if (!lbd) {
                goto err_alloc_lbd;
        }

        lbd->queue = blk_alloc_queue(GFP_KERNEL);
        if (!lbd->queue) {
                goto err_alloc_queue;
        }
        lbd->queue->queuedata = lbd;

        lbd->pers = find_pers("jraid");
        if (!lbd->pers) {
                goto err_find_pers;
        }

        blk_queue_make_request(lbd->queue, jraid_make_request);

        lbd->gendisk = alloc_disk(1);
        if (!lbd->gendisk) {
                goto err_alloc_disk;
        }

        strcpy(lbd->gendisk->disk_name, BLKDEV_DISKNAME);
        lbd->gendisk->major = BLKDEV_DEVICEMAJOR;
        lbd->gendisk->first_minor = 0;
        lbd->gendisk->fops = &lbd_blkdev_fops;
        lbd->gendisk->queue = lbd->queue;

        set_capacity(lbd->gendisk, BLKDEV_SECTORS);
        blk_queue_flush(lbd->gendisk->queue, REQ_FLUSH | REQ_FUA);
        
        add_disk(lbd->gendisk);

        return lbd;	

err_alloc_disk:
err_find_pers:
        blk_cleanup_queue(lbd->queue);
err_alloc_queue:
        kfree(lbd);
err_alloc_lbd:
        return NULL;
}

void lbd_del()
{
        blk_cleanup_queue(lbd->queue);

        del_gendisk(lbd->gendisk);
        put_disk(lbd->gendisk);

        kfree(lbd);
}
