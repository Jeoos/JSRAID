/*
 * locking.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

 #include "locking.h"
 #include "activate.h"
 #include <string.h>
 #include <stdio.h>

static struct locking_type _locking;

static int _lock_vol(struct cmd_context *cmd, const char *resource,
		     uint32_t flags, const struct logical_block_device *lbd)
{
        int ret = 0;

        ret = _locking.lock_resource(cmd, resource, flags, lbd);

        return ret;
}

int lock_vol(struct cmd_context *cmd, const char *vol, uint32_t flags, const struct logical_block_device *lbd)
{
	char resource[258] __attribute__((aligned(8)));

	if (vol && !strncpy(resource, vol, sizeof(resource))) {
                printf("err: resource name %s is too long.\n", vol);
		return 0;
	} else 
                vol = strdup("lbd_name");

	if (!_lock_vol(cmd, resource, flags, lbd))
		return 0;

        return 1;
}

static int _no_lock_resource(struct cmd_context *cmd, const char *resource,
			     uint32_t flags, const struct logical_block_device *lbd)
{
        if (1 == flags)
	        return lbd_deactivate(cmd, resource, lbd);
        else
	        return lbd_activate_with_filter(cmd, resource, 1, 1, 0, lbd);
}

static void init_no_locking(struct locking_type *locking, struct cmd_context *cmd __attribute__((unused)),
		    int suppress_messages)
{
	locking->lock_resource = _no_lock_resource;
	locking->flags = 0;
}

int init_locking(int type, struct cmd_context *cmd, int suppress_messages)
{
	switch (type) {
	case 0:
		init_no_locking(&_locking, cmd, suppress_messages);
		return 1;
	default:
		printf("err: unknown locking type requested.\n");
		return 0;
	}
        return 1;
}
