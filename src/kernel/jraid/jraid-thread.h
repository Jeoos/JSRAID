/* 
 * jraid-thread.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define THREAD_WAKEUP 0
#define MAX_POOL_NAME 32

struct pool_thread {
	void			(*run) (struct pool_thread *thread);
	struct jraid_pool       *jd_pool;
	wait_queue_head_t	wqueue;
	unsigned long		flags;
	struct task_struct	*tsk;
	unsigned long		timeout;
	void			*private;
};

struct jraid_pool {
	struct list_head	lbds;
        struct pool_thread      *sync_thread;
	char		        uuid[16];
        char        pool_name[MAX_POOL_NAME];
};

static inline char *poolname(struct jraid_pool* jd_pool)
{
        return jd_pool->pool_name;
}

struct pool_thread *pool_register_thread(void (*run) (struct pool_thread *),
		struct jraid_pool *jd_pool, const char *name);
void pool_unregister_thread(struct pool_thread **threadp);
void pool_wakeup_thread(struct pool_thread *thread);
void pool_do_sync(struct pool_thread *thread);
