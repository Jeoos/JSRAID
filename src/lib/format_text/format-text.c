/*
 * format-text.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "format-text.h"
#include "lbdcache.h"
#include "toolcontext.h"
#include <malloc.h>

static int _text_dv_initialise(const struct format_type *fmt,
			       struct dv_create_args *dva,
			       struct disk_volume *dv)
{
        return 1;
}

static int _text_dv_write(const struct format_type *fmt, struct disk_volume *dv)
{
        return 1;
}

static void _text_destroy(struct format_type *fmt)
{
        if (fmt->orphan_lp)
                free_orphan_lp(fmt->orphan_lp);
        free(fmt);
}

static struct format_handler _text_handler = {
	.dv_initialise = _text_dv_initialise,
	.dv_write = _text_dv_write,
	.destroy = _text_destroy
};


struct format_type *create_text_format(struct cmd_context *cmd)
{
	struct format_type *fmt;
	if (!(fmt = malloc(sizeof(*fmt)))) {
		printf("failed to allocate text format type structure.\n");
		return NULL;
	}
	fmt->cmd = cmd;
	fmt->ops = &_text_handler;
	fmt->name = FMT_TEXT_NAME;
	fmt->orphan_lp_name = ORPHAN_LP_NAME(FMT_TEXT_NAME);

	if (!(fmt->orphan_lp = alloc_lp("text_orphan", cmd, fmt->orphan_lp_name)))
                goto err_out;

        return fmt;

err_out:
        _text_destroy(fmt);
	return NULL;
}
