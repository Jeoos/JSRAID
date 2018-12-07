/*
 * label.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "label.h"
#include "bcache.h"
#include "jdstruct.h"


#include <stdbool.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <fcntl.h>

#ifndef __USE_GNU
#ifndef O_DIRECT
#define O_DIRECT __O_DIRECT 
#endif

#ifndef O_NOATIME
#define O_NOATIME __O_NOATIME
#endif
#endif

struct bcache *scan_bcache;

static int _setup_bcache(int cache_blocks);

bool dev_read_bytes(struct device *dev, uint64_t start, size_t len, void *data)
{
	if (!scan_bcache) {
		/* should not happen */
		printf("err: dev_read bcache not set up.\n");
		return false;
	}

	if (dev->bcache_fd <= 0) {
		/* should not happen */
		return false;
	}

	if (!bcache_read_bytes(scan_bcache, dev->bcache_fd, start, len, data)) {
		printf("err: reading device at %llu length %u.",
			(unsigned long long)start, (uint32_t)len);
		return false;
	}
	return true;

}

bool dev_write_bytes(struct device *dev, uint64_t start, size_t len, void *data)
{
        printf("dev->bcache_fd = %d\n", dev->bcache_fd);
	if (!bcache_write_bytes(scan_bcache, dev->bcache_fd, start, len, data)) {
                printf("err: bcache write bytes, return false.\n");
                return false;
        }
        return true;
}

struct label *label_create(struct labeller *labeller)
{
        int rt;
	struct label *label;

	if (!(label = malloc(sizeof(*label)))) {
		printf("label allocaction failed.\n");
		return NULL;
	}
	label->labeller = labeller;
	labeller->ops->initialise_label(labeller, label);

        /* FIXME: no good idea setup bcache here */
        rt = _setup_bcache(0);
        if (!rt) {
                free(label);
                printf("err: label and bcache create\n");
                return NULL;
        }

	return label;
}

int label_write(struct device *dev, struct label *label)
{
	char buf[LABEL_SIZE] __attribute__((aligned(8)));
	struct label_header *lh = (struct label_header *) buf;

	if (!(label->labeller->ops->write)(label, buf))
		return 0;
        printf("lh->type = %s.\n", lh->type);

	if (!dev_write_bytes(dev, label->sector << SECTOR_SHIFT, LABEL_SIZE, buf)) {
		printf("failed to write label.\n");
                return 0;
	}

        return 1;
}

#define BCACHE_BLOCK_SIZE_IN_SECTORS 256 /* 256*512 = 128K */

#define MIN_BCACHE_BLOCKS 32
#define MAX_BCACHE_BLOCKS 1024

static int _setup_bcache(int cache_blocks)
{
	struct io_engine *ioe;

	if (cache_blocks < MIN_BCACHE_BLOCKS)
		cache_blocks = MIN_BCACHE_BLOCKS;

	if (cache_blocks > MAX_BCACHE_BLOCKS)
		cache_blocks = MAX_BCACHE_BLOCKS;

	if (!(ioe = create_async_io_engine())) {
		printf("failed to create bcache io engine.\n");
		return 0;
	}

	if (!(scan_bcache = bcache_create(BCACHE_BLOCK_SIZE_IN_SECTORS, cache_blocks, ioe))) {
		printf("failed to create bcache with %d cache blocks.\n", cache_blocks);
		return 0;
	}

	return 1;
}

int label_dev_open(struct device *dev)
{
	struct jd_list *name_list;
	struct jd_str_list *name_sl;
	const char *name;
	int flags = 0;
	int fd;

	if (!dev)
		return 0;

	if (dev->bcache_fd > 0) {
                printf("dev already open with fd %d\n", dev->bcache_fd);
                return 0;
        }

	if (!(name_list = jd_list_first(&dev->aliases))) {
		printf("device open %d:%d has no path names.",
			  (int)MAJOR(dev->dev), (int)MINOR(dev->dev));

                return 0;
        }

	name_sl = jd_list_item(name_list, struct jd_str_list);
	name = name_sl->str;

        /* diff from env */
	//flags |= O_DIRECT;
	flags |= O_NOATIME;

	flags |= O_RDWR;

	fd = open(name, flags, 0777);
	if (fd < 0) {
                /* FIXME: for retry */
                printf("err: to get dev fd.\n");
                return 0;
        }
	dev->bcache_fd = fd;

        return 1;
}
