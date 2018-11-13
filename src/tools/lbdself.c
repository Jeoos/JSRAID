/*
 * lbdself.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include "common.h"

int lbdself(struct cmd_context *cmd, int argc, char **argv)
{
        printf("in lbdself argc:%d argv:%s\n", argc, argv[2]);
        return 1;
}
