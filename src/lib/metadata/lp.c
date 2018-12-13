/*
 * lp.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "toolcontext.h"
#include "metadata.h"

#include <malloc.h>

struct lbd_pool *alloc_lp(const char *pool_name, struct cmd_context *cmd,
			      const char *lp_name)
{
	struct lbd_pool *lp;
        lp = malloc(sizeof(*lp));
        if (!lp) {
                printf("err malloc.\n");
                return NULL;
        }
	lp->cmd = cmd;
	jd_list_init(&lp->dvs);
	jd_list_init(&lp->lbds);

        return lp;
}

void free_orphan_lp(struct lbd_pool *lp)
{

}

int link_lbd_to_lp(struct lbd_pool *lp, struct logical_block_device *lbd)
{
	struct lbd_list *lbdl;

	if (!(lbdl = malloc(sizeof(*lbdl))))
		return 0;

	lbdl->lbd = lbd;
	lbd->lp = lp;
	jd_list_add(&lp->lbds, &lbdl->list);
	lbd->status &= ~LBD_REMOVED;

	return 1;
}
