/* 
 * jraid-space-map-disk.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "jraid-space-map.h"

static int sm_disk_new_block(struct jd_space_map *sm, jd_block_t *b)
{
        return 0;
}

static struct jd_space_map ops  = {
        .new_block = sm_disk_new_block
};
