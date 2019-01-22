/* 
 * jraid-space-map.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_SPACE_MAP_H__
#define __JRAID_SPACE_MAP_H__

#include "jraid-block-manager.h"

struct jd_space_map {
	int (*new_block)(struct jd_space_map *sm, jd_block_t *b);
};

static inline int jd_sm_new_block(struct jd_space_map *sm, jd_block_t *b)
{
	return sm->new_block(sm, b);
}

static inline int jd_sm_dec_block(struct jd_space_map *sm, jd_block_t b)
{
	return 0;
}

#endif
