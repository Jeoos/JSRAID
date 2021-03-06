/*
 * lbdcmdline.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include "errors.h"
#include "lbdcmdline.h"
#include "command.h"
#include "command-count.h"
#include "common.h"
#include "locking.h"

#include "lbd-version.h"

struct command commands[COMMAND_COUNT];

static struct cmdline_context _cmdline;

extern struct command_name command_names[MAX_COMMAND_NAMES];
extern struct opt_name opt_names[ARG_COUNT + 1];

static int version(struct cmd_context *cmd __attribute__((unused)),
	    int argc __attribute__((unused)),
	    char **argv __attribute__((unused)))
{
	printf("LBD version:     %s\n", LBD_VERSION);

	return ECMD_PROCESSED;
}

static const struct command_function _command_functions[CMD_COUNT] = {
	{ version_general_CMD, version },
	{ dvcreate_general_CMD, dvcreate },
	{ dvremove_general_CMD, dvremove },
	{ lbdpool_general_CMD, lbdpool },
	{ lbdself_general_CMD, lbdself },
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

struct cmd_context *init_lbd(unsigned set_connections, unsigned set_filters)
{
	struct cmd_context *cmd;
        
        /* create tool context */
        cmd = create_toolcontext(set_connections, set_filters);

	_cmdline.opt_names = &opt_names[0];

        return cmd;
}

static struct command_name *_find_command_name(const char *name)
{
	int i;
        
	for (i = 0; i < MAX_COMMAND_NAMES; i++) {
                if (!command_names[i].name)
			break;

		if (!strcmp(command_names[i].name, name))
			return &command_names[i];
	}
	return NULL;
}

static const char *_copy_command_line(struct cmd_context *cmd, int argc, char **argv)
{
	const char *rt="FIXME blah blah ...";
	/*
	 * Build up the complete command line, used as a
	 * description for backups.
	 */
	return rt;
}

static int _process_command_line(struct cmd_context *cmd, int *argc, char ***argv)
{
	return 1;
}

static struct command *_find_command(struct cmd_context *cmd, const char *path, int *argc, char **argv)
{
        const char *name;
        int i;
	int only_i = 0;
	int variants = 0;
        int best_i = 0;

        name = last_path_component(path);

	for (i = 0; i < COMMAND_COUNT; i++) {
		if (strcmp(name, commands[i].name))
			continue;
		variants++;
        }
	for (i = 0; i < COMMAND_COUNT; i++) {
		if (strcmp(name, commands[i].name))
			continue;
                if (variants == 1)
                        only_i = i;

                if (only_i)
                        best_i = i;
                 
                /* steps fix: for !only_i */
        }
	return &commands[best_i];
}

int lbd_run_command(struct cmd_context *cmd, int argc, char **argv) 
{
	char *arg_new, *arg;
	int i, rt=0;
	int skip_hyphens;

	if (!(cmd->name = strdup(last_path_component(argv[0])))) {
		printf("Failed to strdup command basename.");
		return ECMD_FAILED;
	}

	configure_command_option_values(cmd->name);

	/* eliminate '-' from all options starting with -- */
	for (i = 1; i < argc; i++) {

		arg = argv[i];

		if (*arg++ != '-' || *arg++ != '-')
			continue;

		/* if we reach "--" then stop. */
		if (!*arg)
			break;

		arg_new = arg;
		skip_hyphens = 1;
		while (*arg) {
			/* if we encounter '=', stop any further hyphen removal. */
			if (*arg == '='){
				skip_hyphens = 0;
			}

			/* do we need to keep the next character? */
			if (*arg != '-' || !skip_hyphens) {
				if (arg_new != arg){
					*arg_new = *arg;
				}
				++arg_new;
			}
			arg++;
		}

		/* terminate a shortened arg */
		if (arg_new != arg) {
			*arg_new = '\0';
		}
	}

	if (!(cmd->cmd_line = _copy_command_line(cmd, argc, argv)))
		return ECMD_FAILED;

	if (!(cmd->cname = _find_command_name(cmd->name)))
		return ENO_SUCH_CMD;

	if (!_process_command_line(cmd, &argc, &argv)) {
		printf("Error during parsing of command line.\n");
		return EINVALID_CMD_LINE;
	}

	if (!(cmd->command = _find_command(cmd, cmd->name, &argc, argv)))
		return EINVALID_CMD_LINE;

        if (!init_locking(0, cmd, 0))
		return ENO_SUCH_CMD;

	if (cmd->command->functions) {
		rt = cmd->command->functions->fn(cmd, argc, argv);
	}

        return rt;
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

int check_do_remove(int argc, char **argv, int *do_remove)
{
        int i, j;

        for (i = 0; i < argc; i++) {
                for (j = 0; j < ARG_COUNT; j++) {
                        if (opt_names[j].opt_enum == remove_ARG) {
                                if (!strcmp(argv[i], opt_names[j].long_opt)) {
                                        *do_remove = 1;
                                        break;
                                }
                        }
                }
                if (1 == *do_remove)
                        break;
        }
        
        return 1;
}

int lbd_main(int argc, char **argv)
{
        int ret;
	struct cmd_context *cmd;
        const char *run_name;
        const char *run_command_name = NULL;
        int run_shell = 0;

        if(!argv)
                return EINIT_FAILED;
        
        /* analyze misc options */
	if (argc > 1) {
		/* "version" command is simple enough so it doesn't need any complex init */
                if (!strcmp(argv[1], "version")){
			return lbd_return_code(version(NULL, argc, argv));
                }

		/* turn 'lbd -h', 'lbd --help', 'lbd -?' into 'lbd help' */
                if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-?")){
			argv[1] = (char *)"help";
                }

		if (*argv[1] == '-') {
			printf("Specify options after a command: lbd [command] [options].\n");
			return EINVALID_CMD_LINE;
		}
	}
        
        /* filling in cmd context */
        if (!(cmd = init_lbd(0, 0)))
		return EINIT_FAILED;
        cmd->argv = argv;

        /* 
         * Register command
         * and if READLINE_SUPPORT, do lbd shell and exit for further execution
         */
	argc--;
	argv++;
	run_name = argv[0];

        if(!run_name)
                run_shell = 1;
	run_command_name = run_name;
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
                printf("No such command. Try 'lbd help'.\n");
                goto out;
        }

out:
        return lbd_return_code(ret);
}
