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

#define ID_LEN 32
#define ORPHAN_VG_NAME "ORPHAN"

static JD_LIST_INIT(_lpinfos);
static struct jd_hash_table *_lpid_hash = NULL;
static struct jd_hash_table *_lpname_hash = NULL;

/* one per device */
struct lbdcache_info {
	struct jd_list list;	/* join VG members together */
	struct jd_list mdas;	/* list head for metadata areas */
	struct jd_list das;	/* list head for data areas */
	struct lbdcache_lpinfo *lpinfo;	/* NULL == unknown */
	//struct label *label;
	//const struct format_type *fmt;
	struct device *dev;
	uint64_t device_size;	/* bytes */
};

/* one per LP */
struct lbdcache_lpinfo {
	struct jd_list list;	/* join these lpinfos together */
	struct jd_list infos;	/* list head for lbdcache_infos */
	//const struct format_type *fmt;
	char *lpname;		/* "" == orphan */
	uint32_t status;
	char lpid[ID_LEN + 1];
	char _padding[7];
	struct lbdcache_lpinfo *next; /* another LP with same name? */
};

static int init_lbdcache_orphans(struct cmd_context *cmd)
{
        struct lbdcache_lpinfo *lpinfo;

        if (!(lpinfo = jd_hash_lookup(_lpname_hash, ORPHAN_VG_NAME))){
                if (!(lpinfo = malloc(sizeof(*lpinfo)))) {
                        printf("err malloc.\n");
                        return 0;
                }
                lpinfo->lpname = strdup(ORPHAN_VG_NAME);

                if (!jd_hash_insert(_lpname_hash, lpinfo->lpname, lpinfo))
                        return 0;

	        jd_list_add(&_lpinfos, &lpinfo->list);
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
		if (!include_internal)
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
