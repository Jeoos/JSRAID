/*
 * config.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

 #include "../../include/config.h"
 #include "../../include/device.h"

struct config_file {
	off_t st_size;
	char *filename;
	int exists;
	int keep_open;
	struct device *dev;
};

struct config_source {
	config_source_t type;
	struct timespec timestamp;
	union {
		struct config_file *file;
		struct config_file *profile;
	} source;
};

struct jd_config_tree *config_open(config_source_t source,
				   const char *filename,
				   int keep_open)
{
	struct jd_config_tree *cft = jd_config_create();
	struct config_source *cs;
	struct config_file *cf;

	if (!cft)
		return NULL;

        /* steps fix: cs and cf malloc */

	return cft;
}
