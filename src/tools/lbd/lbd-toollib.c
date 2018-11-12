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

struct dvcreate_device {
	struct jd_list list;
	const char *name;
	struct device *dev;
};

struct processing_handle *init_processing_handle(struct cmd_context *cmd, 
                                        struct processing_handle *parent_handle)
{
	struct processing_handle *handle;
        
	if (!(handle = malloc(sizeof(struct processing_handle)))) {
		printf("_init_processing_handle: failed to allocate memory for \
                                processing handle");
		return NULL;
	}

	handle->parent = parent_handle;

	return handle;
}

int process_each_dv(struct cmd_context *cmd,
		    int argc, char **argv, const char *only_this_vgname,
		    int all_is_set, uint32_t read_flags,
		    struct processing_handle *handle,
		    process_single_dv_fn_t process_single_dv)
{
        return 1;
}

int process_each_pl(struct cmd_context *cmd,
		    int argc, char **argv, const char *only_this_vgname,
		    int all_is_set, uint32_t read_flags,
		    struct processing_handle *handle,
		    process_single_pl_fn_t process_single_pl)
{
        return 1;
}

static int _dvremove_check_single(struct cmd_context *cmd,
				  struct disk_volume *dv,
				  struct processing_handle *handle)
{
        return 1;
}

static int _plremove_check_single(struct cmd_context *cmd,
				  struct processing_handle *handle)
{
        return 1;
}

static int _dvcreate_check_single(struct cmd_context *cmd,
				  struct disk_volume *dv,
				  struct processing_handle *handle)
{
        return 1;
}

static int _plcreate_check_single(struct cmd_context *cmd,
				  struct processing_handle *handle)
{
        return 1;
}

int dvcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct dvcreate_params *pp)
{
	struct dvcreate_device *dd, *dd2;
	struct disk_volume *dv;
	struct dv_list *dvl;
	struct device_list *devl;
	const char *dv_name;
	unsigned i;

	handle->custom_handle = pp;

	for (i = 0; i < pp->dv_count; i++) {
		dv_name = pp->dv_names[i];

		if (!(dd = malloc(sizeof(struct dvcreate_device)))) {
			printf("malloc failed.\n");
			return 0;
		}

		if (!(dd->name = strdup(dv_name))) {
			printf("strdup failed.\n");
			return 0;
		}

		jd_list_add(&pp->arg_devices, &dd->list);
	}

        /* lock */

        /* process each disk volume */
	process_each_dv(cmd, 0, NULL, NULL, 1, 0, handle,
			pp->is_remove ? _dvremove_check_single : _dvcreate_check_single);

        /* ulock */

	jd_list_iterate_items_safe(dd, dd2, &pp->arg_create) {
                /* disk volume initialise */
		if (!(dv = dv_create(cmd, dd->dev, &pp->dva))) {
			printf("Failed to setup physical volume \"%s\".\n", dv_name);
			jd_list_move(&pp->arg_fail, &dd->list);
			continue;
		}

                /* write the metadata */
		if (!dv_write(cmd, dv)) {
			printf("Failed to write physical volume \"%s\".\n", dv_name);
			jd_list_move(&pp->arg_fail, &dd->list);
			continue;
		}
        }
        return 1;
}

int plcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct plcreate_params *pp)
{
        /* lock */

        /* process each pool */
	process_each_pl(cmd, 0, NULL, NULL, 1, 0, handle,
			pp->is_remove ? _plremove_check_single : _plcreate_check_single);

        /* ulock */
        return 1;
}
