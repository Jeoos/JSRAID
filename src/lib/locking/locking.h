/*
 * locking.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LOCKING_H__
#define __LOCKING_H__

#include <stdint.h>

struct logical_block_device;
struct cmd_context;

#define LCK_LBD_ACTIVATE 0
#define LCK_LBD_DEACTIVATE 1

#define lock_lbd_vol(cmd, lbd, flags) lock_vol(cmd, NULL, flags, lbd) 

#define lock_lbd(cmd, lbd, flags) \
({ \
	int rr = 0; \
\
        /* FIXME: lock activation */ \
        rr = lock_lbd_vol((cmd), (lbd), (flags)); \
        /* FIXME: unlock activation */ \
\
	rr; \
})

#define activate_lbd(cmd, lbd)        \
                lock_lbd(cmd, lbd, LCK_LBD_ACTIVATE)

#define deactivate_lbd(cmd, lbd)        \
                lock_lbd(cmd, lbd, LCK_LBD_DEACTIVATE)

typedef int (*lock_resource_fn) (struct cmd_context * cmd, const char *resource,
				 uint32_t flags, const struct logical_block_device *lbd);
typedef int (*query_resource_fn) (const char *resource, const char *node, int *mode);

typedef void (*fin_lock_fn) (void);
typedef void (*reset_lock_fn) (void);

struct locking_type {
	uint32_t flags;
	lock_resource_fn lock_resource;
	query_resource_fn query_resource;

	reset_lock_fn reset_locking;
	fin_lock_fn fin_locking;
};

int lock_vol(struct cmd_context *cmd, const char *vol, uint32_t flags, const struct logical_block_device *lbd);

int init_locking(int type, struct cmd_context *cmd, int suppress_messages);
#endif
