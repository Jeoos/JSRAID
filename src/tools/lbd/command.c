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
#include <string.h>
#include "command.h"

#define PERMITTED_READ_ONLY     0x00000002
#define NO_METADATA_PROCESSING  0x00000040

struct cmd_name {
	const char *enum_name; /* "foo_CMD" */
	int cmd_enum;          /* foo_CMD */
	const char *name;      /* "foo" from string after ID: */
};

/* create table of command IDs */
struct cmd_name cmd_names[CMD_COUNT + 1] = {
#define cmd(a, b) { # a, a, # b },
#include "cmds.h"
#undef cmd
};

struct command_name command_names[MAX_COMMAND_NAMES] = {
#define xx(a, b, c...) { # a, b, c },
#include "commands.h"
#undef xx
};

int define_commands(struct cmd_context *cmdtool, const char *run_name)
{
        return 1;
}

/* "foo" string to foo_CMD int */
int command_id_to_enum(const char *str)
{
	int i;

	for (i = 1; i < CMD_COUNT; i++) {
		if (!strcmp(str, cmd_names[i].name))
			return cmd_names[i].cmd_enum;
	}

	return CMD_NONE;
}
