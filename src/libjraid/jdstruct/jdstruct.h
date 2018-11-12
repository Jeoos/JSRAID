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


#define jd_list_struct_base(v, t, head) \
    ((t *)((const char *)(v) - (const char *)&((t *) 0)->head))

#endif

#define jd_list_item(v, t) jd_list_struct_base((v), t, list)

#define jd_list_iterate_items_gen_safe(v, t, head, field) \
	for (v = jd_list_struct_base((head)->n, __typeof__(*v), field), \
	     t = jd_list_struct_base(v->field.n, __typeof__(*v), field); \
	     &v->field != (head); \
	     v = t, t = jd_list_struct_base(v->field.n, __typeof__(*v), field))

#define jd_list_iterate_items_safe(v, t, head) \
	jd_list_iterate_items_gen_safe(v, t, (head), list)

#define jd_list_iterate_items_gen(v, head, field) \
	for (v = jd_list_struct_base((head)->n, __typeof__(*v), field); \
	     &v->field != (head); \
	     v = jd_list_struct_base(v->field.n, __typeof__(*v), field))

#define jd_list_iterate_items(v, head) jd_list_iterate_items_gen(v, (head), list)
