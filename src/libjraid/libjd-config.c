/*
 * libjd-config.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "libjraid.h"

struct jd_config_tree *jd_config_create(void)
{
	struct jd_config_tree *cft;
	struct jd_pool *mem = jraid_pool_create("config", 10 * 1024);

	if (!mem) {
		printf("Failed to allocate config pool.");
		return 0;
	}

        /* step fix: malloc the cft */

	cft->mem = mem;

	return cft;
}
