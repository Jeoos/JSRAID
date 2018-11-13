/*
 * bdstruct.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __BDSTRUCT_H__
#define __BDSTRUCT_H__

struct btree;
struct jd_pool;

struct btree *btree_create(struct jd_pool *mem);

void *btree_lookup(const struct btree *t, uint32_t k);
int btree_insert(struct btree *t, uint32_t k, void *data);

struct btree_iter;
void *btree_get_data(const struct btree_iter *it);

struct btree_iter *btree_first(const struct btree *t);
struct btree_iter *btree_next(const struct btree_iter *it);

#endif
