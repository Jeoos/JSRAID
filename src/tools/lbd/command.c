/*
 * command.c
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
#include <ctype.h>
#include "command.h"
#include "command-count.h"

#define PERMITTED_READ_ONLY     0x00000002
#define NO_METADATA_PROCESSING  0x00000040
#define ENABLE_ALL_DEVS 0x00000008

#define MAX_LINE 1024
#define MAX_LINE_ARGC 256

#include "command-lines-input.h"

static inline int psize_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }
static inline int size_mb_arg(struct cmd_context *cmd, struct arg_values *av) { return 0; }

/* create foo_VAL enums for option and position values */
enum {
#define val(a, b, c, d) a ,
#include "vals.h"
#undef val
};

/* create foo_ARG enums for --option's */
enum {
#define arg(a, b, c, d, e, f, g) a ,
#include "args.h"
#undef arg
};

enum {
#define cmd(a, b) a ,
#include "cmds.h"
#undef cmd
};

struct cmd_name {
	const char *enum_name; /* "foo_CMD" */
	int cmd_enum;          /* foo_CMD */
	const char *name;      /* "foo" from string after ID: */
};

/* create table of value names, e.g. String, and corresponding enum from vals.h */
struct val_name val_names[VAL_COUNT + 1] = {
#define val(a, b, c, d) { # a, a, b, c, d },
#include "vals.h"
#undef val
};

/* create table of option names, e.g. --foo, and corresponding enum from args.h */
struct opt_name opt_names[ARG_COUNT + 1] = {
#define arg(a, b, c, d, e, f, g) { # a, a, b, "", "--" c, d, e, f, g },
#include "args.h"
#undef arg
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
struct command commands[COMMAND_COUNT];

static int _copy_line(char *line, int max_line, int *position)
{
	int p = *position;
	int i = 0;

	memset(line, 0, max_line);

	while (1) {
		line[i] = _command_input[p];
		i++;
		p++;

		if (_command_input[p] == '\n') {
			p++;
			break;
		}

		if (i == (max_line - 1))
			break;
	}
	*position = p;
	return 1;
}

static char *_split_line(char *buf, int *argc, char **argv, char sep)
{
	char *p = buf, *rp = NULL;
	int i;

	argv[0] = p;

	for (i = 1; i < MAX_LINE_ARGC; i++) {
		p = strchr(buf, sep);
		if (!p)
			break;
		*p = '\0';

		argv[i] = p + 1;
		buf = p + 1;
	}
	*argc = i;

	/* we ended by hitting \0, return the point following that */
	if (!rp)
		rp = strchr(buf, '\0') + 1;

	return rp;
}

static struct command_name *_find_command_name(const char *name)
{
	int i;

	if (!islower(name[0]))
		return NULL; /* Commands starts with lower-case */

	for (i = 0; i < MAX_COMMAND_NAMES; i++) {
		if (!command_names[i].name)
			break;
		if (!strcmp(command_names[i].name, name))
			return &command_names[i];
	}

	return NULL;
}

static const char *_is_command_name(char *str)
{
	const struct command_name *c;

	if ((c = _find_command_name(str)))
		return c->name;

	return NULL;
}

static int _is_id_line(char *str)
{
	if (!strncmp(str, "ID:", 3))
		return 1;
	return 0;
}

int define_commands(struct cmd_context *cmdtool, const char *run_name)
{
	char *n;
	char line[MAX_LINE];
	char line_orig[MAX_LINE];
	char *line_argv[MAX_LINE_ARGC];
	const char *name;
	int copy_pos = 0;
	int line_argc;
	int cmd_count = 0;
	struct command *cmd = NULL;

	if (run_name && !strcmp(run_name, "help"))
		run_name = NULL;

	/* Process each line of command-lines-input.h (from command-lines.in) */
	while (_copy_line(line, MAX_LINE, &copy_pos)) {
                if(line[0] == '\n'){
                        break;
                }

		if ((n = strchr(line, '\n')))
			*n = '\0';

		memcpy(line_orig, line, sizeof(line));
		_split_line(line, &line_argc, line_argv, ' ');

		if (!line_argc)
			continue;
		if ((name = _is_command_name(line_argv[0]))) {
			if (cmd_count >= COMMAND_COUNT) {
				return 0;
			}

			cmd = &commands[cmd_count];
			cmd->command_index = cmd_count;
			cmd_count++;
			cmd->name = strdup(name);

			if (!cmd->name) {
				/* FIXME */
				return 0;
			}
			continue;
                }

                if (_is_id_line(line_argv[0]) && cmd) {
                        cmd->command_id = strdup(line_argv[1]);
                        if (!cmd->command_id) {
                                /* FIXME */
                                return 0;
                        }
                        continue;
                }
        }

        return 1;
}

/* "foo" string to foo_CMD int */
int command_id_to_enum(const char *str)
{
	int i;

	for (i = 1; i < CMD_COUNT; i++) {
                if (!strcmp(str, cmd_names[i].name)){
			return cmd_names[i].cmd_enum;
                }
	}

	return CMD_NONE;
}

void configure_command_option_values(const char *name)
{
        if (!strcmp(name, "lbdcreate")) {
		opt_names[size_ARG].val_enum = psizemb_VAL;
		return;
        }
}
