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

struct local_block_device {
        struct gendisk  *gendisk;
        char        lbd_name[MAX_LBD_NAME];

        struct request_queue  *queue;
        struct jraid_pool  *jd_pool;
        struct jraid_thread *sync_thread;
	struct pool_personality	 *pers;

        spinlock_t lock;
        struct list_head  list;
        struct list_head  dvs;
	struct work_struct misc_work;	/* used for misc items */
};

/* lbd stripe head */
struct lstripe_head {
	struct hlist_node	hash;
	struct list_head	lru;	      /* inactive_list or handle_list */
	unsigned long		state;		/* state flags */

        struct pconf *conf;
        int cpu;

	sector_t		sector;		/* sector of this row */
	int			disks;		/* disks in stripe */
	spinlock_t		stripe_lock;
        atomic_t                count;        /* nr of active thread/requests */

	struct jddev {
		struct bio	req;
		struct bio_vec	vec;
		struct page	*page;
		struct bio	*toread, *read, *towrite, *written;
		sector_t	sector;			/* sector of this page */
		unsigned long	flags;
		u32		log_checksum;
	} dev[1]; /* allocated with extra space depending of JRAID geometry */
};

struct local_block_device *lbd_alloc(void);
void lbd_del(void);
void lbd_check_recovery(struct local_block_device *lbd);
void lbd_do_sync(struct jraid_thread *thread);

#endif
