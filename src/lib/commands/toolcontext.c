/*
 * toolcontext.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "toolcontext.h"
#include "libjraid.h"
#include "configure.h"
#include "config.h"

void destroy_toolcontext(struct cmd_context *cmd)
{

}

static int _load_config_file(struct cmd_context *cmd, const char *tag, int local)
{
	return 1;
}

/*
 * Find and read lbd.conf.
 */
static int _init_lbd_conf(struct cmd_context *cmd)
{
	/* no config file if LBD_SYSTEM_DIR is empty */
	if (!*cmd->system_dir) {
		if (!(cmd->cft = config_open(CONFIG_FILE, NULL, 0))) {
			printf("Failed to create config tree");
			return 0;
		}
		return 1;
	}

	if (!_load_config_file(cmd, "", 0))
		return 0;

	return 1;
}

static int _init_dev_cache(struct cmd_context *cmd)
{
	if (!dev_cache_init(cmd))
		return 0;

        /* steps fix ... */

        return 1;
}

struct cmd_context *create_toolcontext(unsigned set_connections, unsigned set_filters)
{
        struct cmd_context *cmd;

	if (!(cmd = malloc(sizeof(*cmd)))) {
		printf("Failed to allocate command context");
		return NULL;
	}

        strcpy(cmd->system_dir, DEFAULT_SYS_DIR);

	if (*cmd->system_dir && !jd_create_dir(cmd->system_dir)) {
		printf("Failed to create LBD system dir for metadata backups, config "
			  "files and internal cache.");
		printf("Set environment variable LBD_SYSTEM_DIR to alternative location "
			  "or empty string.");
                goto out;
        }

	if (!(cmd->libmem = jraid_pool_create("library", 4 * 1024))) {
		printf("Library memory pool creation failed");
		goto out;
	}

	if (!(cmd->mem = jraid_pool_create("command", 4 * 1024))) {
		printf("Command memory pool creation failed");
		goto out;
	}

	if (!_init_lbd_conf(cmd))
		goto out;

	if (!_init_dev_cache(cmd))
		goto out;

        return cmd;
out:
	destroy_toolcontext(cmd);

        return NULL;
}
