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
#include "lbdcache.h"
#include "dev-cache.h"

struct device_id_list {
	struct jd_list list;
	struct device *dev;
	char lpid[ID_LEN + 1];
};

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

static int _process_dvs_in_pools(struct cmd_context *cmd, 
                                uint32_t read_flags,
			        struct jd_list *all_lpnameids,
			        struct jd_list *all_devices,
                                struct jd_list *arg_devices,
			        struct processing_handle *handle,
			        process_single_dv_fn_t process_single_pv)
{
	//struct disk_volume *dv;
        return 1;
}

static int _get_arg_dvnames(struct cmd_context *cmd,
			    int argc, char **argv,
			    struct jd_list *arg_dvnames)
{
        int opt = 0;
	char *arg_name;
	int ret_max = ECMD_PROCESSED;
	for (; opt < argc; opt++) {
	        arg_name = argv[opt];

		if (!str_list_add(arg_dvnames, strdup(arg_name))) {
			printf("strlist allocation failed.\n");
			return ECMD_FAILED;
		}

        }
        return ret_max;
}

static int _get_all_devices(struct cmd_context *cmd, struct jd_list *all_devices)
{
	struct device *dev;
	struct dev_iter *iter;
	struct device_id_list *dil;
	int r = ECMD_FAILED;
        printf("in 0...\n");

	if (!(iter = dev_iter_create(1))) {
		printf("dev_iter creation failed.");
		return ECMD_FAILED;
	}
        printf("in 1...\n");

	while ((dev = dev_iter_get(iter))) {
        printf("in 2...\n");
		if (!(dil = malloc(sizeof(*dil)))) {
			printf("device_id_list alloc failed.\n");
			goto out;
		}
                printf("in 2.1 ... dev:%p dev->lpid %s\n", dev, dev->lpid);
                printf("in 2.20...\n");

		strncpy(dil->lpid, dev->lpid, ID_LEN);
                printf("in 2.2...\n");
		dil->dev = dev;
                printf("in 2.3...\n");
		jd_list_add(all_devices, &dil->list);
	}
        printf("in 3...\n");

	r = ECMD_PROCESSED;
out:
        free(iter);
	return r;
}

int process_each_dv(struct cmd_context *cmd, int argc, char **argv,
                    int all_is_set, uint32_t read_flags,
		    struct processing_handle *handle,
		    process_single_dv_fn_t process_single_dv)
{
	int ret_max = ECMD_PROCESSED;
        int ret;
	struct jd_list arg_dvnames;	/* str_list */
	struct jd_list arg_devices;	/* device_id_list */
	struct jd_list all_lpnameids;	/* lpnameid_list */
	struct jd_list all_devices;	/* device_id_list */


	jd_list_init(&arg_dvnames);
	jd_list_init(&arg_devices);
	jd_list_init(&all_lpnameids);
	jd_list_init(&all_devices);

        /* steps FIXME: cmd value for items getting */

        /* get arg dvnames */
	if ((ret = _get_arg_dvnames(cmd, argc, argv, &arg_dvnames)) != ECMD_PROCESSED) {
		ret_max = ret;
		goto out;
	}

        /* get lpnameids */
	if (!get_lpnameids(cmd, &all_lpnameids, 1)) {
		ret_max = ret;
		goto out;
        }

        /* get all devices */
	if ((ret = _get_all_devices(cmd, &all_devices)) != ECMD_PROCESSED) {
		ret_max = ret;
		goto out;
	}

        _process_dvs_in_pools(cmd, read_flags, &all_lpnameids, &all_devices, &arg_devices,
                    handle, process_single_dv);

        return 1;
out:
        return ret_max;
}

static int _dvremove_check_single(struct cmd_context *cmd,
				  struct disk_volume *dv,
				  struct processing_handle *handle)
{
        return 1;
}

static int _dvcreate_check_single(struct cmd_context *cmd,
				  struct disk_volume *dv,
				  struct processing_handle *handle)
{
	struct dvcreate_device *dd;
	struct dvcreate_params *dp = (struct dvcreate_params *) handle->custom_handle;
	int found = 0;

        if (!dv->dev)
                return 1;

	jd_list_iterate_items(dd, &dp->arg_devices) {
		if (dd->dev != dv->dev)
			continue;
                found = 1;
                break;
        }

        if (!found)
                return 1;

        /* steps fixme: more check items */

        /* move to arg_process list */
	dd->dev = dv->dev;  /* for sure */
	jd_list_move(&dp->arg_process, &dd->list);

        return 1;
}

int dvcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct dvcreate_params *dp)
{
	struct dvcreate_device *dd, *dd2;
	struct disk_volume *dv;
	//struct dv_list *dvl;
	//struct device_list *devl;
	const char *dv_name = NULL;
	unsigned i;

	handle->custom_handle = dp;

        /* steps fixme */
        dp->dv_count--;
        dp->dv_names++;

	for (i = 0; i < dp->dv_count; i++) {
		dv_name = dp->dv_names[i];
                printf("dv_name =%s\n", dv_name);

		if (!(dd = malloc(sizeof(struct dvcreate_device)))) {
			printf("malloc failed.\n");
			return 0;
		}

		if (!(dd->name = strdup(dv_name))) {
			printf("strdup failed.\n");
			return 0;
		}
		jd_list_add(&dp->arg_devices, &dd->list);
	}

        jd_list_iterate_items(dd, &dp->arg_devices)
		dd->dev = dev_cache_get(dd->name);

        /* process each disk volume */
	process_each_dv(cmd, 0, NULL, 1, 0, handle,
			dp->is_remove ? _dvremove_check_single : _dvcreate_check_single);

	if (dp->is_remove)
		jd_list_splice(&dp->arg_remove, &dp->arg_process);
        else 
		jd_list_splice(&dp->arg_create, &dp->arg_process);

	jd_list_iterate_items_safe(dd, dd2, &dp->arg_create) {
                printf("debug 1 ...\n");
                /* disk volume initialise */
		if (!(dv = dv_create(cmd, dd->dev, &dp->dva))) {
			printf("Failed to setup physical volume \"%s\".\n", dv_name);
			jd_list_move(&dp->arg_fail, &dd->list);
			continue;
		}
                printf("debug 2 ...\n");

                /* write the metadata */
		if (!dv_write(cmd, dv)) {
			printf("Failed to write physical volume \"%s\".\n", dv_name);
			jd_list_move(&dp->arg_fail, &dd->list);
			continue;
		}
        }
        printf("debug 3 ...\n");
        return 1;
}
