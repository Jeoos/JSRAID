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

static struct lbd_pool *_lp_read_orphans(struct cmd_context *cmd,
					     uint32_t warn_flags,
					     const char *orphan_lpname,
					     int *consistent)
{
        struct lbd_pool *lp = NULL;

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
