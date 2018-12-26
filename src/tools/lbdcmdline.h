/*
 * lbdcmdline.h
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

int lbd_return_code(int ret);
struct cmd_context *init_lbd(unsigned set_connections, unsigned set_filters);
int lbd_shell(struct cmd_context *cmd, struct cmdline_context *cmdline);
int lbd_run_command(struct cmd_context *cmd, int argc, char **argv);
int lbd_register_commands(struct cmd_context *cmd, const char *run_name);
int check_do_remove(int argc, char **argv, int *do_remove);
#endif
