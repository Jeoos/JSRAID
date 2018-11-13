/*
 * metadata.c
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
#include "../../include/device.h"
#include "metadata.h"

struct disk_volume *dv_create(struct cmd_context *cmd,
				  struct device *dev,
				  struct dv_create_args *dva)
{
        return NULL;
}

int dv_write(struct cmd_context *cmd, struct disk_volume *dv)
{
        return 1;
}

void dv_defaults_init(struct dvcreate_params *pp)
{
        memset(pp, 0, sizeof(struct dvcreate_params));

	jd_list_init(&pp->arg_devices);
	jd_list_init(&pp->arg_create);
	jd_list_init(&pp->arg_remove);
	jd_list_init(&pp->arg_fail);
	jd_list_init(&pp->arg_process);
}
