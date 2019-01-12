/* 
 * jraid-thread.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#include "jraid-thread.h"
#include "jraid-pool.h"


static int pool_thread(void *arg)
{
	struct pool_thread *thread = arg;

	/*
	 * pool_thread is a 'system-thread', it's priority should be very
	 * high. 
	 */

	allow_signal(SIGKILL);
	while (!kthread_should_stop()) {

		/* 
                 * We need to wait INTERRUPTIBLE so that
		 * we don't add to the load-average.
		 * That means we need to be sure no signals are
		 * pending
		 */
		if (signal_pending(current))
			flush_signals(current);

		wait_event_interruptible_timeout
			(thread->wqueue,
			 test_bit(THREAD_WAKEUP, &thread->flags)
			 || kthread_should_stop(),
			 thread->timeout);

		clear_bit(THREAD_WAKEUP, &thread->flags);
		if (!kthread_should_stop())
			thread->run(thread);
	}

	return 0;
}

void pool_wakeup_thread(struct pool_thread *thread)
{
	if (thread) {
		pr_debug("pool: waking up thread %s.\n", thread->tsk->comm);
		set_bit(THREAD_WAKEUP, &thread->flags);
		wake_up(&thread->wqueue);
	}
}

struct pool_thread *pool_register_thread(void (*run) (struct pool_thread *),
		struct jraid_pool *jd_pool, const char *name)
{
	struct pool_thread *thread;

	thread = kzalloc(sizeof(struct pool_thread), GFP_KERNEL);
	if (!thread)
		return NULL;

	init_waitqueue_head(&thread->wqueue);

	thread->run = run;
	thread->jd_pool = jd_pool;
	thread->timeout = MAX_SCHEDULE_TIMEOUT;
	thread->tsk = kthread_run(pool_thread, thread,
				  "%s_%s",
				  poolname(thread->jd_pool),
				  name);
	if (IS_ERR(thread->tsk)) {
		kfree(thread);
		return NULL;
	}
	return thread;
}

void pool_unregister_thread(struct pool_thread **threadp)
{
	struct pool_thread *thread = *threadp;
	if (!thread)
		return;
	pr_debug("interrupting pool-thread pid %d\n", task_pid_nr(thread->tsk));
	/* 
         * Locking ensures that jd_pool_unlock does not wake_up a
	 * non-existent thread
	 */
	spin_lock(&pers_lock);
	*threadp = NULL;
	spin_unlock(&pers_lock);

	kthread_stop(thread->tsk);
	kfree(thread);
}
