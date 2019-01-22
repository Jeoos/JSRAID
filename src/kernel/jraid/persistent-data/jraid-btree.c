/* 
 * jraid-btree.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>

#include "jraid-btree.h"
#include "jraid-block-manager.h"
#include "jraid-transaction-manager.h"

uint32_t calc_max_entries(size_t value_size, size_t block_size)
{
	uint32_t total, n;
	size_t elt_size = sizeof(uint64_t) + value_size; /* key + value */

	block_size -= sizeof(struct node_header);
	total = block_size / elt_size;
	n = total / 3;		/* rounds down */

	return 3 * n;
}

static void node_prepare_for_write(struct jd_block_validator *v,
				   struct jd_block *b,
				   size_t block_size)
{

}

static int node_check(struct jd_block_validator *v,
		      struct jd_block *b,
		      size_t block_size)
{
        return 0;
}

struct jd_block_validator btree_node_validator = {
	.name = "btree_node",
	.prepare_for_write = node_prepare_for_write,
	.check = node_check
};

int new_block(struct jd_btree_info *info, struct jd_block **result)
{
	return jd_tm_new_block(info->tm, &btree_node_validator, result);
}

void unlock_block(struct jd_btree_info *info, struct jd_block *b)
{

}

int jd_btree_empty(struct jd_btree_info *info, jd_block_t *root)
{
	int r;
	struct jd_block *b;
	struct btree_node *n;
	size_t block_size;
	uint32_t max_entries;

	r = new_block(info, &b);
	if (r < 0)
		return r;

	block_size = jd_bm_block_size(jd_tm_get_bm(info->tm));
	max_entries = calc_max_entries(0, block_size);

	n = jd_block_data(b);
	memset(n, 0, block_size);
	n->header.flags = cpu_to_le32(0);
	n->header.nr_entries = cpu_to_le32(0);
	n->header.max_entries = cpu_to_le32(max_entries);
	n->header.value_size = cpu_to_le32(0);

	*root = jd_block_location(b);
	unlock_block(info, b);

	return 0;
}
