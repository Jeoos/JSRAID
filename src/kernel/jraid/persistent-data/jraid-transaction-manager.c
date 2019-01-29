/* 
 * jraid-transaction-manager.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/types.h>
#include <linux/hash.h>
#include <linux/slab.h>

#include "../jraid-bufio.h"
#include "jraid-block-manager.h"
#include "jraid-space-map.h"

struct jd_block_manager {
	struct jd_bufio_client *bufio;
	bool read_only:1;
};

#define JD_HASH_SIZE 256
#define JD_HASH_MASK (JD_HASH_SIZE - 1)

struct shadow_info {
	struct hlist_node hlist;
	jd_block_t where;
};

struct jd_transaction_manager {
	struct jd_block_manager *bm;
	struct jd_space_map *sm;

	spinlock_t lock;
	struct hlist_head buckets[JD_HASH_SIZE];
};

struct jd_block_manager *jd_tm_get_bm(struct jd_transaction_manager *tm)
{
	return tm->bm;
}

static inline unsigned jd_hash_block(jd_block_t b, unsigned hash_mask)
{
	const unsigned BIG_PRIME = 4294967291UL;

	return (((unsigned) b) * BIG_PRIME) & hash_mask;
}

static void insert_shadow(struct jd_transaction_manager *tm, jd_block_t b)
{
	unsigned bucket;
	struct shadow_info *si;

	si = kmalloc(sizeof(*si), GFP_NOIO);
	if (si) {
		si->where = b;
		bucket = jd_hash_block(b, JD_HASH_MASK);
		spin_lock(&tm->lock);
		hlist_add_head(&si->hlist, tm->buckets + bucket);
		spin_unlock(&tm->lock);
	}
}


int jd_tm_new_block(struct jd_transaction_manager *tm,
		    struct jd_block_validator *v,
		    struct jd_block **result)
{
	int r;
	jd_block_t new_block;

	r = jd_sm_new_block(tm->sm, &new_block);
	if (r < 0)
		return r;

	r = jd_bm_write_lock_zero(tm->bm, new_block, v, result);
	if (r < 0) {
		jd_sm_dec_block(tm->sm, new_block);
		return r;
	}

	insert_shadow(tm, new_block);

        return r;
}

void jd_tm_inc(struct jd_transaction_manager *tm, jd_block_t b)
{
        jd_sm_inc_block(tm->sm, b);
}

void jd_tm_dec(struct jd_transaction_manager *tm, jd_block_t b)
{
	jd_sm_dec_block(tm->sm, b);
}

int jd_tm_shadow_block(struct jd_transaction_manager *tm, jd_block_t orig,
		       struct jd_block_validator *v, struct jd_block **result,
		       int *inc_children)
{
        return 0;
}

void jd_tm_unlock(struct jd_transaction_manager *tm, struct jd_block *b)
{
}
