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

LIST_HEAD(pers_list);
DEFINE_SPINLOCK(pers_lock);
extern struct jraid_pool *jd_pool;

void pool_do_sync(struct pool_thread *thread)
{
        printk("In %s ...\n", __func__);
}

struct jraid_pool *sigle_pool_init(void)
{
        jd_pool = kmalloc(sizeof(struct jraid_pool), GFP_KERNEL);
        if (!jd_pool) {
	        pr_debug("failed to alloc jd_pool.\n");
                goto err_alloc_pool;
        }

        jd_pool->sync_thread = pool_register_thread(pool_do_sync, jd_pool, "resync") ;
        if (!jd_pool->sync_thread) {
                goto abort;
        }
        return jd_pool;
abort:
        kfree(jd_pool);
err_alloc_pool:
        return NULL;
}

static void make_request(struct local_block_device *lbd, struct bio * bi)
{
       printk("in %s ...\n", __func__);
}

struct pool_personality jraid_personality =
{
        .name = "jraid",
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
