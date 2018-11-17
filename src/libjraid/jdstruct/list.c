/*
 * list.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <assert.h>
#include <stdio.h>
#include "jdstruct.h"

void jd_list_init(struct jd_list *head)
{
	head->n = head->p = head;
}

void jd_list_add(struct jd_list *head, struct jd_list *elem)
{
	assert(head->n);

	elem->n = head;
	elem->p = head->p;

	head->p->n = elem;
	head->p = elem;
}

void jd_list_add_h(struct jd_list *head, struct jd_list *elem)
{
	assert(head->n);

	elem->n = head->n;
	elem->p = head;

	head->n->p = elem;
	head->n = elem;
}

void jd_list_del(struct jd_list *elem)
{
	elem->n->p = elem->p;
	elem->p->n = elem->n;
}

void jd_list_move(struct jd_list *head, struct jd_list *elem)
{
        jd_list_del(elem);
        jd_list_add(head, elem);
}

int jd_list_empty(const struct jd_list *head)
{
	return head->n == head;
}

void jd_list_splice(struct jd_list *head, struct jd_list *head1)
{
	assert(head->n);
	assert(head1->n);

        if (jd_list_empty(head1))
	        return;

	head1->p->n = head;
	head1->n->p = head->p;

	head->p->n = head1->n;
	head->p = head1->p;

	jd_list_init(head1);
}

unsigned int jd_list_size(const struct jd_list *head)
{
	unsigned int s = 0;
	const struct jd_list *v;

	jd_list_iterate(v, head)
	    s++;

	return s;
}
