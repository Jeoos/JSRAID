/*
 * lbd_manip.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "metadata-out.h"
#include "format-text.h"
#include "locking.h"

#include <string.h>

#define NAME_LEN 128

char *generate_lbd_name(struct lbd_pool *lp, const char *format,
		       char *buffer, size_t len)
{
	struct lbd_list *lbdl;
	int high = -1, i;

	jd_list_iterate_items(lbdl, &lp->lbds) {
		if (sscanf(lbdl->lbd->name, format, &i) != 1)
			continue;

		if (i > high)
			high = i;
	}

	if (snprintf(buffer, len, format, high + 1) < 0)
		return NULL;

	return buffer;
}

struct logical_block_device *alloc_lbd(void)
{
	struct logical_block_device *lbd;

	if (!(lbd = malloc(sizeof(*lbd)))) {
                printf("err: unable to allocate logical block device structure.\n");
		return NULL;
	}

	jd_list_init(&lbd->segments);
	jd_list_init(&lbd->tags);

	return lbd;
}

struct logical_block_device *lbd_create_empty(const char *name,
				       uint64_t status,
				       struct lbd_pool *lp)
{
	struct format_instance *fi = lp->fid;
        struct logical_block_device *lbd;
	char dname[NAME_LEN];

	if (strstr(name, "%d") &&
	    !(name = generate_lbd_name(lp, name, dname, sizeof(dname)))) {
                printf("err: failed to generate unique name for the new "
			  "logical block device.\n");
		return NULL;
	}

        printf("info: creating logical block device %s\n", name);
	if (!(lbd = alloc_lbd()))
		return NULL;

	if (!(lbd->name = strdup(name)))
		goto bad;

	lbd->status = status;
	lbd->major = -1;
	lbd->minor = -1;
	lbd->size = UINT64_C(0);
	lbd->le_count = 0;

	if (!link_lbd_to_lp(lp, lbd))
		goto bad;

	if (fi->fmt->ops->lbd_setup && !fi->fmt->ops->lbd_setup(fi, lbd))
		goto bad;

        return lbd;
bad:
        return NULL;
}

static struct logical_block_device *_lbd_create_an_lbd(struct lbd_pool *lp,
					       struct lbdcreate_params *lbd_p,
					       const char *new_lbd_name)
{
        struct logical_block_device *lbd;
	struct cmd_context *cmd = lp->cmd;
	uint64_t status = 0;

	if (!(lbd = lbd_create_empty(new_lbd_name ? : "lbdol%d", status, lp)))
		return NULL;

        if (is_lockd_type(lbd->lp->lock_type)) {
	        if (is_change_activating(lbd_p->activate)) {
	                if (!lbd_active_change(cmd, lbd, CHANGE_AEY, 0)) {
	                        printf("err:aborting. failed to activate LBD %s.\n",
	                                lbd->name);
	                        goto err_out;
			}
                }
        } else if (is_change_activating(lbd_p->activate) && !activate_lbd(cmd, lbd)) {
			printf("err: aborting. failed to activate LV %s locally exclusively.\n",
                                        lbd->name);
			goto err_out;
	}
        return lbd;

err_out:
        return NULL;
}

static int _lbd_remove_an_lbd(struct lbd_pool *lp,
					       struct lbdcreate_params *lbd_p,
					       const char *new_lbd_name)
{
        struct logical_block_device *lbd = NULL;
	struct cmd_context *cmd = lp->cmd;

        /* find lbd */

        if (lbd && is_lockd_type(lbd->lp->lock_type)) {
	        if (is_change_activating(lbd_p->activate)) {
	                if (!lbd_active_change(cmd, lbd, CHANGE_AN, 0)) {
	                        printf("err:aborting. failed to deactivate LBD %s.\n",
	                                lbd->name);
	                        goto err_out;
			}
                }
        } else if (is_change_activating(lbd_p->activate) && !deactivate_lbd(cmd, lbd)) {
			printf("err: aborting. failed to activate LV %s locally exclusively.\n",
                                        lbd->name);
			goto err_out;
	}
        return 1;

err_out:
        return 0;
}

struct logical_block_device *lbd_create_single(struct lbd_pool *lp,
					struct lbdcreate_params *lbd_p)
{
        struct logical_block_device *lbd;

	if (lbd_p->create_pool) {
                /*FIXME: create pool first if necessary */
        }

	if (!(lbd = _lbd_create_an_lbd(lp, lbd_p, lbd_p->lbd_name)))
		return NULL;

        return lbd;
}

int lbd_remove_single(struct lbd_pool *lp,
					struct lbdcreate_params *lbd_p)
{
	if (lbd_p->create_pool) {
                /*FIXME: create pool first if necessary */
        }

	if (!(_lbd_remove_an_lbd(lp, lbd_p, lbd_p->lbd_name)))
		return 0;

        return 1;
}
