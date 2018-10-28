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

struct cmd_context *create_toolcontext(unsigned set_connections, unsigned set_filters)
{
        struct cmd_context *cmd;

	if (!(cmd = malloc(sizeof(*cmd)))) {
		printf("Failed to allocate command context");
		return NULL;
	}

        return cmd;
}
