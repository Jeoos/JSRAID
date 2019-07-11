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
#define LSTRIPE_SIZE    PAGE_SIZE
#define LSTRIPE_SECTORS (LSTRIPE_SIZE>>9)

#define	SECTOR_SHIFT	9
#define	PAGE_SECTORS_SHIFT	(PAGE_SHIFT - SECTOR_SHIFT)
#define PAGE_SECTORS		(1 << PAGE_SECTORS_SHIFT)

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

static void pool_wakeup_stripe_thread(struct lstripe_head *lsh)
{
	struct pconf *conf = lsh->conf;
        struct jraid_pool *jd_pool = conf->jd_pool;
	struct pworker_group *group;
	int thread_cnt;
	int i, cpu = lsh->cpu;

	if (!cpu_online(cpu)) {
		cpu = cpumask_any(cpu_online_mask);
		lsh->cpu = cpu;
	}

	group = conf->worker_groups + cpu_to_node(lsh->cpu);

	group->workers[0].working = true;
	/* at least one worker should run to avoid race */
	queue_work_on(lsh->cpu, jd_pool->lbds_wq, &group->workers[0].work);

	thread_cnt = group->stripes_cnt / MAX_STRIPE_BATCH - 1;
	/* wakeup more workers */
	for (i = 1; i < conf->worker_cnt_per_group && thread_cnt > 0; i++) {
		if (group->workers[i].working == false) {
			group->workers[i].working = true;
			queue_work_on(lsh->cpu, jd_pool->lbds_wq,
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
        struct lstripe_head *lsh;

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
	lsh = kzalloc(sizeof(struct lstripe_head), GFP_KERNEL);
        lsh->conf = pconf;
	lsh->cpu = smp_processor_id();

        /* maybe sigle group, one worker(default) */
	if (!alloc_thread_groups(pconf, 1, &group_cnt, &worker_cnt_per_group,
				 &new_group)) {
		pconf->group_cnt = group_cnt;
		pconf->worker_cnt_per_group = worker_cnt_per_group;
		pconf->worker_groups = new_group;
        } else
                goto err_groups;

        pool_wakeup_stripe_thread(lsh);

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

static struct lstripe_head *alloc_lstripe(struct kmem_cache *lsc, gfp_t gfp)
{
	struct lstripe_head *lsh;

	lsh = kmem_cache_zalloc(lsc, gfp);
	if (lsh) {
		spin_lock_init(&lsh->stripe_lock);
		INIT_LIST_HEAD(&lsh->lru);
		atomic_set(&lsh->count, 1);
	}
	return lsh;
}

static inline sector_t sync_request(struct local_block_device *lbd, sector_t sector_nr, int *skipped)
{
        return 0;
}

sector_t jraid_compute_blocknr(struct lstripe_head *lsh, int i)
{
        return 0;
}

static void jraid_build_block(struct lstripe_head *lsh, int i)
{
	struct jddev *dev = &lsh->dev[i];

	bio_init(&dev->req);
	dev->req.bi_io_vec = &dev->vec;
	dev->req.bi_max_vecs = 1;
	dev->req.bi_private = lsh;

	dev->flags = 0;
	dev->sector = jraid_compute_blocknr(lsh, i);
}

static void init_lstripe(struct lstripe_head *lsh, sector_t sector)
{
        int i;
        BUG_ON(atomic_read(&lsh->count) != 0);
        lsh->sector = sector;

        for (i = lsh->disks; i++; ) {
                struct jddev *dev = &lsh->dev[i];
                jraid_build_block(lsh, i);
        }
        lsh->cpu = smp_processor_id();
}

static void make_request(struct local_block_device *lbd, struct bio *bi)
{
        int rw = bio_data_dir(bi);
	sector_t logical_sector, last_sector;
        struct lstripe_head *lsh;
        struct kmem_cache *lsc;

        printk("in %s ...\n", __func__);
	if (unlikely(bi->bi_rw & REQ_FLUSH)) {
                /* FIXME: jraid_flush_request. */
                return ;
        }

	if (unlikely(bi->bi_rw & REQ_DISCARD)) {
                /* FIXME: make_discard_request. */
                return ;
        }

        /* simple multi dv write by lbd stripe. */
	lsc = kmem_cache_create("lstripe_head kmem_cache",
			       sizeof(struct lstripe_head) + 2*sizeof(struct jddev),
			       0, 0, NULL);
	if (!lsc)
		return ;

        lsh = alloc_lstripe(lsc, GFP_KERNEL);

        init_lstripe(lsh, logical_sector);

	logical_sector = bi->bi_iter.bi_sector & ~((sector_t)LSTRIPE_SECTORS-1);
	last_sector = bio_end_sector(bi);

	for (;logical_sector < last_sector; logical_sector += LSTRIPE_SECTORS) {
                printk("in %s logical_sector=%lu ...\n", __func__, logical_sector);
        }

        kmem_cache_free(lsc, lsh);
}

/* actually differ from write and read needed */
static void io_end_bio(struct bio *bio)
{
	struct disk_volume *dv = bio->bi_private;
        printk(KERN_INFO "in end bio\n");

        if (bio->bi_error) {
	        printk(KERN_ERR "%s-%d: Not uptodata bio try again !%ld \n",
	                __func__, __LINE__, bio->bi_iter.bi_sector);
                /* set blk back to original state, try again */
	        bio_put(bio);
                return ;
        }
	smp_mb();

        /* FIXME: set blk mapped to disk */
	bio_put(bio);
}

struct submit_bio_ret {
        struct completion event;
        int error;
};

static void _submit_bio_wait_endio(struct bio *bio, int error)
{
        struct submit_bio_ret *ret = bio->bi_private;
        printk("in[%s] ...\n", __func__);

        ret->error = error;
        complete(&ret->event);
}

int _submit_bio_wait(int rw, struct bio *bio)
{
        struct submit_bio_ret ret;

        printk("in[%s] ...\n", __func__);
        rw |= REQ_SYNC;
        init_completion(&ret.event);
        bio->bi_private = &ret;
        bio->bi_end_io = _submit_bio_wait_endio;
        submit_bio(rw, bio);
        wait_for_completion(&ret.event);

        return ret.error;
}

int submit_readwrite(int rw, struct disk_volume *dv, struct page *page, sector_t sector)
{
	struct bio *bio;
	int ret = 0;
	
	bio = bio_kmalloc(GFP_KERNEL, 1);
	if (!bio) {
		printk(KERN_ERR "%s-%d: Cannot alloc bio\n", __func__, __LINE__);
		return -ENOMEM;
	}
	/* init bio */
	bio->bi_bdev = dv->bdev;
        printk("bio->bi_sector = %u\n", sector);
	bio->bi_iter.bi_sector = (sector / PAGE_SECTORS)*PAGE_SECTORS;
        /* FIXME: */
        if (bio->bi_iter.bi_sector >= dv->sectors) {
                bio->bi_iter.bi_sector = 0;
                goto out;
        }
	bio->bi_private = dv;

	bio->bi_rw |= REQ_FAILFAST_DEV |
		REQ_FAILFAST_TRANSPORT |
		REQ_FAILFAST_DRIVER;

	bio_add_page(bio, page, PAGE_SIZE, 0);

	//bio->bi_end_io = io_end_bio;
	//submit_bio(rw, bio);
        
        if (rw == WRITE) {
                printk("WRITE ... \n");
	        //ret = _submit_bio_wait(WRITE, bio);
	        ret = submit_bio_wait(WRITE, bio);
                if (ret)
                        printk(KERN_ERR "failed to wait submit WRITE\n");
        } else {
                printk("READ ... \n");
	        //ret = _submit_bio_wait(READ, bio);
	        ret = submit_bio_wait(READ, bio);
                if (ret)
                        printk(KERN_ERR "failed to wait submit READ\n");
        }

out:
        bio_put(bio);

	return 0;
}

static int non_stripe_write(struct disk_volume *dv, struct page *page, unsigned int len, 
                unsigned int off, sector_t sector, uint64_t bi_sector, uint32_t bi_size)
{
	unsigned int offset = (sector & (PAGE_SECTORS -1)) << SECTOR_SHIFT;
	unsigned int copy = min_t(unsigned int, len, PAGE_SIZE - offset);
        char sectors;
        int nr,i,index;

        void *dst = NULL, *src = NULL;
        src = page_address(page) + off;
        nr = (len > (PAGE_SIZE - offset)) ? 2 : 1;

	for (i=0; i<nr; i++) {
                page = alloc_page(GFP_KERNEL);
                if (!page) {
                        printk(KERN_ERR "%s-%d: Cannot alloc page\n", __func__, __LINE__);
                        return -ENOMEM;
                }
                sectors = copy >> SECTOR_SHIFT;
                src += i * (len-copy);
                dst = page_address(page) + offset;
                memcpy(dst, src, copy);

                submit_readwrite(WRITE, dv, page, sector);

		sector += sectors;
		copy = len - copy;
		offset = 0;
                /* FIXME: async write, wait for io_end_bio */
                __free_pages(page, 0);
        }
        return 0;
}

static int non_stripe_read(struct disk_volume *dv, struct page *page, unsigned int len, 
                        unsigned int off, sector_t sector)
{
	unsigned int offset = (sector & (PAGE_SECTORS - 1)) << SECTOR_SHIFT;
	unsigned int copy = min_t(unsigned int, len, PAGE_SIZE - offset);
	char sectors;
	int nr, i;
	void *dst = NULL, *src = NULL;
	dst = page_address(page) + off;
	nr = (len > (PAGE_SIZE - offset)) ? 2 : 1;

        for(i = 0; i < nr; i++) {
                page = alloc_page(GFP_KERNEL);
                if (!page) {
                        printk(KERN_ERR "%s-%d: Cannot alloc page\n", __func__, __LINE__);
                        return -ENOMEM;
                }
                sectors = copy >>SECTOR_SHIFT;          
		dst += i * (len-copy);

	        src = page_address(page)+offset;
                submit_readwrite(READ, dv, page, sector);

                /* FIXME: async read, wait for io_end_bio */
	        memcpy(dst, src, copy);
		sector += sectors;
		copy = len - copy;
		offset = 0;
                __free_pages(page, 0);
        }
        return 0;
}

/*
 * for non stripe io request
 */
static int _make_request(struct local_block_device *lbd, struct bio* bi)
{
        int rw, i, err;
        struct disk_volume *dv;
	struct bio_vec bvl;
	struct bvec_iter iter;
        sector_t sector = bi->bi_iter.bi_sector;

        list_for_each_entry(dv, &lbd->dvs, llist) {
                printk("the dv desc_nr = %u\n", dv->desc_nr);
                dv->bio = bi;
                bio_for_each_segment(bvl, bi, iter) {
                        if(rw == WRITE)
                                err = non_stripe_write(dv, bvl.bv_page, bvl.bv_len,
                                        bvl.bv_offset, sector, bi->bi_iter.bi_sector, bi->bi_iter.bi_size);
                        else
                                err = non_stripe_read(dv, bvl.bv_page, bvl.bv_len,
                                        bvl.bv_offset, sector);
                        if (err) break;
                        sector += bvl.bv_len >> SECTOR_SHIFT;
                }
                break;
        }
        bio_endio(bi);
}

struct pool_personality jraid_personality =
{
        .name = "jraid",
        .sync_request = sync_request,
        //.make_request = make_request,
        .make_request = _make_request,
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
