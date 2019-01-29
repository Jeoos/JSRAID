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

#include "jraid-block-manager.h"

#include <linux/types.h>
#include <linux/kernel.h>

#ifdef __CHECKER__
#  define __jd_written_to_disk(x) __releases(x)
#  define __jd_reads_from_disk(x) __acquires(x)
#  define __jd_bless_for_disk(x) __acquire(x)
#  define __jd_unblessfor_disk(x) __release(x)
#else
#  define __jd_written_to_disk(x)
#  define __jd_reads_from_disk(x)
#  define __jd_bless_for_disk(x)
#  define __jd_unbless_for_disk(x)
#endif

struct jd_transaction_manager;
extern struct jd_block_validator btree_node_validator;

enum node_flags {
	INTERNAL_NODE = 1,
	LEAF_NODE = 1 << 1
};

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
	void *context;

	uint32_t size;

	void (*inc)(void *context, const void *value);
	void (*dec)(void *context, const void *value);
	int (*equal)(void *context, const void *value1, const void *value2);
};

struct shadow_spine {
	struct jd_btree_info *info;

	int count;
	struct jd_block *nodes[2];

	jd_block_t root;
};

struct jd_btree_info {
	struct jd_transaction_manager *tm;

	/*
	 * Number of nested btrees. (not the depth of a single tree.)
	 */
	unsigned levels;
	struct jd_btree_value_type value_type;
};

struct ro_spine {
	struct jd_btree_info *info;

	int count;
	struct jd_block *nodes[2];
};

static inline void *value_base(struct btree_node *n)
{
	return &n->keys[le32_to_cpu(n->header.max_entries)];
}

static inline void *value_ptr(struct btree_node *n, uint32_t index)
{
	uint32_t value_size = le32_to_cpu(n->header.value_size);
	return value_base(n) + (value_size * index);
}

/*
 * Assumes the values are suitably-aligned and converts to core format.
 */
static inline uint64_t value64(struct btree_node *n, uint32_t index)
{
	__le64 *values_le = value_base(n);

	return le64_to_cpu(values_le[index]);
}

static inline __le64 *key_ptr(struct btree_node *n, uint32_t index)
{
	return n->keys + index;
}

int jd_btree_empty(struct jd_btree_info *info, jd_block_t *root);
int jd_btree_lookup(struct jd_btree_info *info, jd_block_t root,
		    uint64_t *keys, void *value_le);
int jd_btree_insert(struct jd_btree_info *info, jd_block_t root,
		    uint64_t *keys, void *value, jd_block_t *new_root)
		    __jd_written_to_disk(value);
int jd_btree_remove(struct jd_btree_info *info, jd_block_t root,
		    uint64_t *keys, jd_block_t *new_root);

void inc_children(struct jd_transaction_manager *tm, struct btree_node *n,
		  struct jd_btree_value_type *vt);
struct jd_block *shadow_current(struct shadow_spine *s);
int jd_tm_read_lock(struct jd_transaction_manager *tm, jd_block_t b,
		    struct jd_block_validator *v,
		    struct jd_block **blk);
int lower_bound(struct btree_node *n, uint64_t key);
int shadow_step(struct shadow_spine *s, jd_block_t b,
		struct jd_btree_value_type *vt);
int shadow_has_parent(struct shadow_spine *s);
int shadow_root(struct shadow_spine *s);
struct jd_block *shadow_parent(struct shadow_spine *s);
int exit_shadow_spine(struct shadow_spine *s);
void init_le64_type(struct jd_transaction_manager *tm,
		    struct jd_btree_value_type *vt);
void init_shadow_spine(struct shadow_spine *s, struct jd_btree_info *info);
#endif
