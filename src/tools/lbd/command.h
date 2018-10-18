/*
 * command.h for the kernel software
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

struct cmd_context;

#define COMMAND_COUNT 161
#define CMD_COUNT 130

struct command_name {

};

struct command {

};

struct opt_name {

};

int define_commands(struct cmd_context *cmdtool, const char *run_name);
int command_id_to_enum(const char *str);

#endif
