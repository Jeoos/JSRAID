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
struct local_block_device;
extern spinlock_t pers_lock;
extern struct list_head pers_list;

struct pool_personality
{
	char *name;
	struct list_head list;

        /* pool tactics */
	void (*make_request)(struct local_block_device *lbd, struct bio *bi);
};

struct jraid_pool *sigle_pool_init(void);
int register_pool_personality(struct pool_personality *p);
int unregister_pool_personality(struct pool_personality *p);
struct pool_personality *find_pers(char *pname);

#endif
