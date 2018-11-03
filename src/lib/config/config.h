/*
 * config.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "libjraid.h"

typedef enum {
	CONFIG_UNDEFINED,	/* undefined/uninitialized config */
	CONFIG_FILE		/* one file config */
} config_source_t;

struct jd_config_tree *config_open(config_source_t source, const char *filename, int keep_open);
