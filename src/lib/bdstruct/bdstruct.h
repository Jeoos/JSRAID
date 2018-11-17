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
struct jd_list;

struct btree *btree_create(struct jd_pool *mem);

void *btree_lookup(const struct btree *t, uint32_t k);
int btree_insert(struct btree *t, uint32_t k, void *data);

struct btree_iter;
void *btree_get_data(const struct btree_iter *it);

struct btree_iter *btree_first(const struct btree *t);
struct btree_iter *btree_next(const struct btree_iter *it);

struct jd_list *str_list_create(void);
int str_list_add_no_dup_check(struct jd_list *sll, const char *str);
int str_list_add_h_no_dup_check(struct jd_list *sll, const char *str);
int str_list_add(struct jd_list *sll, const char *str);
int str_list_add_list(struct jd_list *sll, struct jd_list *sll2);
void str_list_del(struct jd_list *sll, const char *str);
void str_list_wipe(struct jd_list *sll);
int str_list_dup(struct jd_list *sllnew, const struct jd_list *sllold);
int str_list_match_item(const struct jd_list *sll, const char *str);
int str_list_match_list(const struct jd_list *sll, const struct jd_list *sll2, const char **tag_matched);
int str_list_lists_equal(const struct jd_list *sll, const struct jd_list *sll2);
char *str_list_to_str(const struct jd_list *list, const char *delim);
struct jd_list *str_to_str_list(const char *str, const char *delim, int ignore_multiple_delim);

#endif
