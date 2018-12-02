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
#include <stdlib.h>

#include "device.h"
#include "libjraid.h"
#include "toolcontext.h"
#include "bdstruct.h"

struct dev_iter {
	struct btree_iter *current;
        /* maybe filter */
};

static struct {
	struct jd_pool *mem;
	struct jd_hash_table *names;
	struct jd_hash_table *lbdid_index;
	struct btree *devices;
	const char *dev_dir;

	struct jd_list dirs;
	struct jd_list files;

} _cache;

static int _insert_dir(const char *dir)
{
        return 1;
}

static void _dev_init(struct device *dev, int max_error_count)
{
	dev->phys_block_size = -1;
	dev->block_size = -1;
	dev->fd = -1;
	dev->read_ahead = -1;
	dev->max_error_count = max_error_count;
}

static struct device *_dev_create(dev_t d)
{
	struct device *dev;

	if (!(dev = malloc(sizeof(*dev)))) {
		printf("struct device allocation failed\n");
		return NULL;
	}

	_dev_init(dev, 0);
	dev->dev = d;

	return dev;
}

static int _insert_dev(const char *path, dev_t d)
{
	struct device *dev;
	struct device *dev_by_devt;
	struct device *dev_by_path;
        struct jd_str_list *alias;

	dev_by_devt = (struct device *) btree_lookup(_cache.devices, (uint32_t) d);
	dev_by_path = (struct device *) jd_hash_lookup(_cache.names, path);

	if (dev_by_devt && dev_by_path && (dev_by_devt == dev_by_path)) {
		printf("found dev %d:%d %s - exists.\n",
			       (int)MAJOR(d), (int)MINOR(d), path);
		return 1;
	}

        if (!dev_by_devt && !dev_by_path) {
                printf("create for %d:%d %s\n",
                        (int)MAJOR(d), (int)MINOR(d), path);

		dev = _dev_create(d);

                alias = malloc(sizeof(*alias));
                if (!alias) {
                        return 0;
                        printf("err: failed to alloc alias.\n");
                }
                alias->str = strdup(path);
                jd_list_init(&dev->aliases);
	        jd_list_add(&dev->aliases, &alias->list);

		if (!(btree_insert(_cache.devices, (uint32_t) d, dev))) {
			printf("couldn't insert device into binary tree.\n");
			free(dev);
			return 0;
		}

		if (!jd_hash_insert(_cache.names, path, dev)) {
			printf("couldn't add name to hash in dev cache.\n");
			return 0;
		}
                return 1;
        }

        /* steps fixme: for other cases */

        return 0;
}

static int _insert(const char *path, int rec, const struct stat *info)
{
	struct stat tinfo;

	if (!info) {
		if (stat(path, &tinfo) < 0) {
                        printf("stat:%s\n", path);
			return 0;
		}
		info = &tinfo;
	}

	if (S_ISDIR(info->st_mode)) {	/* add a directory */
		/* check it's not a symbolic link */
		if (lstat(path, &tinfo) < 0) {
                        printf("lstat:%s\n", path);
			return 0;
		}

		if (S_ISLNK(tinfo.st_mode)) {
			printf("%s: Symbolic link to directory\n", path);
			return 1;
		}

		if (rec && !_insert_dir(path))
			return 0;
	} else {		/* add a device */
                if (!S_ISBLK(info->st_mode)){
			return 1;
                }
		if (!_insert_dev(path, info->st_rdev))
			return 0;
	}

        return 1;
}

struct device *dev_cache_get(const char *name)
{
	struct stat buf;
	struct device *d = (struct device *) jd_hash_lookup(_cache.names, name);

        if (d && (d->flags & DEV_REGULAR)){
		return d;
        }

        /* the entry's wrong, then remove it */
	if (stat(name, &buf) < 0) {
		if (d)
			jd_hash_remove(_cache.names, name);
		printf("stat:%s\n", name);
		d = NULL;
        }

	if (d && (buf.st_rdev != d->dev)) {
		jd_hash_remove(_cache.names, name);
		d = NULL;
	}

	if (!d) {
                /* create and insert */
		_insert(name, 0, &buf);
		d = (struct device *) jd_hash_lookup(_cache.names, name);
                if (!d) {
                        printf("err: can not find \n");
                        /* steps fixme: try one more time */
                }
        }

	if (!d)
                return NULL;
	return d;
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

struct dev_iter *dev_iter_create(int unused)
{
	struct dev_iter *di = malloc(sizeof(*di));

	if (!di) {
		printf("dev_iter allocation failed");
		return NULL;
	}

	di->current = btree_first(_cache.devices);

	return di;
}

static struct device *_iter_next(struct dev_iter *iter)
{
	struct device *d = btree_get_data(iter->current);
	iter->current = btree_next(iter->current);
	return d;
}

struct device *dev_iter_get(struct dev_iter *iter)
{
	while (iter->current) {
		struct device *d = _iter_next(iter);

                /* FIXME: more check */

                if (d)
			return d;
	}

	return NULL;
}
