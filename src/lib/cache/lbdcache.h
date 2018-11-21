/*
 * lbdcache.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LBDCACHE_H__
#define  __LBDCACHE_H__

#define LP_ORPHANS	"#orphans"
#define ORPHAN_PREFIX LP_ORPHANS
#define ORPHAN_LP_NAME(fmt) ORPHAN_PREFIX "_" fmt

struct jd_list;
struct cmd_context;
struct format_type;
struct lbdcache_lpinfo;
struct lbdcache_info;
struct disk_volume;
struct labeller;
struct device;

int lbdcache_init(struct cmd_context *cmd);
int lbdcache_get_lpnameids(struct cmd_context *cmd, int include_internal,
			   struct jd_list *lpnameids);

const struct format_type *lbdcache_fmt_from_lpname(struct cmd_context *cmd,
                                const char *lpname, const char *lpid,
			        unsigned revalidate_labels);

struct lbdcache_lpinfo *lbdcache_lpinfo_from_lpname(const char *lpname, const char *lpid);

int lbdcache_foreach_dv(struct lbdcache_lpinfo *lpinfo,
			int (*fun)(struct lbdcache_info *, void *),
			void *transf);
struct lbdcache_info * _create_info(struct labeller *labeller, struct device *dev);
void _lpinfo_attach_info(struct lbdcache_lpinfo *lpinfo,
				struct lbdcache_info *info);
void lbdcache_set_dv(struct disk_volume *dv, struct lbdcache_info *info);
#endif
