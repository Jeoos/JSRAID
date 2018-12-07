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

#define FMTT_MAGIC "\040\114\126\115\062\040\170\133\065\101\045\162\060\116\052\076"
#define FMTT_VERSION 1
#define MDA_HEADER_SIZE 512

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

/* on disk */
/* ttructure size limited to one sector */
struct mda_header {
	uint32_t checksum_xl;	/* checksum of rest of mda_header */
	int8_t magic[16];	/* to aid scans for metadata */
	uint32_t version;
	uint64_t start;		/* absolute start byte of mda_header */
	uint64_t size;		/* size of metadata area */

	struct raw_locn raw_locns[0];	/* NULL-terminated list */
} __attribute__ ((packed));

struct format_type *create_text_format(struct cmd_context *cmd);
struct labeller *text_labeller_create(const struct format_type *fmt);

/* maybe needed, so export */
struct mda_header *raw_read_mda_header(const struct format_type *fmt,
				       struct device_area *dev_area, int primary_mda);

#endif
