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

#include <stdbool.h>
#include <malloc.h>

struct bcache *scan_bcache;

static int _setup_bcache(int cache_blocks);

bool dev_write_bytes(struct device *dev, uint64_t start, size_t len, void *data)
{
        printf("dev->bcache_fd = %d\n", dev->bcache_fd);
	if (!bcache_write_bytes(scan_bcache, dev->bcache_fd, start, len, data)) {
                printf("err: bcache write bytes, return false.\n");
                return false;
        }
        printf("return true.\n");
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
	//struct label_header *lh = (struct label_header *) buf;

	if (!(label->labeller->ops->write)(label, buf))
		return 0;

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
