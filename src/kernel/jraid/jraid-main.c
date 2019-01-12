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

static int __init jraid_main_init(void)
{
        int ret = 0;

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

        if (jd_pool->sync_thread)
                pool_wakeup_thread(jd_pool->sync_thread);

        return ret;

err_lbd_init:
        if (jd_pool->sync_thread)
	        pool_unregister_thread(&jd_pool->sync_thread);
        kfree(jd_pool);
err_pool_init:
        unregister_pool_personality(&jraid_personality);
        return ret;
}
  
static void __exit jraid_main_exit(void)
{
        lbd_del();
        if (jd_pool->sync_thread) {
	        pool_unregister_thread(&jd_pool->sync_thread);
        }
        kfree(jd_pool);

        unregister_pool_personality(&jraid_personality);

        return;
}

module_init(jraid_main_init);
module_exit(jraid_main_exit);

MODULE_LICENSE("GPL");
