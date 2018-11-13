/*
 * command.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __LBD_COMMAND_H__
#define __LBD_COMMAND_H__

#include <stdint.h>

struct cmd_context;
struct arg_values;

#define MAX_COMMAND_NAMES 64

typedef int (*command_id_fn) (struct cmd_context *cmd, int argc, char **argv);

struct command_function {
	int command_enum;
	command_id_fn fn;
};

struct command_name {
	const char *name;
	const char *desc; /* general command description from commands.h */
	unsigned int flags;
};

struct command {
        const char *name;
	const char *command_id; /* ID string in command-lines.in */
	int command_enum; /* <command_id>_CMD */
	int command_index; /* position in commands[] */

	const struct command_function *functions;
};

struct val_name {
	const char *enum_name;  /* "foo_VAL" */
	int val_enum;           /* foo_VAL */
	int (*fn) (struct cmd_context *cmd, struct arg_values *av); /* foo_arg() */
	const char *name;       /* FooVal */
	const char *usage;
};

struct opt_name {
	const char *name;       /* "foo_ARG" */
	int opt_enum;           /* foo_ARG */
	const char short_opt;   /* -f */
	char _padding[7];
	const char *long_opt;   /* --foo */
	int val_enum;           /* xyz_VAL when --foo takes a val like "--foo xyz" */
	uint32_t flags;
	uint32_t prio;
	const char *desc;
};

int define_commands(struct cmd_context *cmdtool, const char *run_name);
int command_id_to_enum(const char *str);
void configure_command_option_values(const char *name);

#endif
