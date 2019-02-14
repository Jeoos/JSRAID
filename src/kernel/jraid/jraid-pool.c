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

#include "jraid-pool.h"
#include "jraid-thread.h"
#include "jraid-lbd.h"
#include "jraid-io.h"
#include "jraid-dv.h"

#include <linux/spinlock.h>
#include <linux/slab.h>

#define MAX_STRIPE_BATCH	8

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


static void pool_do_work(struct work_struct *work)
{
        printk("in %s ...\n", __func__);
}

static int alloc_thread_groups(struct pconf *conf, int cnt,
			       int *group_cnt,
			       int *worker_cnt_per_group,
			       struct pworker_group **worker_groups)
{
	int i, j, k;
	ssize_t size;
	struct pworker *workers;

	*worker_cnt_per_group = cnt;
	if (cnt == 0) {
		*group_cnt = 0;
		*worker_groups = NULL;
		return 0;
	}
	*group_cnt = num_possible_nodes();
        printk("*group_cnt = %d\n", *group_cnt);
	size = sizeof(struct pworker) * cnt;
	workers = kzalloc(size * *group_cnt, GFP_NOIO);
	*worker_groups = kzalloc(sizeof(struct pworker_group) *
				*group_cnt, GFP_NOIO);
	if (!*worker_groups || !workers) {
		kfree(workers);
		kfree(*worker_groups);
		return -ENOMEM;
	}

	for (i = 0; i < *group_cnt; i++) {
		struct pworker_group *group;

		group = &(*worker_groups)[i];
		INIT_LIST_HEAD(&group->handle_list);
		group->conf = conf;
		group->workers = workers + i * cnt;

		for (j = 0; j < cnt; j++) {
			struct pworker *worker = group->workers + j;
			worker->group = group;
			INIT_WORK(&worker->work, pool_do_work);

			for (k = 0; k < NR_STRIPE_HASH_LOCKS; k++)
				INIT_LIST_HEAD(worker->temp_list + k);
		}
	}

	return 0;
}

static void pool_wakeup_stripe_thread(struct stripe_head *sh)
{
	struct pconf *conf = sh->conf;
        struct jraid_pool *jd_pool = conf->jd_pool;
	struct pworker_group *group;
	int thread_cnt;
	int i, cpu = sh->cpu;

	if (!cpu_online(cpu)) {
		cpu = cpumask_any(cpu_online_mask);
		sh->cpu = cpu;
	}

	group = conf->worker_groups + cpu_to_node(sh->cpu);

	group->workers[0].working = true;
	/* at least one worker should run to avoid race */
	queue_work_on(sh->cpu, jd_pool->lbds_wq, &group->workers[0].work);

	thread_cnt = group->stripes_cnt / MAX_STRIPE_BATCH - 1;
	/* wakeup more workers */
	for (i = 1; i < conf->worker_cnt_per_group && thread_cnt > 0; i++) {
		if (group->workers[i].working == false) {
			group->workers[i].working = true;
			queue_work_on(sh->cpu, jd_pool->lbds_wq,
				      &group->workers[i].work);
			thread_cnt--;
		}
	}
}

struct jraid_pool *sigle_pool_init(void)
{
        struct pconf *pconf;
	int group_cnt, worker_cnt_per_group;
	struct pworker_group *new_group;
        struct stripe_head *sh;

        jd_pool = kzalloc(sizeof(struct jraid_pool), GFP_KERNEL);
        if (!jd_pool) {
	        pr_debug("failed to alloc jd_pool.\n");
                goto err_alloc_pool;
        }
        /* 
         * FIXME: jraid pools each have two workqueue blow, the name should be differ 
         *
         * pool_wq: used for basic pool manager.
         * lbds_wq: for all lbds of current pool, to handle data stripe.
         */
	jd_pool->pool_wq = alloc_workqueue("poolwq", WQ_MEM_RECLAIM, 0);
	if (!jd_pool->pool_wq)
		goto err_pool_wq;

	jd_pool->lbds_wq = alloc_workqueue("lbdswq",
		WQ_UNBOUND|WQ_MEM_RECLAIM|WQ_CPU_INTENSIVE|WQ_SYSFS, 0);
	if (!jd_pool->lbds_wq)
		goto err_lbds_wq;

        strcpy(jd_pool->pool_name, "pool0");

        jd_pool->thread = jraid_register_thread(poold, jd_pool, POOL_THREAD, "poold") ;
        if (!jd_pool->thread) {
                goto abort;
        }
        
        spin_lock_init(&jd_pool->lock);
        INIT_LIST_HEAD(&jd_pool->lbds);
        INIT_LIST_HEAD(&jd_pool->dvs);

	pconf = kzalloc(sizeof(struct pconf), GFP_KERNEL);
        if (NULL == pconf)
                goto err_alloc_pconf;

        pconf->jd_pool = jd_pool;

        /* for simple test */
	sh = kzalloc(sizeof(struct stripe_head), GFP_KERNEL);
        sh->conf = pconf;
	sh->cpu = smp_processor_id();

        /* maybe sigle group, one worker(default) */
	if (!alloc_thread_groups(pconf, 1, &group_cnt, &worker_cnt_per_group,
				 &new_group)) {
		pconf->group_cnt = group_cnt;
		pconf->worker_cnt_per_group = worker_cnt_per_group;
		pconf->worker_groups = new_group;
        } else
                goto err_groups;

        pool_wakeup_stripe_thread(sh);

        return jd_pool;
err_groups:
        kfree(pconf);
err_alloc_pconf:
        if (jd_pool->thread)
                jraid_unregister_thread(&jd_pool->thread, POOL_THREAD);
abort:
	destroy_workqueue(jd_pool->lbds_wq);
err_lbds_wq:
	destroy_workqueue(jd_pool->pool_wq);
err_pool_wq:
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
