/*
 * lbdcache.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <malloc.h>
#include <string.h>
#include "lbdcache.h"
#include "libjraid.h"
#include "toolcontext.h"
#include "metadata-out.h"
#include "label.h"

#define ID_LEN 32

static JD_LIST_INIT(_lpinfos);
static struct jd_hash_table *_lpid_hash = NULL;
static struct jd_hash_table *_lpname_hash = NULL;

/* one per device */
struct lbdcache_info {
	struct jd_list list;	/* join LP members together */
	struct jd_list mdas;	/* list head for metadata areas */
	struct jd_list das;	/* list head for data areas */
	struct lbdcache_lpinfo *lpinfo;	/* NULL == unknown */
	struct label *label;
	const struct format_type *fmt;
	struct device *dev;
	uint64_t device_size;	/* bytes */
};

/* one per LP */
struct lbdcache_lpinfo {
	struct jd_list list;	/* join these lpinfos together */
	struct jd_list infos;	/* list head for lbdcache_infos */
	const struct format_type *fmt;
	char *lpname;		/* "" == orphan */
	uint32_t status;
	char lpid[ID_LEN + 1];
	char _padding[7];
	struct lbdcache_lpinfo *next; /* another LP with same name? */
};

static int init_lbdcache_orphans(struct cmd_context *cmd)
{
        struct lbdcache_lpinfo *lpinfo;
	struct format_type *fmt;

        jd_list_iterate_items(fmt, &cmd->formats) {
                if(!fmt->orphan_lp_name) {
		        printf("return items.\n");
                        return 1;
                }
                if (!(lpinfo = jd_hash_lookup(_lpname_hash, fmt->orphan_lp_name))){
                        if (!(lpinfo = malloc(sizeof(*lpinfo)))) {
                                printf("err malloc.\n");
                                return 0;
                        }
                        lpinfo->lpname = strdup(fmt->orphan_lp_name);
                        jd_list_init(&lpinfo->infos);

                        if (!jd_hash_insert(_lpname_hash, lpinfo->lpname, lpinfo))
                                return 0;

                        lpinfo->fmt = fmt;
                        jd_list_add(&_lpinfos, &lpinfo->list);
                }
        }

	return 1;
}

int lbdcache_init(struct cmd_context *cmd)
{
	jd_list_init(&_lpinfos);

	if (!(_lpname_hash = jd_hash_create(128)))
		return 0;

	if (!(_lpid_hash = jd_hash_create(128)))
		return 0;

	if (!init_lbdcache_orphans(cmd))
		return 0;

	return 1;
}

int lbdcache_get_lpnameids(struct cmd_context *cmd, int include_internal,
			   struct jd_list *lpnameids)
{
	struct lpnameid_list *lpnl;
	struct lbdcache_lpinfo *lpinfo;

	jd_list_iterate_items(lpinfo, &_lpinfos) {
                /* FIXME: should find any, but orphan lp */
		//if (!include_internal && is_orphan_lp(lpinfo->lpname))
		if (!include_internal && !is_orphan_lp(lpinfo->lpname))
                        continue;
		
		if (!(lpnl = malloc(sizeof(*lpnl)))) {
			printf("lpnameid_list allocation failed.\n");
			return 0;
		}
		//lpnl->lpid = strdup(lpinfo->lpid);
		lpnl->lp_name = strdup(lpinfo->lpname);

		if (!lpnl->lpid && !lpnl->lp_name) {
			printf("lpnameid_list member allocation failed.\n");
			return 0;
		}

		jd_list_add(lpnameids, &lpnl->list);
	}
	return 1;
}

struct lbdcache_lpinfo *lbdcache_lpinfo_from_lpname(const char *lpname, const char *lpid)
{
	struct lbdcache_lpinfo *lpinfo;
        /* FIXME: when just use lpid */
        if (!lpname)
                return NULL;

	if (!(lpinfo = jd_hash_lookup(_lpname_hash, lpname))) {
                printf("err: hash lookup for lpinfo.\n");
                return NULL;
        }
        return lpinfo;
}

const struct format_type *lbdcache_fmt_from_lpname(struct cmd_context *cmd,
                                const char *lpname, const char *lpid,
	                        unsigned revalidate_labels)
{
        struct lbdcache_lpinfo *lpinfo;

	if (!(lpinfo = lbdcache_lpinfo_from_lpname(lpname, lpid))) {
                printf("err find lpinfo.\n");
                return NULL;
        }

        /* FIXME: for revalidate_labels? */ 

        return lpinfo->fmt;
}

int lbdcache_foreach_dv(struct lbdcache_lpinfo *lpinfo,
			int (*fun)(struct lbdcache_info *, void *),
			void *transf)
{
	struct lbdcache_info *info;
	jd_list_iterate_items(info, &lpinfo->infos) {
		if (!fun(info, transf))
			return 0;
	}
	return 1;
}

struct lbdcache_info *_create_info(struct labeller *labeller, struct device *dev)
{
        struct lbdcache_info *info;
	struct label *label;

        if (!labeller) 
                goto next;

	if (!(label = label_create(labeller)))
		return NULL;
next:
        if (!(info = malloc(sizeof(*info)))) {
                printf("err: failed to alloc info.\n");
                return NULL;
        }
	info->dev = dev;

        return info;
}

struct lbdcache_info *lbdcache_info_from_lpinfo(struct device *dev, 
                        struct lbdcache_lpinfo *lpinfo)
{
        struct lbdcache_info *info;
        jd_list_iterate_items(info, &lpinfo->infos) {
                if (dev == info->dev)
                        return info;
        }
        return NULL;
}

int _info_attach_label(struct lbdcache_info *info, struct label *label)
{
        struct labeller *labeller;

        if (label) {
               info->label = label;
               return 1;
        }
        labeller = info->fmt->labeller;
        if (!(label = label_create(labeller))) {
                printf("err: to attach labeller.\n");
		return 0;
        }

        info->label = label;
        return 1;
}

void _lpinfo_attach_info(struct lbdcache_lpinfo *lpinfo,
				struct lbdcache_info *info)
{
	if (!lpinfo)
		return;

	info->lpinfo = lpinfo;
        info->fmt = lpinfo->fmt;
	jd_list_add(&lpinfo->infos, &info->list);
}

void lbdcache_set_dv(struct disk_volume *dv, struct lbdcache_info *info)
{
        dv->dev = info->dev;
}

char *lbdcache_get_lpname(struct lbd_pool *lp)
{
        printf("lpname:%s\n", lp->lpinfo->lpname);
        if(lp->lpinfo)
                return lp->lpinfo->lpname;
        return NULL;
}

struct label *lbdcache_get_label(struct lbdcache_info *info) 
{
        return info->label;
}
