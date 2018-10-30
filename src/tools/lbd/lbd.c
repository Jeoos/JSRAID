/*
 * lbd.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include "../../include/tools.h"
#include "lbdcmdline.h"

int main(int argc, char **argv)
{
        printf("In lbd, blah blah ...\n");
        //do_method();
        return lbd_main(argc, argv);
}

int lbd_shell(struct cmd_context *cmd, struct cmdline_context *cmdline)
{
        return 0;
}
