/*
 * activate.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "activate.h"
#include "dev_manager.h"

static int _lbd_activate_lbd(const struct logical_block_device *lbd, struct lbd_activate_opts *laopts)
{
	int r;
	struct dev_manager *dm;

	if (!(dm = dev_manager_create(lbd->lp->cmd, lbd->lp->name, 0)))
		return 0;

	if (!(r = dev_manager_activate(dm, lbd, laopts)))
		printf("err: dev manager activate. \n");

	return r;
}

static int _lbd_activate(struct cmd_context *cmd, const char *resource,
			struct lbd_activate_opts *laopts, int filter,
	                const struct logical_block_device *lbd)
{
        int r = 0;

        /* FIXME: for filter */

	if (!(r = _lbd_activate_lbd(lbd, laopts)))
                printf("err: activate lbd.\n");
        return r;
}

int lbd_activate_with_filter(struct cmd_context *cmd, const char *resource, int exclusive,
			    int noscan, int temporary, const struct logical_block_device *lbd)
{
	struct lbd_activate_opts laopts = { .exclusive = exclusive,
					   .noscan = noscan,
					   .temporary = temporary };

	if (!_lbd_activate(cmd, resource, &laopts, 1, lbd))
		return 0;

	return 1;
}
