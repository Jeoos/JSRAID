/*
 * device.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>
#include <sys/types.h> 

#define DEV_REGULAR		0x00000002	/* Regular file? */

struct device {
	dev_t dev;

	int fd;
	int open_count;
	int error_count;
	int max_error_count;
	int phys_block_size;
	int block_size;
	int read_ahead;
	int bcache_fd;
	uint32_t flags;
	unsigned size_seqno;
	uint64_t size;
	uint64_t end;

	const char *poolid;
	const char *lbdid;
};

#endif
