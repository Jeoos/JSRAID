/*
 * dev-cache.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <sys/stat.h>
#include <string.h>

#include "libjraid.h"
#include "toolcontext.h"
#include "bdstruct.h"

static struct {
	struct jd_pool *mem;
	struct jd_hash_table *names;
	struct jd_hash_table *lbdid_index;
	struct btree *devices;
	const char *dev_dir;

	struct jd_list dirs;
	struct jd_list files;

} _cache;

struct device *dev_cache_get(const char *name)
{
	struct stat buf;
	struct device *d = (struct device *) jd_hash_lookup(_cache.names, name);

        if (d && (d->flags & DEV_REGULAR)){
                printf("oh yes ...\n");
		return d;
        }

        /* the entry's wrong, then remove it */
	if (d && (buf.st_rdev != d->dev)) {
		jd_hash_remove(_cache.names, name);
		d = NULL;
	}

	if (!d) {
                /* create and insert */
        }

        if (d && (d->flags & DEV_REGULAR)){
                printf("oh yes ...\n");
		return d;
        }

        printf("oh no ...\n");
        return NULL;
}

int dev_cache_exit(void)
{
	memset(&_cache, 0, sizeof(_cache));
        return 1;
}

int dev_cache_init(struct cmd_context *cmd)
{
	_cache.names = NULL;

	if (!(_cache.mem = jraid_pool_create("dev_cache", 10 * 1024)))
	        return 0;

	if (!(_cache.names = jd_hash_create(128)) ||
	    !(_cache.lbdid_index = jd_hash_create(32))) {
		jraid_pool_destroy(_cache.mem);
		_cache.mem = 0;
		return 0;
	}

	if (!(_cache.devices = btree_create(_cache.mem))) {
		printf("Couldn't create binary tree for dev-cache.");
		goto bad;
	}

	if (!(_cache.dev_dir = strdup(cmd->dev_dir))) {
		printf("strdup dev_dir failed.");
		goto bad;
	}

	jd_list_init(&_cache.dirs);
	jd_list_init(&_cache.files);

	return 1;

bad:
	dev_cache_exit();
	return 0;
}
