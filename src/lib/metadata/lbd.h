/*
 * lbd.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LBD_H__
#define __LBD_H__

#include "libjraid.h"

struct logical_block_device {
	const char *name;

	struct lbd_pool *lp;
	uint64_t status;
	int32_t major;
	int32_t minor;

	uint64_t size;
	uint32_t le_count;

	struct jd_list segments;
	struct jd_list tags;
};

#endif
