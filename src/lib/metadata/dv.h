/*
 * dv.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __DV_H__
#define __DV_H__

#include "libjraid.h"

struct device;
struct lbd_pool;
struct format_instance;

struct disk_volume {
        struct device *dev;
	const struct format_type *fmt;
	struct format_instance *fid;

	const char *lpname;
        struct lbd_pool *lp;
};

#endif
