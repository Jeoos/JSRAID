/*
 * str_list.c 
 *
* Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bdstruct.h"
#include "libjraid.h"

struct jd_list *str_list_create(void)
{
	struct jd_list *sl;

	if (!(sl = malloc(sizeof(struct jd_list)))) {
		printf("str_list allocation failed\n");
		return NULL;
	}

	jd_list_init(sl);

	return sl;
}

static int _str_list_add_no_dup_check(struct jd_list *sll, const char *str, int as_first)
{
	struct jd_str_list *sln;

	if (!str)
		return 0;

	if (!(sln = malloc(sizeof(*sln))))
		return 0;

	sln->str = str;
	if (as_first)
		jd_list_add_h(sll, &sln->list);
	else
		jd_list_add(sll, &sln->list);

	return 1;
}

int str_list_add_no_dup_check(struct jd_list *sll, const char *str)
{
	return _str_list_add_no_dup_check(sll, str, 0);
}

int str_list_add_h_no_dup_check(struct jd_list *sll, const char *str)
{
	return _str_list_add_no_dup_check(sll, str, 1);
}

int str_list_add(struct jd_list *sll, const char *str)
{
	if (!str)
		return 0;

	/* already in list? */
	if (str_list_match_item(sll, str))
		return 1;

	return str_list_add_no_dup_check(sll, str);
}

/* add contents of sll2 to sll */
int str_list_add_list(struct jd_list *sll, struct jd_list *sll2)
{
	struct jd_str_list *sl;

	if (!sll2)
		return 0;

	jd_list_iterate_items(sl, sll2)
		if (!str_list_add(sll, sl->str))
			return 0;

	return 1;
}

void str_list_del(struct jd_list *sll, const char *str)
{
	struct jd_list *slh, *slht;

	jd_list_iterate_safe(slh, slht, sll)
		if (!strcmp(str, jd_list_item(slh, struct jd_str_list)->str))
			jd_list_del(slh);
}

void str_list_wipe(struct jd_list *sll)
{
	struct jd_list *slh, *slht;

	jd_list_iterate_safe(slh, slht, sll)
		jd_list_del(slh);
}

int str_list_dup(struct jd_list *sllnew, const struct jd_list *sllold)
{
	struct jd_str_list *sl;

	jd_list_init(sllnew);

	jd_list_iterate_items(sl, sllold) {
		if (!str_list_add(sllnew, strdup(sl->str)))
			return 0;
	}

	return 1;
}

/*
 * Is item on list?
 */
int str_list_match_item(const struct jd_list *sll, const char *str)
{
	struct jd_str_list *sl;

	jd_list_iterate_items(sl, sll)
		if (!strcmp(str, sl->str))
			return 1;

	return 0;
}

/*
 * Is at least one item on both lists?
 * If tag_matched is non-NULL, it is set to the tag that matched.
 */
int str_list_match_list(const struct jd_list *sll, const struct jd_list *sll2, const char **tag_matched)
{
	struct jd_str_list *sl;

	jd_list_iterate_items(sl, sll)
		if (str_list_match_item(sll2, sl->str)) {
			if (tag_matched)
				*tag_matched = sl->str;
			return 1;
		}

	return 0;
}

/*
 * Do both lists contain the same set of items?
 */
int str_list_lists_equal(const struct jd_list *sll, const struct jd_list *sll2)
{
	struct jd_str_list *sl;

	if (jd_list_size(sll) != jd_list_size(sll2))
		return 0;

	jd_list_iterate_items(sl, sll)
		if (!str_list_match_item(sll2, sl->str))
			return 0;

	return 1;
}

char *str_list_to_str(const struct jd_list *list, const char *delim)
{
	size_t delim_len = strlen(delim);
	unsigned list_size = jd_list_size(list);
	struct jd_str_list *sl;
	char *str, *p;
	size_t len = 0;
	unsigned i = 0;

	jd_list_iterate_items(sl, list)
		len += strlen(sl->str);
	if (list_size > 1)
		len += ((list_size - 1) * delim_len);

	str = malloc(len+1);
	if (!str) {
		printf("str_list_to_str: string allocation failed.");
		return NULL;
	}
	str[len] = '\0';
	p = str;

	jd_list_iterate_items(sl, list) {
		len = strlen(sl->str);
		memcpy(p, sl->str, len);
		p += len;

		if (++i != list_size) {
			memcpy(p, delim, delim_len);
			p += delim_len;
		}
	}

	return str;
}

struct jd_list *str_to_str_list(const char *str, const char *delim, int ignore_multiple_delim)
{
	size_t delim_len = strlen(delim);
	struct jd_list *list;
	const char *p1, *p2, *next;
	char *str_item;
	size_t len;

	if (!(list = str_list_create())) {
		printf("str_to_str_list: string list allocation failed.");
		return NULL;
	}

	p1 = p2 = str;
	while (*p1) {
		if (!(p2 = strstr(p1, delim)))
			next = p2 = str + strlen(str);
		else
			next = p2 + delim_len;

		len = p2 - p1;
		str_item = malloc(len+1);
		if (!str_item) {
			printf("str_to_str_list: string list item allocation failed.");
			goto bad;
		}
		memcpy(str_item, p1, len);
		str_item[len] = '\0';

		if (!str_list_add_no_dup_check(list, str_item))
			goto bad;

		if (ignore_multiple_delim) {
			while (!strncmp(next, delim, delim_len))
				next += delim_len;
		}

		p1 = next;
	}

	return list;
bad:
	free(list);

	return NULL;
}
