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

static int _process_dvs_in_pool(struct cmd_context *cmd,
			      struct lbd_pool *lp,
			      struct jd_list *all_devices,
			      struct jd_list *arg_devices,
			      struct processing_handle *handle,
			      process_single_dv_fn_t process_single_dv)
{
        struct disk_volume *dv = NULL;
	struct dv_list *dvl;
	int ret_max = ECMD_PROCESSED;
	int ret = 0;

        /* get dv */
	jd_list_iterate_items(dvl, &lp->dvs) {
                dv = dvl->dv;
                printf("dv->dev:%p\n", dv->dev);
        }

	ret = process_single_dv(cmd, lp, dv, handle);
        if (ret != ECMD_PROCESSED) {
                printf("err handle .\n");
                ret_max = ret;
        }

        /* FIXME: it's unnecessary */
        if (is_orphan_lp(lbdcache_get_lpname(lp)))
                cmd->custom_ptr = lp;

        return ret_max;
}

static int _process_dvs_in_pools(struct cmd_context *cmd, 
                                uint32_t read_flags,
			        struct jd_list *all_lpnameids,
			        struct jd_list *all_devices,
                                struct jd_list *arg_devices,
			        struct processing_handle *handle,
			        process_single_dv_fn_t process_single_dv)
{
        int ret;
	int ret_max = ECMD_PROCESSED;
	struct lbd_pool *lp;
	struct lbdcache_info *info;
	struct lbdcache_lpinfo *lpinfo;
        struct device *dev = NULL;
	struct device_id_list *dil;
	struct lpnameid_list *lpnl;
	uint32_t lockd_state = 0;
	const char *lp_name;
	const char *lp_uuid = NULL;
        struct label *label = NULL;

	jd_list_iterate_items(lpnl, all_lpnameids) {
                lp_name = lpnl->lp_name;
                lp_uuid = lpnl->lpid;

                /* FIXME: should not be like this. */
                /* set dv */
	        if (is_orphan_lp(lp_name)) {
	                jd_list_iterate_items(dil, all_devices) {
                                if (dil->dev) {
                                        dev = dil->dev;
                                }
                                break;
                        }
                        info = _create_info(NULL, dev);
                        if (!info) {
                                printf("err: info get failed.\n");
                                return NULL;
                        }
                                
	                if (!(lpinfo = lbdcache_lpinfo_from_lpname(lp_name, NULL)))
		                return NULL;

                        _lpinfo_attach_info(lpinfo, info);
                        _info_attach_label(info, label);
                }

                /* get lp */
                lp = lp_read(cmd, lp_name, lp_uuid, read_flags, lockd_state);

		ret = _process_dvs_in_pool(cmd, lp, all_devices, arg_devices, 
					 handle, process_single_dv);
                if (ret != ECMD_PROCESSED) {
                        printf("err handle.\n");
                        ret_max = ret;
                }
        }
        return ret_max;
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

	if (!(iter = dev_iter_create(1))) {
		printf("dev_iter creation failed.");
		return ECMD_FAILED;
	}

	while ((dev = dev_iter_get(iter))) {
		if (!(dil = malloc(sizeof(*dil)))) {
			printf("device_id_list alloc failed.\n");
			goto out;
		}
                /* lpid not malloc yet */
		//strncpy(dil->lpid, dev->lpid, ID_LEN);
		dil->dev = dev;
		jd_list_add(all_devices, &dil->list);
	}

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

        /* FIXME: for arg_devices when needed */

        _process_dvs_in_pools(cmd, read_flags, &all_lpnameids, &all_devices, &arg_devices,
                    handle, process_single_dv);

out:
        return ret_max;
}

static int _dvremove_check_single(struct cmd_context *cmd,
				  struct lbd_pool *lp,
				  struct disk_volume *dv,
				  struct processing_handle *handle)
{
        return 1;
}

static int _dvcreate_check_single(struct cmd_context *cmd,
				  struct lbd_pool *lp,
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

        if (!found) {
                return 1;
        }

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
                /* disk volume initialise */
		if (!(dv = dv_create(cmd, dd->dev, &dp->dva))) {
			printf("failed to setup physical volume \"%s\".\n", dv_name);
			jd_list_move(&dp->arg_fail, &dd->list);
			continue;
		}

                /* write the metadata */
		if (!dv_write(cmd, dv)) {
			printf("failed to write physical volume \"%s\".\n", dv_name);
			jd_list_move(&dp->arg_fail, &dd->list);
			continue;
		}
        }
        return 1;
}
