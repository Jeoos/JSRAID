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
#include <malloc.h>
#include "device.h"
#include "metadata.h"
#include "lbdcache.h"
#include "toolcontext.h"

#define DEFAULT_EXTENT_SIZE 4096

int is_orphan_lp(const char *lp_name)
{
	return (lp_name && !strncmp(lp_name, ORPHAN_PREFIX, sizeof(ORPHAN_PREFIX) - 1)) ? 1 : 0;
}

struct disk_volume *dv_create(struct cmd_context *cmd,
				  struct device *dev,
				  struct dv_create_args *dva)
{
	const struct format_type *fmt = cmd->fmt;
        struct dv_list *dvl;
        struct disk_volume *dv = NULL;
        struct lbd_pool *lp = (struct lbd_pool *)cmd->custom_ptr;

        jd_list_iterate_items(dvl, &lp->dvs)
                dv = dvl->dv; 

        dv->fmt = fmt;
        dv->lpname = fmt->orphan_lp_name;

	if (!fmt->ops->dv_initialise(fmt, dva, dv)) {
		printf("format-specific initialisation of physical volume \
                                device failed.");
		goto bad;
	}
        return dv;
bad:
        return NULL;
}

int dv_write(struct cmd_context *cmd, struct disk_volume *dv)
{
	if (!dv->fmt->ops->dv_write(dv->fmt, dv))
		return 0;

        return 1;
}

void dv_defaults_init(struct dvcreate_params *dp)
{
        memset(dp, 0, sizeof(struct dvcreate_params));

	jd_list_init(&dp->arg_devices);
	jd_list_init(&dp->arg_create);
	jd_list_init(&dp->arg_remove);
	jd_list_init(&dp->arg_fail);
	jd_list_init(&dp->arg_process);
	jd_list_init(&dp->arg_process);
	jd_list_init(&dp->dvs);
}

int get_lpnameids(struct cmd_context *cmd, struct jd_list *lpnameids,
                int include_internal)
{
	if(!lbdcache_get_lpnameids(cmd, include_internal, lpnameids))
                return 0;
        return 1;
}

struct _lp_read_orphan_transf {
	struct cmd_context *cmd;
	struct lbd_pool *lp;
	const struct format_type *fmt;
};

void add_dvl_to_lps(struct lbd_pool *lp, struct dv_list *dvl)
{
	jd_list_add(&lp->dvs, &dvl->list);
	dvl->dv->lp = lp;
}

static struct disk_volume *_alloc_dv(struct device *dev)
{
        struct disk_volume *dv;

	if (!(dv = malloc(sizeof(*dv)))) {
		printf("failed to allocate dv structure.\n");
		return NULL;
        }
        dv->dev = dev;

        return dv;
}

static struct disk_volume *_dv_read(struct cmd_context *cmd,
					const struct format_type *fmt,
					struct lbd_pool *lp,
					struct lbdcache_info *info)
{
        struct disk_volume *dv;

	if (!(dv = _alloc_dv(NULL))) {
		printf("dv allocation failed,\n");
		return NULL;
	}

        return dv;
}

static int _lp_read_orphan_dv(struct lbdcache_info *info, void *transf)
{
        struct disk_volume *dv;
	struct dv_list *dvl;
        struct _lp_read_orphan_transf *t = transf;
	if (!(dv = _dv_read(t->cmd, t->fmt, t->lp, info))) {
                printf("err: dv no found.\n");
		return 1;
	}

	if (!(dvl = malloc(sizeof(*dvl)))) {
                printf("err: dv_list allocation failed.\n");
		return 0;
	}
        lbdcache_set_dv(dv, info);
	dvl->dv = dv;
	add_dvl_to_lps(t->lp, dvl);

        return 1;
}

static struct lbd_pool *_lp_read_orphans(struct cmd_context *cmd,
					     uint32_t warn_flags,
					     const char *orphan_lpname,
					     int *consistent)
{
	const struct format_type *fmt;
        struct lbdcache_lpinfo *lpinfo = NULL;
        struct _lp_read_orphan_transf transf;
        struct lbd_pool *lp = NULL;

	if (!(lpinfo = lbdcache_lpinfo_from_lpname(orphan_lpname, NULL)))
		return NULL;

	if (!(fmt = lbdcache_fmt_from_lpname(cmd, orphan_lpname, NULL, 0)))
		return NULL;

        lp = fmt->orphan_lp;
        lp->lpinfo = lpinfo;
        jd_list_init(&lp->dvs);

        transf.cmd = cmd;
        transf.lp = lp;
        transf.fmt = fmt;

        if (!lbdcache_foreach_dv(lpinfo, _lp_read_orphan_dv, &transf))
                return NULL;
        return lp;
}

struct lbd_pool *_lp_read(struct cmd_context *cmd, const char *lp_name,
			     const char *lpid, uint32_t read_flags)
{
	int consistent = 1;
        uint32_t warn_flags=0; 
	if (is_orphan_lp(lp_name)) {
	        return _lp_read_orphans(cmd, warn_flags, lp_name, &consistent);
        }
        /* FIXME: for !is_orphan_lp*/
        return NULL;
}

struct lbd_pool *lp_read(struct cmd_context *cmd, const char *lp_name,
                const char *lpid, uint32_t read_flags, uint32_t lockd_state)
{
        struct lbd_pool *lp;
        lp = _lp_read(cmd, lp_name, lpid, read_flags);
        if (!lp) {
                printf("err read lp.\n");
                return NULL;
        }
        return lp; 
}

struct lbd_pool *lp_create(struct cmd_context *cmd, const char *lp_name)
{
	struct lbd_pool *lp;
	struct format_instance_ctx fic = {
		.type = FMT_INSTANCE_MDAS | FMT_INSTANCE_AUX_MDAS,
		.context.lp_ref.lp_name = lp_name
	};
	struct format_instance *fid;

	if (!(lp = alloc_lp("lp_create", cmd, lp_name)))
		goto bad;

	lp->extent_size = DEFAULT_EXTENT_SIZE * 2;
	lp->system_id = NULL;
	lp->max_lbd = 0;
	lp->max_dv = 0;
	lp->mda_copies = 0;

	if (!(fid = cmd->fmt->ops->create_instance(cmd->fmt, &fic))) {
		printf("err: failed to create format instance.\n");
		goto bad;
	}

bad:
        return NULL;
}

int lp_write(struct lbd_pool *lp)
{
	struct metadata_area *mda;
	/* write to each copy of the metadata area */
	jd_list_iterate_items(mda, &lp->fid->metadata_areas_in_use) {
		if (!mda->ops->lp_write(lp->fid, lp, mda))
                        break;
        }
        return 1;
}

struct format_instance *alloc_fid(const struct format_type *fmt,
				  const struct format_instance_ctx *fic)
{
	struct format_instance *fid;

	if (!(fid = malloc(sizeof(*fid)))) {
		printf("err: couldn't allocate format_instance object.\n");
		goto bad;
	}

	fid->ref_count = 1;
	fid->type = fic->type;
	fid->fmt = fmt;

	jd_list_init(&fid->metadata_areas_in_use);
	jd_list_init(&fid->metadata_areas_ignored);

	return fid;

bad:
	free(fid);
	return NULL;
}

int fid_add_mda(struct format_instance *fid, struct metadata_area *mda,
		 const char *key, size_t key_len, const unsigned sub_key)
{
        /* FIXME: */
        return 1;
}

void dv_set_fid(struct disk_volume *dv,
		struct format_instance *fid)
{
	if (fid == dv->fid)
		return;

	dv->fid = fid;
}

void lp_set_fid(struct lbd_pool *lp,
		 struct format_instance *fid)
{
	struct dv_list *dvl;

	if (fid == lp->fid)
		return;

	jd_list_iterate_items(dvl, &lp->dvs)
		dv_set_fid(dvl->dv, fid);

	lp->fid = fid;
}
