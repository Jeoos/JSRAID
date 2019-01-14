/* 
 * jraid-pool.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/spinlock.h>
#include <linux/slab.h>

#include "jraid-pool.h"
#include "jraid-thread.h"
#include "jraid-lbd.h"

LIST_HEAD(pers_list);
DEFINE_SPINLOCK(pers_lock);
extern struct jraid_pool *jd_pool;

void poold(struct jraid_thread *thread)
{
        struct local_block_device *lbd;
        struct jraid_pool *jd_pool = (struct jraid_pool *)thread->d_struct;

        printk("In %s ...\n", __func__);

        /* check for each lbd belongs to the current pool.*/
        list_for_each_entry(lbd, &jd_pool->lbds, list) {
                lbd_check_recovery(lbd);
        }
}

struct jraid_pool *sigle_pool_init(void)
{
        jd_pool = kmalloc(sizeof(struct jraid_pool), GFP_KERNEL);
        if (!jd_pool) {
	        pr_debug("failed to alloc jd_pool.\n");
                goto err_alloc_pool;
        }
        /* FIXME: jraid pools each have one workqueue, the name should be differ */
	jd_pool->pool_wq = alloc_workqueue("poolwq", WQ_MEM_RECLAIM, 0);
	if (!jd_pool->pool_wq)
		goto err_wq;

        strcpy(jd_pool->pool_name, "pool0");

        jd_pool->thread = jraid_register_thread(poold, jd_pool, POOL_THREAD, "poold") ;
        if (!jd_pool->thread) {
                goto abort;
        }
        
        spin_lock_init(&jd_pool->lock);
        INIT_LIST_HEAD(&jd_pool->lbds);

        return jd_pool;
abort:
	destroy_workqueue(jd_pool->pool_wq);
err_wq:
        kfree(jd_pool);
err_alloc_pool:
        return NULL;
}

static inline sector_t sync_request(struct local_block_device *lbd, sector_t sector_nr, int *skipped)
{
        return 0;
}

static void make_request(struct local_block_device *lbd, struct bio * bi)
{
       printk("in %s ...\n", __func__);
}

struct pool_personality jraid_personality =
{
        .name = "jraid",
        .sync_request = sync_request,
        .make_request = make_request,
};

struct pool_personality *find_pers(char *pname)
{
        struct pool_personality *pers;
        list_for_each_entry(pers, &pers_list, list) {
                if (strcmp(pers->name, pname) == 0)
                        return pers;
        }
        return NULL;
}

int register_pool_personality(struct pool_personality *p)
{
	spin_lock(&pers_lock);
	list_add_tail(&p->list, &pers_list);
	spin_unlock(&pers_lock);
	return 0;
}

int unregister_pool_personality(struct pool_personality *p)
{
	spin_lock(&pers_lock);
	list_del_init(&p->list);
	spin_unlock(&pers_lock);
	return 0;
}
