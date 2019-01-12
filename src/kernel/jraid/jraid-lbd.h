/* 
 * jraid-lbd.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_LBD_H__
#define __JRAID_LBD_H__

struct local_block_device
{
        struct gendisk  *gendisk;

        struct request_queue  *queue;
        struct jraid_pool  *jd_pool;
	struct pool_personality	 *pers;
};

struct local_block_device *lbd_alloc(void);
void lbd_del(void);

#endif
