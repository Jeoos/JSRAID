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
#include "../libjraid.h"

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

void jd_list_del(struct jd_list *elem)
{
	elem->n->p = elem->p;
	elem->p->n = elem->n;
}
