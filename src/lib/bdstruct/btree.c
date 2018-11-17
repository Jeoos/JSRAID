/*
 * btree.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <string.h>

#include "bdstruct.h"

struct node {
	uint32_t key;
	struct node *l, *r, *p;

	void *data;
};

struct btree_iter {

};

struct btree {
	struct jd_pool *mem;
	struct node *root;
};

struct btree *btree_create(struct jd_pool *mem)
{
	struct btree *t = malloc(sizeof(*t));

	if (t) {
		t->mem = mem;
		t->root = NULL;
	}

	return t;
}

/*
 * Shuffle the bits in a key, to try and remove
 * any ordering.
 */
static uint32_t _shuffle(uint32_t k)
{
#if 1
	return ((k & 0xff) << 24 |
		(k & 0xff00) << 8 |
		(k & 0xff0000) >> 8 | (k & 0xff000000) >> 24);
#else
	return k;
#endif
}

static struct node *const *_lookup(struct node *const *c, uint32_t key,
			     struct node **p)
{
	*p = NULL;
	while (*c) {
		*p = *c;
		if ((*c)->key == key)
			break;

		if (key < (*c)->key)
			c = &(*c)->l;

		else
			c = &(*c)->r;
	}

	return c;
}

void *btree_lookup(const struct btree *t, uint32_t k)
{
	uint32_t key = _shuffle(k);
	struct node *p, *const *c = _lookup(&t->root, key, &p);
	return (*c) ? (*c)->data : NULL;
}

int btree_insert(struct btree *t, uint32_t k, void *data)
{
	uint32_t key = _shuffle(k);
	struct node *p, **c = (struct node **) _lookup(&t->root, key, &p), *n;

	if (!*c) {
		if (!(n = malloc(sizeof(*n))))
			return 0;

		n->key = key;
		n->data = data;
		n->l = n->r = NULL;
		n->p = p;

		*c = n;
	}

	return 1;
}

void *btree_get_data(const struct btree_iter *it)
{
	return ((const struct node *) it)->data;
}

static struct node *_left(struct node *n)
{
	while (n->l)
		n = n->l;
	return n;
}

struct btree_iter *btree_first(const struct btree *t)
{
	if (!t->root)
		return NULL;

	return (struct btree_iter *) _left(t->root);
}

struct btree_iter *btree_next(const struct btree_iter *it)
{
	struct node *n = (struct node *) it;
	uint32_t k = n->key;

	if (n->r)
		return (struct btree_iter *) _left(n->r);

	do
		n = n->p;
	while (n && k > n->key);

	return (struct btree_iter *) n;
}
