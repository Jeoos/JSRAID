/*
 * lbdcmdline.h for the kernel software
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LBDCMDLINE_H__
#define __LBDCMDLINE_H__

struct cmd_context;

struct cmdline_context {
	struct opt_name *opt_names;
	struct command *commands;
	int num_commands;
	struct command_name *command_names;
	int num_command_names;
};

int lbd_main(int agrc, char **argv);

int lbd_shell(struct cmd_context *cmd, struct cmdline_context *cmdline);

#endif
