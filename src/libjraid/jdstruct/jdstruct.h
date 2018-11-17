/*
 * jdstruct.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JDSTRUCT_H__
#define __JDSTRUCT_H__

#include <stdint.h>

typedef void (*jd_hash_iterate_fn) (void *data);

struct jd_hash_table;

struct jd_list {
	struct jd_list *n, *p;
};

void jd_list_init(struct jd_list *head);
void jd_list_add(struct jd_list *head, struct jd_list *elem);
void jd_list_add_h(struct jd_list *head, struct jd_list *elem);
void jd_list_del(struct jd_list *elem);
int jd_list_empty(const struct jd_list *head);
void jd_list_move(struct jd_list *head, struct jd_list *elem);
void jd_list_splice(struct jd_list *head, struct jd_list *head1);

struct jd_hash_table *jd_hash_create(unsigned size_hint);
void jd_hash_destroy(struct jd_hash_table *t);

int jd_hash_insert(struct jd_hash_table *t, const char *key, void *data);
int jd_hash_insert_binary(struct jd_hash_table *t, const void *key,
			  uint32_t len, void *data);
int jd_hash_insert_allow_multiple(struct jd_hash_table *t, const char *key,
				  const void *val, uint32_t val_len);

void *jd_hash_lookup(struct jd_hash_table *t, const char *key);
void *jd_hash_lookup_with_count(struct jd_hash_table *t, const char *key, int *count);
void *jd_hash_lookup_with_val(struct jd_hash_table *t, const char *key,
			      const void *val, uint32_t val_len);
void *jd_hash_lookup_binary(struct jd_hash_table *t, const void *key,
			    uint32_t len);

void jd_hash_remove(struct jd_hash_table *t, const char *key);
void jd_hash_remove_with_val(struct jd_hash_table *t, const char *key,
			     const void *val, uint32_t val_len);
void jd_hash_remove_binary(struct jd_hash_table *t, const void *key,
			uint32_t len);

struct jd_hash_node *jd_hash_get_first(struct jd_hash_table *t);
struct jd_hash_node *jd_hash_get_next(struct jd_hash_table *t, struct jd_hash_node *n);
unsigned jd_hash_get_num_entries(struct jd_hash_table *t);
char *jd_hash_get_key(struct jd_hash_table *t __attribute__((unused)),
		      struct jd_hash_node *n);
void *jd_hash_get_data(struct jd_hash_table *t __attribute__((unused)),
		       struct jd_hash_node *n);

void jd_hash_iter(struct jd_hash_table *t, jd_hash_iterate_fn f);
void jd_hash_wipe(struct jd_hash_table *t);

unsigned int jd_list_size(const struct jd_list *head);

#define jd_list_struct_base(v, t, head) \
    ((t *)((const char *)(v) - (const char *)&((t *) 0)->head))


#define jd_list_item(v, t) jd_list_struct_base((v), t, list)

#define jd_list_iterate_items_gen_safe(v, t, head, field) \
	for (v = jd_list_struct_base((head)->n, __typeof__(*v), field), \
	     t = jd_list_struct_base(v->field.n, __typeof__(*v), field); \
	     &v->field != (head); \
	     v = t, t = jd_list_struct_base(v->field.n, __typeof__(*v), field))

#define jd_list_iterate_safe(v, t, head) \
	for (v = (head)->n, t = v->n; v != head; v = t, t = v->n)

#define jd_list_iterate_items_safe(v, t, head) \
	jd_list_iterate_items_gen_safe(v, t, (head), list)

#define jd_list_iterate_items_gen(v, head, field) \
	for (v = jd_list_struct_base((head)->n, __typeof__(*v), field); \
	     &v->field != (head); \
	     v = jd_list_struct_base(v->field.n, __typeof__(*v), field))

#define jd_list_iterate_items(v, head) jd_list_iterate_items_gen(v, (head), list)

#define jd_list_iterate(v, head) \
	for (v = (head)->n; v != head; v = v->n)

#endif
