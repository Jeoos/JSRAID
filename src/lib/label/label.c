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

bool dev_write_bytes(struct device *dev, uint64_t start, size_t len, void *data)
{
	if (!bcache_write_bytes(scan_bcache, dev->bcache_fd, start, len, data)) {
                printf("err: bcache write bytes, return false.\n");
                return false;
        }
        printf("return true.\n");
        return true;
}

struct label *label_create(struct labeller *labeller)
{
	struct label *label;

	if (!(label = malloc(sizeof(*label)))) {
		printf("label allocaction failed.\n");
		return NULL;
	}
	label->labeller = labeller;
	labeller->ops->initialise_label(labeller, label);

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
