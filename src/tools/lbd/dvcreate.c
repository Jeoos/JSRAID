/*
 * dvcreate.c
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

int dvcreate(struct cmd_context *cmd, int argc, char **argv)
{
        int rt;
	struct processing_handle *handle;
	struct dvcreate_params pp;
        printf("in dv argc:%d argv:%s\n", argc, argv[2]);
        if (!argc) {
                printf("A right path needed.\n");
                return 0;
        }

        /* set the default pool param vaules.*/

	pp.dv_count = argc;
	pp.dv_names = argv;

	if (!(handle = init_processing_handle(cmd, NULL))) {
		printf("Failed to initialize processing handle.");
		return ECMD_FAILED;
	}

	if (!dvcreate_each_device(cmd, handle, &pp))
		rt = ECMD_FAILED;

        return rt;
}
