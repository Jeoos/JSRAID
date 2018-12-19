/*
 * activate.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __ACTIVATE_H__
#define __ACTIVATE_H__

#include "metadata-out.h"

struct lbd_activate_opts {
	int exclusive;
	unsigned noscan;

	unsigned temporary;

	const struct logical_block_device *component_lbd;
};

int lbd_activate_with_filter(struct cmd_context *cmd, const char *resource, int exclusive,
			    int noscan, int temporary, const struct logical_block_device *lbd);

#endif
