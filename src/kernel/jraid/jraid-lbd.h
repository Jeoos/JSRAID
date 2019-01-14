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

#define MAX_LBD_NAME 32
struct jraid_thread;

struct local_block_device
{
        struct gendisk  *gendisk;
        char        lbd_name[MAX_LBD_NAME];

        struct request_queue  *queue;
        struct jraid_pool  *jd_pool;
        struct jraid_thread *sync_thread;
	struct pool_personality	 *pers;

        struct list_head  list;
	struct work_struct misc_work;	/* used for misc items */
};

struct local_block_device *lbd_alloc(void);
void lbd_del(void);
void lbd_check_recovery(struct local_block_device *lbd);
void lbd_do_sync(struct jraid_thread *thread);

#endif
