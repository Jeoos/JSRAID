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

struct jd_list;
struct cmd_context;

int lbdcache_init(struct cmd_context *cmd);
int lbdcache_get_lpnameids(struct cmd_context *cmd, int include_internal,
			   struct jd_list *lpnameids);

#endif
