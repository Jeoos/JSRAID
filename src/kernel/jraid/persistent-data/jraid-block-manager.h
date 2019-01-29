/* 
 * jraid-block-manager.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_BLOCK_MANAGER_H__
#define __JRAID_BLOCK_MANAGER_H__

#include <linux/types.h>

typedef uint64_t jd_block_t;
struct jd_block;
struct jd_block_manager;

struct jd_block_validator {
	const char *name;
	void (*prepare_for_write)(struct jd_block_validator *v, struct jd_block *b, size_t block_size);

	/*
	 * Return 0 if the checksum is valid or < 0 on error.
	 */
	int (*check)(struct jd_block_validator *v, struct jd_block *b, size_t block_size);
};


unsigned jd_bm_block_size(struct jd_block_manager *bm);
void *jd_block_data(struct jd_block *b);
jd_block_t jd_block_location(struct jd_block *b);
int jd_bm_write_lock_zero(struct jd_block_manager *bm,
			  jd_block_t b, struct jd_block_validator *v,
			  struct jd_block **result);
u32 jd_bm_checksum(const void *data, size_t len, u32 init_xor);

#endif
