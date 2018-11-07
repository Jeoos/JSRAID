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

#include "../../include/metadata-out.h"

struct cmd_context;

struct processing_handle {
        struct processing_handle *parent;
	void *custom_handle;
};

struct processing_handle *init_processing_handle(struct cmd_context *cmd, struct processing_handle *parent_handle);

int plcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct plcreate_params *pp);

#endif
