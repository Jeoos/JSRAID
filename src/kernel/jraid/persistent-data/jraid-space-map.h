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

typedef void (*jd_sm_threshold_fn)(void *context);

struct jd_space_map {
	void (*destroy)(struct jd_space_map *sm);

	int (*extend)(struct jd_space_map *sm, jd_block_t extra_blocks);

	int (*get_nr_blocks)(struct jd_space_map *sm, jd_block_t *count);

	int (*get_nr_free)(struct jd_space_map *sm, jd_block_t *count);

	int (*get_count)(struct jd_space_map *sm, jd_block_t b, uint32_t *result);

	int (*count_is_more_than_one)(struct jd_space_map *sm, jd_block_t b,
				      int *result);

	int (*set_count)(struct jd_space_map *sm, jd_block_t b, uint32_t count);

	int (*commit)(struct jd_space_map *sm);

	int (*new_block)(struct jd_space_map *sm, jd_block_t *b);

	int (*inc_block)(struct jd_space_map *sm, jd_block_t b);
	int (*dec_block)(struct jd_space_map *sm, jd_block_t b);

	int (*root_size)(struct jd_space_map *sm, size_t *result);
	int (*copy_root)(struct jd_space_map *sm, void *copy_to_here_le, size_t len);

	int (*register_threshold_callback)(struct jd_space_map *sm, 
                                jd_block_t threshold, 
                                jd_sm_threshold_fn fn, 
                                void *context);
};

static inline int jd_sm_new_block(struct jd_space_map *sm, jd_block_t *b)
{
	return sm->new_block(sm, b);
}

static inline int jd_sm_inc_block(struct jd_space_map *sm, jd_block_t b)
{
	return sm->inc_block(sm, b);
}

static inline int jd_sm_dec_block(struct jd_space_map *sm, jd_block_t b)
{
	return sm->dec_block(sm, b);
}
#endif
