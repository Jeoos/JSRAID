/* 
 * jraid-btree.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_BTREE_H__
#define __JRAID_BTREE_H__

#include <linux/types.h>

struct jd_transaction_manager;

/*
 * Every btree node begins with this structure.  make sure it's a multiple
 * of 8-bytes in size, otherwise the 64bit keys will be mis-aligned.
 */
struct node_header {
	__le32 csum;
	__le32 flags;
	__le64 blocknr;

	__le32 nr_entries;
	__le32 max_entries;
	__le32 value_size;
	__le32 padding;
} __packed;

struct btree_node {
	struct node_header header;
	__le64 keys[0];
} __packed;

struct jd_btree_value_type {

};

struct jd_btree_info {
	struct jd_transaction_manager *tm;

	/*
	 * Number of nested btrees. (not the depth of a single tree.)
	 */
	unsigned levels;
	struct jd_btree_value_type value_type;
};

#endif
