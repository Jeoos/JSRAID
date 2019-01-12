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

struct disk_volume
{
        sector_t sectors; /* device size */
        struct block_device *bdev;
};

#endif
