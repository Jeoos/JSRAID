/*
 * lp.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LP_H__
#define __LP_H__

struct cmd_context;
struct lbdcache_lpinfo;
struct jd_list;

struct lbd_pool {
	struct cmd_context *cmd;
	struct lbdcache_lpinfo *lpinfo;
	struct jd_list dvs;
};
struct lbd_pool *alloc_lp(const char *pool_name, struct cmd_context *cmd,
			      const char *lp_name);

void free_orphan_lp(struct lbd_pool *lp);
#endif
