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
#include "device.h"
#include "metadata.h"
#include "lbdcache.h"

int is_orphan_lp(const char *lp_name)
{
	return (lp_name && !strncmp(lp_name, ORPHAN_PREFIX, sizeof(ORPHAN_PREFIX) - 1)) ? 1 : 0;
}

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

int get_lpnameids(struct cmd_context *cmd, struct jd_list *lpnameids,
                int include_internal)
{
	lbdcache_get_lpnameids(cmd, include_internal, lpnameids);
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
        if (!lp){
                printf("err read lp.\n");
                return NULL;
        }
        return lp; 
}
