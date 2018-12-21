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
#include "libjd-targets.h"
#include "libjraid.h"
#include "jd-ioctl.h"

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

static struct jd_task *jd_task_init(void)
{
	struct jd_task *jdt = malloc(sizeof(*jdt));
        if (!jdt) {
                printf("err: failed to alloc jdt.\n");
                return NULL;
        }

	jdt->type = 0;
	jdt->minor = -1;
	jdt->major = -1;

	return jdt;
}

static int _resume_node(const char *name, uint32_t major, uint32_t minor,
			uint32_t read_ahead, uint32_t read_ahead_flags,
			struct jd_info *newinfo)
{
	struct jd_task *jdt;
        int r = 0;

        if(!(jdt = jd_task_init())) {
                printf("err: failed to init jdt.\n");
                return 0;
        }

	if (!(r = jd_task_run(jdt)))
		goto out;
out:
        free(jdt);
        return r;
}

static int _tree_action(struct dev_manager *dm, const struct logical_block_device *lbd,
			struct lbd_activate_opts *laopts, action_t action)
{

	switch(action) {
	case CLEAN:
                break;
	case DEACTIVATE:
                break;
	case ACTIVATE:
                _resume_node(NULL, 0, 0, 0, 0, NULL);
                break;
	case PRELOAD:
                break;
        default:
                printf("err: action: %u not supported.\n", action);
        }
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
