/* 
 * jraid-space-map-metadata.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JD_SPACE_MAP_METADATA_H__
#define __JD_SPACE_MAP_METADATA_H__

#include "jraid-transaction-manager.h"

#define JD_SM_METADATA_BLOCK_SIZE (4096 >> SECTOR_SHIFT)

/*
 * The metadata device is currently limited in size.
 *
 * We have one block of index, which can hold 255 index entries.  Each
 * index entry contains allocation info about ~16k metadata blocks.
 */
#define JD_SM_METADATA_MAX_BLOCKS (255 * ((1 << 14) - 64))
#define JD_SM_METADATA_MAX_SECTORS (JD_SM_METADATA_MAX_BLOCKS * JD_SM_METADATA_BLOCK_SIZE)

/*
 * Unfortunately we have to use two-phase construction due to the cycle
 * between the tm and sm.
 */
struct jd_space_map *jd_sm_metadata_init(void);

/*
 * Create a fresh space map.
 */
int jd_sm_metadata_create(struct jd_space_map *sm,
			  struct jd_transaction_manager *tm,
			  jd_block_t nr_blocks,
			  jd_block_t superblock);

/*
 * Open from a previously-recorded root.
 */
int jd_sm_metadata_open(struct jd_space_map *sm,
			struct jd_transaction_manager *tm,
			void *root_le, size_t len);

#endif	/* JD_SPACE_MAP_METADATA_H */
