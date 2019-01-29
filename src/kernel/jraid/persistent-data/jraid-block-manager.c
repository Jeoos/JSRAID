/* 
 * jraid-block-manager.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "jraid-block-manager.h"

#include <linux/crc32c.h>

unsigned jd_bm_block_size(struct jd_block_manager *bm)
{
	return 0; 
}

void *jd_block_data(struct jd_block *b)
{
        return NULL;
}

jd_block_t jd_block_location(struct jd_block *b)
{
	return 0;
}

int jd_bm_write_lock_zero(struct jd_block_manager *bm,
			  jd_block_t b, struct jd_block_validator *v,
			  struct jd_block **result)
{
        return 0;
}

u32 jd_bm_checksum(const void *data, size_t len, u32 init_xor)
{
	return crc32c(~(u32) 0, data, len) ^ init_xor;
}
