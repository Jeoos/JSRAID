/* 
 * jraid-main.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "jraid-thread.h"
#include "jraid-pool.h"
#include "jraid-lbd.h"

struct jraid_pool *jd_pool;
struct local_block_device *lbd;
extern struct pool_personality jraid_personality;
struct workqueue_struct *jraid_misc_wq;

static int __init jraid_main_init(void)
{
        int ret = 0;

	jraid_misc_wq = alloc_workqueue("jraid_misc", 0, 0);
	if (!jraid_misc_wq)
		goto err_misc_wq;

        register_pool_personality(&jraid_personality);

        /* FIXME: just for a sigle pool, one lbd now */
        jd_pool = sigle_pool_init();
        if (!jd_pool) {
                ret = EINVAL;
                goto err_pool_init;
        }

        lbd = lbd_alloc();
        if (!lbd) {
                ret = EINVAL;
                goto err_lbd_init;
        }

	spin_lock(&jd_pool->lock);
	list_add_tail(&lbd->list, &jd_pool->lbds);
	spin_unlock(&jd_pool->lock);

        if (jd_pool->thread)
                jraid_wakeup_thread(jd_pool->thread);

        return ret;

err_lbd_init:
        if (jd_pool->thread)
	        jraid_unregister_thread(&jd_pool->thread, POOL_THREAD);
        kfree(jd_pool);
err_pool_init:
        unregister_pool_personality(&jraid_personality);
	destroy_workqueue(jraid_misc_wq);
err_misc_wq:
        return ret;
}
  
static void __exit jraid_main_exit(void)
{
        lbd_del();
        if (jd_pool->thread)
	        jraid_unregister_thread(&jd_pool->thread, POOL_THREAD);
        if (jd_pool->pool_wq)
	        destroy_workqueue(jd_pool->pool_wq);

        kfree(jd_pool);

        unregister_pool_personality(&jraid_personality);
	destroy_workqueue(jraid_misc_wq);

        return;
}

module_init(jraid_main_init);
module_exit(jraid_main_exit);

MODULE_LICENSE("GPL");
