/*
 * toolcontext.h for the kernel software
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

struct cmd_context {

        const char *cmd_line;
	const char *name;  /* needed before cmd->command is set */
};

struct cmd_context *create_toolcontext(unsigned set_connections, unsigned set_filters);

#endif
