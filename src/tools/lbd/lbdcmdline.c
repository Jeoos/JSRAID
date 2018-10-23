/*
 * lbdcmdline.c for the kernel software
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
#include "../../include/tools.h"
#include "../../include/toolcontext.h"
#include "errors.h"
#include "lbdcmdline.h"
#include "command.h"

struct command commands[COMMAND_COUNT];

static struct cmdline_context _cmdline;

extern struct command_name command_names[MAX_COMMAND_NAMES];

static const struct command_function _command_functions[CMD_COUNT] = {

};


static const char *last_path_component(char const *name)
{
	char const *slash = strrchr(name, '/');

	return (slash) ? slash + 1 : name;
}

int lbd_return_code(int ret)
{
        /* FIXME */

	return (ret == ECMD_PROCESSED ? 0 : ret);
}

int version(struct cmd_context *cmd __attribute__((unused)),
	    int argc __attribute__((unused)),
	    char **argv __attribute__((unused)))
{
        /* FIXME */

	return ECMD_PROCESSED;
}

struct cmd_context *init_lbd(unsigned set_connections, unsigned set_filters)
{
	struct cmd_context *cmd;
        
        /* create tool context */
        cmd = create_toolcontext(set_connections, set_filters);

        return cmd;
}

int lbd_run_command(struct cmd_context *cmd, int argc, char **argv)
{
        return 0;
}

static const struct command_function *_find_command_id_function(int command_enum)
{
	int i;

	if (!command_enum)
		return NULL;

	for (i = 0; i < CMD_COUNT; i++) {
		if (_command_functions[i].command_enum == command_enum)
			return &_command_functions[i];
	}

	return NULL;
}

int lbd_register_commands(struct cmd_context *cmd, const char *run_name)
{
	int i;

	/* already initialized */
	if (_cmdline.commands)
		return 1;

	memset(&commands, 0, sizeof(commands));

        if(!define_commands(cmd, run_name)) {
                return 0;
        }

	_cmdline.commands = commands;
	_cmdline.num_commands = COMMAND_COUNT;

	for (i = 0; i < COMMAND_COUNT; i++) {
		commands[i].command_enum = command_id_to_enum(commands[i].command_id);
		if (!commands[i].command_enum) {
			_cmdline.commands = NULL;
			_cmdline.num_commands = 0;
			return 0;
		}

		commands[i].functions = _find_command_id_function(commands[i].command_enum);
        }

	_cmdline.command_names = command_names;
	_cmdline.num_command_names = 0;


        return 1;
}

int lbd_main(int argc, char **argv)
{
        int ret;
        const char *base;
	struct cmd_context *cmd;
        const char *run_name;
        const char *run_command_name = NULL;
        int run_shell = 0;

        if(!argv)
                return EINIT_FAILED;
        
        /* find last path component */
	base = last_path_component(argv[0]);
        printf("base cmd = %s\n", base);

        /* analyze misc options */
	if (argc > 1) {
		/* "version" command is simple enough so it doesn't need any complex init */
		if (!strcmp(argv[1], "version"))
			return lbd_return_code(version(NULL, argc, argv));

		/* turn 'lbd -h', 'lbd --help', 'lbd -?' into 'lbd help' */
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-?"))
			argv[1] = (char *)"help";

		if (*argv[1] == '-') {
			//log_error("Specify options after a command: lbd [command] [options].");
			return EINVALID_CMD_LINE;
		}
	}
        
        /* filling in cmd context */
	if (!(cmd = init_lbd(0, 0)))
		return EINIT_FAILED;
        else
                run_command_name = run_name;

        /* 
         * Register command
         * and if READLINE_SUPPORT, do lbd shell and exit for further execution
         */
	argc--;
	argv++;
	run_name = argv[0];
        if(!run_name)
                run_shell = 1;

	if (!lbd_register_commands(cmd, run_command_name)) {
		ret = ECMD_FAILED;
		goto out;
	}

        if(run_shell) {
                ret = lbd_shell(cmd, &_cmdline);
                goto out;
        }

        /* do the mainly command */
        ret = lbd_run_command(cmd, argc, argv);

        if (ret == ENO_SUCH_CMD){
                printf("No such command. Try 'lbd help'.");
                goto out;
        }

out:
        return lbd_return_code(ret);
}
