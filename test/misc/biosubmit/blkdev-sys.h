/* 
 * jraid-sys.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __BLKDEV_SYS_H__
#define __BLKDEV_SYS_H__

#include <linux/spinlock.h>
#include <linux/blkdev.h>

#define MAX_NAME_LEN 16

struct Tblock_device {
        struct gendisk  *gendisk;
        struct block_device *bdev;
        struct list_head list;
        spinlock_t  dev_lock;

        char filename[MAX_NAME_LEN];
        char Tdev_name[MAX_NAME_LEN];

        struct request_queue  *queue;
	sector_t sectors;
};

int blkdev_sys_init(void);
void blkdev_sys_exit(void);

#endif
