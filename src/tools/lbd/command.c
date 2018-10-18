/*
 * command.c for the kernel software
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "command.h"

/* create table of command IDs */
/*struct cmd_name cmd_names[CMD_COUNT + 1] = {
#define cmd(a, b) { # a, a, # b },
#include "cmds.h"
#undef cmd
};
*/

int define_commands(struct cmd_context *cmdtool, const char *run_name)
{
        return 1;
}

/* "foo" string to foo_CMD int */
/*
int command_id_to_enum(const char *str)
{
	int i;

	for (i = 1; i < CMD_COUNT; i++) {
		if (!strcmp(str, cmd_names[i].name))
			return cmd_names[i].cmd_enum;
	}

	return CMD_NONE;
}
*/

