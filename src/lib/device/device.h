
/*
 * All devices in LVM will be represented by one of these.
 * pointer comparisons are valid.
 */

#include <stdint.h>
#include <sys/types.h> 

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
