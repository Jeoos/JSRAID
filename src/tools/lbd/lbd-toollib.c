/*
 * lbd-toollib.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <malloc.h>
#include "common.h"

struct processing_handle *init_processing_handle(struct cmd_context *cmd, struct processing_handle *parent_handle)
{
	struct processing_handle *handle;
        
	if (!(handle = malloc(sizeof(struct processing_handle)))) {
		printf("_init_processing_handle: failed to allocate memory for processing handle");
		return NULL;
	}

	handle->parent = parent_handle;

	return handle;
}

int plcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct plcreate_params *pp)
{
        return 1;    
}
