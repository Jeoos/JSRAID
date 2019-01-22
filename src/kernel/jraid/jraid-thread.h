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

#ifndef __JRAID_THREAD_H__
#define __JRAID_THREAD_H__

#define THREAD_WAKEUP 0

#include "jraid-pool.h"
#include "jraid-lbd.h"

/* deamon types */
typedef enum {
        POOL_THREAD,
        LBD_THREAD
} dtype_t;

struct jraid_thread {
	void			(*run) (struct jraid_thread *thread);
	void                    *d_struct;
	wait_queue_head_t	wqueue;
        dtype_t                 type;
	unsigned long		flags;
	struct task_struct	*tsk;
	unsigned long		timeout;
	void			*private;
};

static inline char *d_name(void *d_struct, dtype_t type)
{
        struct jraid_pool *jd_pool;
        struct local_block_device *lbd;

        switch (type) {
        case POOL_THREAD:
                jd_pool = (struct jraid_pool *)d_struct;
                return jd_pool->pool_name;
        case LBD_THREAD:
                lbd = (struct local_block_device *)d_struct;
                return lbd->lbd_name;
        default:
                pr_debug("err thread type input.\n");
                return NULL;
        }
        return NULL;
}

struct jraid_thread *jraid_register_thread(void (*run) (struct jraid_thread *),
		void *d_struct, dtype_t type, const char *name);
void jraid_unregister_thread(struct jraid_thread **threadp, dtype_t type);
void jraid_wakeup_thread(struct jraid_thread *thread);

#endif
