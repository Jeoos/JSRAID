/*
 * dstruct.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __DSTRUCT_H__
#define __DSTRUCT_H__

void jd_list_init(struct jd_list *head);
void jd_list_add(struct jd_list *head, struct jd_list *elem);
void jd_list_del(struct jd_list *elem);

#endif
