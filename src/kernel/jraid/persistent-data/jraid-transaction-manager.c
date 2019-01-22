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
#include "../jraid-bufio.h"
#include "jraid-block-manager.h"
#include "jraid-space-map.h"

struct jd_block_manager {
	struct jd_bufio_client *bufio;
	bool read_only:1;
};

struct jd_transaction_manager {
	struct jd_block_manager *bm;
	struct jd_space_map *sm;
};

struct jd_block_manager *jd_tm_get_bm(struct jd_transaction_manager *tm)
{
	return tm->bm;
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

        return r;
}
