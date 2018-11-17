/*
 * lbd-toollib.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LBDTOOLLIB_H__
#define __LBDTOOLLIB_H__

#include "metadata-out.h"

struct cmd_context;

struct processing_handle {
        struct processing_handle *parent;
	void *custom_handle;
};

struct processing_handle *init_processing_handle(struct cmd_context *cmd, struct processing_handle *parent_handle);

int dvcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct dvcreate_params *dp);

typedef int (*process_single_dv_fn_t) (struct cmd_context *cmd,
				  struct disk_volume *dv,
				  struct processing_handle *handle);

int process_each_dv(struct cmd_context *cmd, int argc, char **argv, 
                    int all_is_set, uint32_t read_flags,
		    struct processing_handle *handle,
		    process_single_dv_fn_t process_single_dv);

#endif
