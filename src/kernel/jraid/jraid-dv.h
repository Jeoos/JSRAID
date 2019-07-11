/* 
 * jraid-dv.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_DV_H__ 
#define __JRAID_DV_H__ 

#include <linux/blkdev.h>

#define MAX_NAME_LEN 56

struct disk_volume
{
        sector_t sectors; /* device size */
        struct block_device *bdev;
        /* jd_pool dvs */
        struct list_head plist;
        /* lbd dvs */
        struct list_head llist;
        char filename[MAX_NAME_LEN]; /* filename */
        int desc_nr;
        
        struct bio *bio;
};

#endif
