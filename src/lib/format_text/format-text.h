/*
 * format-text.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __FORMAT_TEXT_H__
#define __FORMAT_TEXT_H__

#include "metadata.h"
#include "device.h"

#define FMT_TEXT_NAME "lbd"
#define LBD_LABEL "LBD  001"

struct mda_lists {
	struct jd_list dirs;
	struct jd_list raws;
	struct metadata_area_ops *file_ops;
	struct metadata_area_ops *raw_ops;
};

struct text_context {
	const char *path_live;
	const char *path_edit;
	const char *desc;
};

/* on disk */
struct raw_locn {
	uint64_t offset;	/* offset in bytes to start sector */
	uint64_t size;		/* bytes */
	uint32_t checksum;
	uint32_t flags;
} __attribute__ ((packed));

struct mda_context {
	struct device_area area;
	uint64_t free_sectors;
	struct raw_locn rlocn;	/* Store inbetween write and commit */
};

struct format_type *create_text_format(struct cmd_context *cmd);
struct labeller *text_labeller_create(const struct format_type *fmt);

#endif
