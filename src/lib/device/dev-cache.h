/*
 * dev-cache.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

 #ifndef __DEV_CACHE_H__
 #define __DEV_CACHE_H__

#include "device.h"

struct cmd_context;
struct device *dev_cache_get(const char *name);
int dev_cache_init(struct cmd_context *cmd);
int dev_cache_exit(void);
struct dev_iter *dev_iter_create(int unused);
struct device *dev_iter_get(struct dev_iter *iter);

 #endif
