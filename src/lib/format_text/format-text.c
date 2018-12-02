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
#include "label.h"
#include <malloc.h>
#include <string.h>

static int _text_dv_initialise(const struct format_type *fmt,
			       struct dv_create_args *dva,
			       struct disk_volume *dv)
{
        return 1;
}

static int _text_dv_write(const struct format_type *fmt, struct disk_volume *dv)
{
	struct label *label = NULL;
	struct lbdcache_info *info;
	struct lbdcache_lpinfo *lpinfo = fmt->orphan_lp->lpinfo;

        info = lbdcache_info_from_lpinfo(dv->dev, lpinfo);
        if (!info)
                return 0;

	label = lbdcache_get_label(info);
        /* label write */
	if (!label_write(dv->dev, label)) {
                printf("err: label write.\n");
		return 0;
	}
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

	if (!(fmt->orphan_lp = alloc_lp("text_orphan", cmd, fmt->orphan_lp_name))){
		printf("couldn't create orphan lbd pool.\n");
                goto err_out;
        }

	if (!(fmt->labeller = text_labeller_create(fmt))) {
		printf("couldn't create text label handler.\n");
		goto err_out;
	}

        return fmt;

err_out:
        _text_destroy(fmt);
	return NULL;
}
