/*
 * dev_manager.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "toolcontext.h"
#include "dev_manager.h"

#include <malloc.h>

typedef enum {
	PRELOAD,
	ACTIVATE,
	DEACTIVATE,
	CLEAN
} action_t;

struct dev_manager {
	struct cmd_context *cmd;

	const char *lp_name;
};

static int _tree_action(struct dev_manager *dm, const struct logical_block_device *lbd,
			struct lbd_activate_opts *laopts, action_t action)
{
        return 1;
}

struct dev_manager *dev_manager_create(struct cmd_context *cmd,
				       const char *lp_name,
				       unsigned track_dvmove_deps)
{
	struct dev_manager *dm;

	if (!(dm = malloc(sizeof(*dm))))
		goto bad;

	dm->cmd = cmd;
	dm->lp_name = lp_name;

	return dm;

bad:
	return NULL;
}

int dev_manager_activate(struct dev_manager *dm, const struct logical_block_device *lbd,
			 struct lbd_activate_opts *laopts)
{
	if (!_tree_action(dm, lbd, laopts, ACTIVATE))
		return 0;

	return 1;
}
