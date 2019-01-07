/* 
 * jraid-main.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/radix-tree.h>
#include <linux/bio.h>
#include <linux/kernel.h>

#include "ioctl_us.h"

#define BLKDEV_DISKNAME "lbd0"
#define BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define BLKDEV_SECTORS (200*1024*1024)

#define RADIX_TREE_TAG_DIRTY	0
#define RADIX_TREE_TAG_TSYNC    1

static struct gendisk *lbd_blkdev_disk;
static struct request_queue *lbd_blkdev_queue;

struct kmem_cache *_cache_blk;
struct dummy_device *blk_dev;

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

static blk_qc_t jraid_make_request(struct request_queue *q, struct bio *bi)
{
        bio_endio(bi);
        return BLK_QC_T_NONE;
}

static int __init jraid_main_init(void)
{
        int ret;

        lbd_blkdev_queue = blk_alloc_queue(GFP_KERNEL);
        if (!lbd_blkdev_queue)
                return -ENOMEM;

        blk_queue_make_request(lbd_blkdev_queue, jraid_make_request);
        if (!lbd_blkdev_queue) {
                ret = -ENOMEM;
                goto err_alloc_queue;
        }

        lbd_blkdev_disk = alloc_disk(1);
        if (!lbd_blkdev_disk) {
                ret = -ENOMEM;
                goto err_alloc_disk;
        }

        strcpy(lbd_blkdev_disk->disk_name, BLKDEV_DISKNAME);
        lbd_blkdev_disk->major = BLKDEV_DEVICEMAJOR;
        lbd_blkdev_disk->first_minor = 0;
        lbd_blkdev_disk->fops = &lbd_blkdev_fops;
        lbd_blkdev_disk->queue = lbd_blkdev_queue;

        set_capacity(lbd_blkdev_disk, BLKDEV_SECTORS);
        blk_queue_flush(lbd_blkdev_disk->queue, REQ_FLUSH | REQ_FUA);
        
        add_disk(lbd_blkdev_disk);

        return 0;

err_alloc_disk:
        blk_cleanup_queue(lbd_blkdev_queue);

err_alloc_queue:
        return ret;
}
  
static void __exit jraid_main_exit(void)
{
        del_gendisk(lbd_blkdev_disk);
        put_disk(lbd_blkdev_disk);

        blk_cleanup_queue(lbd_blkdev_queue);
        return;
}

module_init(jraid_main_init);
module_exit(jraid_main_exit);

MODULE_LICENSE("GPL");
