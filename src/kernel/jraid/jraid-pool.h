/* 
 * jraid-pool.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_POOL_H__
#define __JRAID_POOL_H__

#include <linux/bio.h>

struct jraid_pool;
extern spinlock_t pers_lock;
extern struct list_head pers_list;

struct pool_personality
{
	struct list_head list;

        /* pool tactics */
	void (*make_request)(struct jraid_pool *jd_pool, struct bio *bio);
};

int sigle_pool_init(void);
int register_pool_personality(struct pool_personality *p);
int unregister_pool_personality(struct pool_personality *p);

#endif
