/*
 * label.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LABEL_H__
#define __LABEL_H__

#include "device.h"
#include "metadata-out.h"

#include <stdbool.h> 

#define LABEL_SIZE SECTOR_SIZE

struct labeller;

struct label_header {
	int8_t id[8];		/* LABELONE */
	uint64_t sector_xl;	/* sector number of this label */
	uint32_t crc_xl;	/* from next field to end of sector */
	uint32_t offset_xl;	/* offset from start of struct to contents */
	int8_t type[8];		/* LBD 001 */
} __attribute__ ((packed));

struct label {
	char type[8];
	uint64_t sector;
	struct labeller *labeller;
	struct device *dev;
	void *info;
};

struct label_ops {
	/*
	 * Write a label to a volume.
	 */
	int (*write) (struct label * label, void *buf);

	/*
	 * Read a label from a volume.
	 */
	int (*read) (struct labeller * l, struct device * dev,
		     void *label_buf, struct label ** label);

	/*
	 * populate label_type etc.
	 */
	int (*initialise_label) (struct labeller * l, struct label * label);
};

struct labeller {
	struct label_ops *ops;
	const struct format_type *fmt;
};

int label_write(struct device *dev, struct label *label);
struct label *label_create(struct labeller *labeller);
bool dev_write_bytes(struct device *dev, uint64_t start, size_t len, void *data);

#endif
