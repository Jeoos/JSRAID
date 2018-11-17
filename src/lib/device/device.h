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

#define MAJOR(x) major((x))
#define MINOR(x) minor((x))

#define ID_LEN 32

#define DEV_REGULAR		0x00000002	/* Regular file? */

struct dev_iter;

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

	const char *lpid;
	const char *lbdid;
	char dvid[ID_LEN + 1]; /* if device is a DV */
	char _padding[7];
};

#endif
