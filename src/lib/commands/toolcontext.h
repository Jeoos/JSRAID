/*
 * toolcontext.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LBD_TOOLCONTEXT_H__
#define __LBD_TOOLCONTEXT_H__

#include <limits.h>

struct cmd_context {
	/*
	 * Memory handlers.
	 */
	struct jd_pool *libmem;
	struct jd_pool *mem;

        const char *cmd_line;
	const char *name;  /* needed before cmd->command is set */
	struct command_name *cname;
	struct command *command;
	char system_dir[PATH_MAX];

	struct jd_config_tree *cft;

	char **argv;
};

struct cmd_context *create_toolcontext(unsigned set_connections, unsigned set_filters);

#endif
