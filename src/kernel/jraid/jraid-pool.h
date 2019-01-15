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

#define MAX_POOL_NAME 32
#define NR_STRIPE_HASH_LOCKS 8

struct jraid_thread;
struct local_block_device;
extern spinlock_t pers_lock;
extern struct list_head pers_list;

struct pool_personality
{
	char *name;
	struct list_head list;

        /* pool tactics */
	sector_t (*sync_request)(struct local_block_device *lbd, sector_t sector_nr, int *skipped);
	void (*make_request)(struct local_block_device *lbd, struct bio *bi);
};

struct jraid_pool {
        spinlock_t  lock;
	struct list_head	lbds;
	char		        uuid[16];
        char        pool_name[MAX_POOL_NAME];

        struct jraid_thread      *thread;
        struct workqueue_struct  *pool_wq;
        struct workqueue_struct  *lbds_wq;
};

/* pool worker*/
struct pworker {
	struct work_struct work;
	struct pworker_group *group;
	struct list_head temp_list[NR_STRIPE_HASH_LOCKS];
	bool working;
};

struct pworker_group {
	struct list_head handle_list;
	struct pconf *conf;
	struct pworker *workers;
	int stripes_cnt;
};

/* pool config */
struct pconf {
        struct jraid_pool *jd_pool;
	struct pworker_group	*worker_groups;
	int			group_cnt;
	int			worker_cnt_per_group;
};


struct jraid_pool *sigle_pool_init(void);
int register_pool_personality(struct pool_personality *p);
int unregister_pool_personality(struct pool_personality *p);
struct pool_personality *find_pers(char *pname);

#endif
