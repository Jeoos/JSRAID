/*
 * libjraid.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#ifndef __LIBJRAID_H__
#define __LIBJRAID_H__

struct jd_pool;

struct jd_list {
	struct jd_list *n, *p;
};

#define JD_LIST_HEAD_INIT(name)	 { &(name), &(name) }
#define JD_LIST_INIT(name)	struct jd_list name = JD_LIST_HEAD_INIT(name)

struct jd_pool *jraid_pool_create(const char *name, size_t chunk_hint);
void jraid_pool_destroy(struct jd_pool *p);

#endif
