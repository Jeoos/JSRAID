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
#include <malloc.h>
#include "../../include/toolcontext.h"
#include "../../include/libjraid.h"

void destroy_toolcontext(struct cmd_context *cmd)
{

}

struct cmd_context *create_toolcontext(unsigned set_connections, unsigned set_filters)
{
        struct cmd_context *cmd;

	if (!(cmd = malloc(sizeof(*cmd)))) {
		printf("Failed to allocate command context");
		return NULL;
	}

	if (!(cmd->libmem = jraid_pool_create("library", 4 * 1024))) {
		printf("Library memory pool creation failed");
		goto out;
	}

	if (!(cmd->mem = jraid_pool_create("command", 4 * 1024))) {
		printf("Command memory pool creation failed");
		goto out;
	}

        return cmd;
out:
	destroy_toolcontext(cmd);

        return NULL;
}
