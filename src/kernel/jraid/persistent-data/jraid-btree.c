/* 
 * jraid-btree.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "jraid-btree.h"
#include "jraid-block-manager.h"
#include "jraid-transaction-manager.h"

#include <linux/kernel.h>
#include <asm-generic/bug.h>

static void memcpy_disk(void *dest, const void *src, size_t len)
{
	memcpy(dest, src, len);
}

uint32_t calc_max_entries(size_t value_size, size_t block_size)
{
	uint32_t total, n;
	size_t elt_size = sizeof(uint64_t) + value_size; /* key + value */

	block_size -= sizeof(struct node_header);
	total = block_size / elt_size;
	n = total / 3;		/* rounds down */

	return 3 * n;
}

static void node_prepare_for_write(struct jd_block_validator *v,
				   struct jd_block *b,
				   size_t block_size)
{

}

static int node_check(struct jd_block_validator *v,
		      struct jd_block *b,
		      size_t block_size)
{
        return 0;
}

struct jd_block_validator btree_node_validator = {
	.name = "btree_node",
	.prepare_for_write = node_prepare_for_write,
	.check = node_check
};

int new_block(struct jd_btree_info *info, struct jd_block **result)
{
	return jd_tm_new_block(info->tm, &btree_node_validator, result);
}

void unlock_block(struct jd_btree_info *info, struct jd_block *b)
{

}

int jd_btree_empty(struct jd_btree_info *info, jd_block_t *root)
{
	int r;
	struct jd_block *b;
	struct btree_node *n;
	size_t block_size;
	uint32_t max_entries;

	r = new_block(info, &b);
	if (r < 0)
		return r;

	block_size = jd_bm_block_size(jd_tm_get_bm(info->tm));
	max_entries = calc_max_entries(0, block_size);

	n = jd_block_data(b);
	memset(n, 0, block_size);
	n->header.flags = cpu_to_le32(0);
	n->header.nr_entries = cpu_to_le32(0);
	n->header.max_entries = cpu_to_le32(max_entries);
	n->header.value_size = cpu_to_le32(0);

	*root = jd_block_location(b);
	unlock_block(info, b);

	return 0;
}

static void le64_inc(void *context, const void *value_le)
{
	struct jd_transaction_manager *tm = context;
	__le64 v_le;

	memcpy(&v_le, value_le, sizeof(v_le));
	jd_tm_inc(tm, le64_to_cpu(v_le));
}

static void le64_dec(void *context, const void *value_le)
{
	struct jd_transaction_manager *tm = context;
	__le64 v_le;

	memcpy(&v_le, value_le, sizeof(v_le));
	jd_tm_dec(tm, le64_to_cpu(v_le));
}

static int le64_equal(void *context, const void *value1_le, const void *value2_le)
{
	__le64 v1_le, v2_le;

	memcpy(&v1_le, value1_le, sizeof(v1_le));
	memcpy(&v2_le, value2_le, sizeof(v2_le));
	return v1_le == v2_le;
}

void init_le64_type(struct jd_transaction_manager *tm,
		    struct jd_btree_value_type *vt)
{
	vt->context = tm;
	vt->size = sizeof(__le64);
	vt->inc = le64_inc;
	vt->dec = le64_dec;
	vt->equal = le64_equal;
}

void init_shadow_spine(struct shadow_spine *s, struct jd_btree_info *info)
{
	s->info = info;
	s->count = 0;
}

void inc_children(struct jd_transaction_manager *tm, struct btree_node *n,
		  struct jd_btree_value_type *vt)
{
}

static int bn_shadow(struct jd_btree_info *info, jd_block_t orig,
	      struct jd_btree_value_type *vt,
	      struct jd_block **result)
{
	int r, inc;

	r = jd_tm_shadow_block(info->tm, orig, &btree_node_validator,
			       result, &inc);
	if (!r && inc)
		inc_children(info->tm, jd_block_data(*result), vt);

	return r;
}

int shadow_step(struct shadow_spine *s, jd_block_t b,
		struct jd_btree_value_type *vt)
{
	int r;

	if (s->count == 2) {
		unlock_block(s->info, s->nodes[0]);
		s->nodes[0] = s->nodes[1];
		s->count--;
	}

	r = bn_shadow(s->info, b, vt, s->nodes + s->count);
	if (!r) {
		if (!s->count)
			s->root = jd_block_location(s->nodes[0]);

		s->count++;
	}

	return r;
}

struct jd_block *shadow_current(struct shadow_spine *s)
{
	BUG_ON(!s->count);

	return s->nodes[s->count - 1];
}

struct jd_block *shadow_parent(struct shadow_spine *s)
{
	BUG_ON(s->count != 2);

	return s->count == 2 ? s->nodes[0] : NULL;
}

int shadow_has_parent(struct shadow_spine *s)
{
	return s->count >= 2;
}

static void array_insert(void *base, size_t elt_size, unsigned nr_elts,
			 unsigned index, void *elt)
	__jd_written_to_disk(elt)
{
	if (index < nr_elts)
		memmove(base + (elt_size * (index + 1)),
			base + (elt_size * index),
			(nr_elts - index) * elt_size);

	memcpy_disk(base + (elt_size * index), elt, elt_size);
}


static int insert_at(size_t value_size, struct btree_node *node, unsigned index,
		      uint64_t key, void *value)
		      __jd_written_to_disk(value)
{
	uint32_t nr_entries = le32_to_cpu(node->header.nr_entries);
	__le64 key_le = cpu_to_le64(key);

	if (index > nr_entries ||
	    index >= le32_to_cpu(node->header.max_entries)) {
		pr_debug("too many entries in btree node for insert\n");
		__jd_unbless_for_disk(value);
		return -ENOMEM;
	}

	__jd_bless_for_disk(&key_le);

	array_insert(node->keys, sizeof(*node->keys), nr_entries, index, &key_le);
	array_insert(value_base(node), value_size, nr_entries, index, value);
	node->header.nr_entries = cpu_to_le32(nr_entries + 1);

        return 0;
}

/*
 * Splits a node by creating two new children beneath the given node.
 *
 * Before:
 *	  +----------+
 *	  | A ++++++ |
 *	  +----------+
 *
 *
 * After:
 *	+------------+
 *	| A (shadow) |
 *	+------------+
 *	    |	|
 *   +------+	+----+
 *   |		     |
 *   v		     v
 * +-------+	 +-------+
 * | B +++ |	 | C +++ |
 * +-------+	 +-------+
 */
static int btree_split_beneath(struct shadow_spine *s, uint64_t key)
{
	int r;
	size_t size;
	unsigned nr_left, nr_right;
	struct jd_block *left, *right, *new_parent;
	struct btree_node *pn, *ln, *rn;
	__le64 val;

	new_parent = shadow_current(s);

	r = new_block(s->info, &left);
	if (r < 0)
		return r;

	r = new_block(s->info, &right);
	if (r < 0) {
		unlock_block(s->info, left);
		return r;
	}

	pn = jd_block_data(new_parent);
	ln = jd_block_data(left);
	rn = jd_block_data(right);

	nr_left = le32_to_cpu(pn->header.nr_entries) / 2;
	nr_right = le32_to_cpu(pn->header.nr_entries) - nr_left;

	ln->header.flags = pn->header.flags;
	ln->header.nr_entries = cpu_to_le32(nr_left);
	ln->header.max_entries = pn->header.max_entries;
	ln->header.value_size = pn->header.value_size;

	rn->header.flags = pn->header.flags;
	rn->header.nr_entries = cpu_to_le32(nr_right);
	rn->header.max_entries = pn->header.max_entries;
	rn->header.value_size = pn->header.value_size;

	memcpy(ln->keys, pn->keys, nr_left * sizeof(pn->keys[0]));
	memcpy(rn->keys, pn->keys + nr_left, nr_right * sizeof(pn->keys[0]));

	size = le32_to_cpu(pn->header.flags) & INTERNAL_NODE ?
		sizeof(__le64) : s->info->value_type.size;
	memcpy(value_ptr(ln, 0), value_ptr(pn, 0), nr_left * size);
	memcpy(value_ptr(rn, 0), value_ptr(pn, nr_left),
	       nr_right * size);

	/* new_parent should just point to l and r now */
	pn->header.flags = cpu_to_le32(INTERNAL_NODE);
	pn->header.nr_entries = cpu_to_le32(2);
	pn->header.max_entries = cpu_to_le32(
		calc_max_entries(sizeof(__le64),
				 jd_bm_block_size(
					 jd_tm_get_bm(s->info->tm))));
	pn->header.value_size = cpu_to_le32(sizeof(__le64));

	val = cpu_to_le64(jd_block_location(left));
	__jd_bless_for_disk(&val);
	pn->keys[0] = ln->keys[0];
	memcpy_disk(value_ptr(pn, 0), &val, sizeof(__le64));

	val = cpu_to_le64(jd_block_location(right));
	__jd_bless_for_disk(&val);
	pn->keys[1] = rn->keys[0];
	memcpy_disk(value_ptr(pn, 1), &val, sizeof(__le64));

	/*
	 * rejig the spine.  This is ugly, since it knows too
	 * much about the spine
	 */
	if (s->nodes[0] != new_parent) {
		unlock_block(s->info, s->nodes[0]);
		s->nodes[0] = new_parent;
	}
	if (key < le64_to_cpu(rn->keys[0])) {
		unlock_block(s->info, right);
		s->nodes[1] = left;
	} else {
		unlock_block(s->info, left);
		s->nodes[1] = right;
	}
	s->count = 2;

	return 0;
}

/* makes the assumption that no two keys are the same. */
static int bsearch(struct btree_node *n, uint64_t key, int want_hi)
{
	int lo = -1, hi = le32_to_cpu(n->header.nr_entries);

	while (hi - lo > 1) {
		int mid = lo + ((hi - lo) / 2);
		uint64_t mid_key = le64_to_cpu(n->keys[mid]);

		if (mid_key == key)
			return mid;

		if (mid_key < key)
			lo = mid;
		else
			hi = mid;
	}

	return want_hi ? hi : lo;
}

int lower_bound(struct btree_node *n, uint64_t key)
{
	return bsearch(n, key, 0);
}

static int upper_bound(struct btree_node *n, uint64_t key)
{
	return bsearch(n, key, 1);
}

/*
 * Splits a node by creating a sibling node and shifting half the nodes
 * contents across.  Assumes there is a parent node, and it has room for
 * another child.
 *
 * Before:
 *	  +--------+
 *	  | Parent |
 *	  +--------+
 *	     |
 *	     v
 *	+----------+
 *	| A ++++++ |
 *	+----------+
 *
 *
 * After:
 *		+--------+
 *		| Parent |
 *		+--------+
 *		  |	|
 *		  v	+------+
 *	    +---------+	       |
 *	    | A* +++  |	       v
 *	    +---------+	  +-------+
 *			  | B +++ |
 *			  +-------+
 *
 * Where A* is a shadow of A.
 */
static int btree_split_sibling(struct shadow_spine *s, unsigned parent_index,
			       uint64_t key)
{
	int r;
	size_t size;
	unsigned nr_left, nr_right;
	struct jd_block *left, *right, *parent;
	struct btree_node *ln, *rn, *pn;
	__le64 location;

	left = shadow_current(s);

	r = new_block(s->info, &right);
	if (r < 0)
		return r;

	ln = jd_block_data(left);
	rn = jd_block_data(right);

	nr_left = le32_to_cpu(ln->header.nr_entries) / 2;
	nr_right = le32_to_cpu(ln->header.nr_entries) - nr_left;

	ln->header.nr_entries = cpu_to_le32(nr_left);

	rn->header.flags = ln->header.flags;
	rn->header.nr_entries = cpu_to_le32(nr_right);
	rn->header.max_entries = ln->header.max_entries;
	rn->header.value_size = ln->header.value_size;
	memcpy(rn->keys, ln->keys + nr_left, nr_right * sizeof(rn->keys[0]));

	size = le32_to_cpu(ln->header.flags) & INTERNAL_NODE ?
		sizeof(uint64_t) : s->info->value_type.size;
	memcpy(value_ptr(rn, 0), value_ptr(ln, nr_left),
	       size * nr_right);

	/*
	 * Patch up the parent
	 */
	parent = shadow_parent(s);

	pn = jd_block_data(parent);
	location = cpu_to_le64(jd_block_location(left));
	__jd_bless_for_disk(&location);
	memcpy_disk(value_ptr(pn, parent_index),
		    &location, sizeof(__le64));

	location = cpu_to_le64(jd_block_location(right));
	__jd_bless_for_disk(&location);

	r = insert_at(sizeof(__le64), pn, parent_index + 1,
		      le64_to_cpu(rn->keys[0]), &location);
	if (r) {
		unlock_block(s->info, right);
		return r;
	}

	if (key < le64_to_cpu(rn->keys[0])) {
		unlock_block(s->info, right);
		s->nodes[1] = left;
	} else {
		unlock_block(s->info, left);
		s->nodes[1] = right;
	}

	return 0;
}

static int btree_insert_raw(struct shadow_spine *s, jd_block_t root,
			    struct jd_btree_value_type *vt,
			    uint64_t key, unsigned *index)
{
	int r, i = *index, top = 1;
	struct btree_node *node;

	for (;;) {
		r = shadow_step(s, root, vt);
		if (r < 0)
			return r;

		node = jd_block_data(shadow_current(s));

		/*
		 * We have to patch up the parent node, ugly, but I don't
		 * see a way to do this automatically as part of the spine
		 * op.
		 */
		if (shadow_has_parent(s) && i >= 0) { /* FIXME: second clause unness. */
			__le64 location = cpu_to_le64(jd_block_location(shadow_current(s)));

			__jd_bless_for_disk(&location);
			memcpy_disk(value_ptr(jd_block_data(shadow_parent(s)), i),
				    &location, sizeof(__le64));
		}

		node = jd_block_data(shadow_current(s));

		if (node->header.nr_entries == node->header.max_entries) {
			if (top)
				r = btree_split_beneath(s, key);
			else
				r = btree_split_sibling(s, i, key);

			if (r < 0)
				return r;
		}

		node = jd_block_data(shadow_current(s));

		i = lower_bound(node, key);

		if (le32_to_cpu(node->header.flags) & LEAF_NODE)
			break;

		if (i < 0) {
			/* change the bounds on the lowest key */
			node->keys[0] = cpu_to_le64(key);
			i = 0;
		}

		root = value64(node, i);
		top = 0;
	}

	if (i < 0 || le64_to_cpu(node->keys[i]) != key)
		i++;

	*index = i;
        return 0;
}

int shadow_root(struct shadow_spine *s)
{
	return s->root;
}

int exit_shadow_spine(struct shadow_spine *s)
{
	int r = 0, i;

	for (i = 0; i < s->count; i++) {
		unlock_block(s->info, s->nodes[i]);
	}

	return r;
}

static int insert(struct jd_btree_info *info, jd_block_t root,
		  uint64_t *keys, void *value, jd_block_t *new_root,
		  int *inserted)
		  __jd_written_to_disk(value)
{
	int r, need_insert;
	unsigned level, index = -1, last_level = info->levels - 1;
	jd_block_t block = root;
	struct shadow_spine spine;
	struct btree_node *n;
	struct jd_btree_value_type le64_type;

	init_le64_type(info->tm, &le64_type);
	init_shadow_spine(&spine, info);

	for (level = 0; level < (info->levels - 1); level++) {
		r = btree_insert_raw(&spine, block, &le64_type, keys[level], &index);
		if (r < 0)
			goto bad;

		n = jd_block_data(shadow_current(&spine));
		need_insert = ((index >= le32_to_cpu(n->header.nr_entries)) ||
			       (le64_to_cpu(n->keys[index]) != keys[level]));

		if (need_insert) {
			jd_block_t new_tree;
			__le64 new_le;

			r = jd_btree_empty(info, &new_tree);
			if (r < 0)
				goto bad;

			new_le = cpu_to_le64(new_tree);
			__jd_bless_for_disk(&new_le);

			r = insert_at(sizeof(uint64_t), n, index,
				      keys[level], &new_le);
			if (r)
				goto bad;
		}

		if (level < last_level)
			block = value64(n, index);
	}

	r = btree_insert_raw(&spine, block, &info->value_type,
			     keys[level], &index);
	if (r < 0)
		goto bad;

	n = jd_block_data(shadow_current(&spine));
	need_insert = ((index >= le32_to_cpu(n->header.nr_entries)) ||
		       (le64_to_cpu(n->keys[index]) != keys[level]));

	if (need_insert) {
		if (inserted)
			*inserted = 1;

		r = insert_at(info->value_type.size, n, index,
			      keys[level], value);
		if (r)
			goto bad_unblessed;
	} else {
		if (inserted)
			*inserted = 0;

		if (info->value_type.dec &&
		    (!info->value_type.equal ||
		     !info->value_type.equal(
			     info->value_type.context,
			     value_ptr(n, index),
			     value))) {
			info->value_type.dec(info->value_type.context,
					     value_ptr(n, index));
		}
		memcpy_disk(value_ptr(n, index),
			    value, info->value_type.size);
	}

	*new_root = shadow_root(&spine);
	exit_shadow_spine(&spine);

	return 0;

bad:
	__jd_unbless_for_disk(value);
bad_unblessed:
	exit_shadow_spine(&spine);
	return r;
}

int jd_btree_insert(struct jd_btree_info *info, jd_block_t root,
		    uint64_t *keys, void *value, jd_block_t *new_root)
		    __jd_written_to_disk(value)
{
	return insert(info, root, keys, value, new_root, NULL);
}

int jd_tm_read_lock(struct jd_transaction_manager *tm, jd_block_t b,
		    struct jd_block_validator *v,
		    struct jd_block **blk)
{
        return 0;
}

int bn_read_lock(struct jd_btree_info *info, jd_block_t b,
		 struct jd_block **result)
{
	return jd_tm_read_lock(info->tm, b, &btree_node_validator, result);
}

void init_ro_spine(struct ro_spine *s, struct jd_btree_info *info)
{
	s->info = info;
	s->count = 0;
	s->nodes[0] = NULL;
	s->nodes[1] = NULL;
}

int exit_ro_spine(struct ro_spine *s)
{
	int r = 0, i;

	for (i = 0; i < s->count; i++) {
		unlock_block(s->info, s->nodes[i]);
	}

	return r;
}

int ro_step(struct ro_spine *s, jd_block_t new_child)
{
	int r;

	if (s->count == 2) {
		unlock_block(s->info, s->nodes[0]);
		s->nodes[0] = s->nodes[1];
		s->count--;
	}

	r = bn_read_lock(s->info, new_child, s->nodes + s->count);
	if (!r)
		s->count++;

	return r;
}

struct btree_node *ro_node(struct ro_spine *s)
{
	struct jd_block *block;

	BUG_ON(!s->count);
	block = s->nodes[s->count - 1];

	return jd_block_data(block);
}

static int btree_lookup_raw(struct ro_spine *s, jd_block_t block, uint64_t key,
			    int (*search_fn)(struct btree_node *, uint64_t),
			    uint64_t *result_key, void *v, size_t value_size)
{
	int i, r;
	uint32_t flags, nr_entries;

	do {
		r = ro_step(s, block);
		if (r < 0)
			return r;

		i = search_fn(ro_node(s), key);

		flags = le32_to_cpu(ro_node(s)->header.flags);
		nr_entries = le32_to_cpu(ro_node(s)->header.nr_entries);
		if (i < 0 || i >= nr_entries)
			return -ENODATA;

		if (flags & INTERNAL_NODE)
			block = value64(ro_node(s), i);

	} while (!(flags & LEAF_NODE));

	*result_key = le64_to_cpu(ro_node(s)->keys[i]);
	memcpy(v, value_ptr(ro_node(s), i), value_size);

	return 0;
}

int jd_btree_lookup(struct jd_btree_info *info, jd_block_t root,
		    uint64_t *keys, void *value_le)
{
	unsigned level, last_level = info->levels - 1;
	int r = -ENODATA;
	uint64_t rkey;
	__le64 internal_value_le;
	struct ro_spine spine;

	init_ro_spine(&spine, info);
	for (level = 0; level < info->levels; level++) {
		size_t size;
		void *value_p;

		if (level == last_level) {
			value_p = value_le;
			size = info->value_type.size;

		} else {
			value_p = &internal_value_le;
			size = sizeof(uint64_t);
		}

		r = btree_lookup_raw(&spine, root, keys[level],
				     lower_bound, &rkey,
				     value_p, size);

		if (!r) {
			if (rkey != keys[level]) {
				exit_ro_spine(&spine);
				return -ENODATA;
			}
		} else {
			exit_ro_spine(&spine);
			return r;
		}

		root = le64_to_cpu(internal_value_le);
	}
	exit_ro_spine(&spine);

	return r;
}

static int jd_btree_lookup_next_single(struct jd_btree_info *info, jd_block_t root,
				       uint64_t key, uint64_t *rkey, void *value_le)
{
	int r, i;
	uint32_t flags, nr_entries;
	struct jd_block *node = NULL;
	struct btree_node *n;

	r = bn_read_lock(info, root, &node);
	if (r)
		return r;

	n = jd_block_data(node);
	flags = le32_to_cpu(n->header.flags);
	nr_entries = le32_to_cpu(n->header.nr_entries);

	if (flags & INTERNAL_NODE) {
		i = lower_bound(n, key);
		if (i < 0 || i >= nr_entries) {
			r = -ENODATA;
			goto out;
		}

		r = jd_btree_lookup_next_single(info, value64(n, i), key, rkey, value_le);
		if (r == -ENODATA && i < (nr_entries - 1)) {
			i++;
			r = jd_btree_lookup_next_single(info, value64(n, i), key, rkey, value_le);
		}

	} else {
		i = upper_bound(n, key);
		if (i < 0 || i >= nr_entries) {
			r = -ENODATA;
			goto out;
		}

		*rkey = le64_to_cpu(n->keys[i]);
		memcpy(value_le, value_ptr(n, i), info->value_type.size);
	}
out:
	jd_tm_unlock(info->tm, node);
	return r;
}

int jd_btree_lookup_next(struct jd_btree_info *info, jd_block_t root,
			 uint64_t *keys, uint64_t *rkey, void *value_le)
{
	unsigned level;
	int r = -ENODATA;
	__le64 internal_value_le;
	struct ro_spine spine;

	init_ro_spine(&spine, info);
	for (level = 0; level < info->levels - 1u; level++) {
		r = btree_lookup_raw(&spine, root, keys[level],
				     lower_bound, rkey,
				     &internal_value_le, sizeof(uint64_t));
		if (r)
			goto out;

		if (*rkey != keys[level]) {
			r = -ENODATA;
			goto out;
		}

		root = le64_to_cpu(internal_value_le);
	}

	r = jd_btree_lookup_next_single(info, root, keys[level], rkey, value_le);
out:
	exit_ro_spine(&spine);
	return r;
}

/*
 * FIXME: We shouldn't use a recursive algorithm when we have limited stack
 * space.  Also this only works for single level trees.
 */
static int walk_node(struct jd_btree_info *info, jd_block_t block,
		     int (*fn)(void *context, uint64_t *keys, void *leaf),
		     void *context)
{
	int r;
	unsigned i, nr;
	struct jd_block *node = NULL;
	struct btree_node *n;
	uint64_t keys;

	r = bn_read_lock(info, block, &node);
	if (r)
		return r;

	n = jd_block_data(node);

	nr = le32_to_cpu(n->header.nr_entries);
	for (i = 0; i < nr; i++) {
		if (le32_to_cpu(n->header.flags) & INTERNAL_NODE) {
			r = walk_node(info, value64(n, i), fn, context);
			if (r)
				goto out;
		} else {
			keys = le64_to_cpu(*key_ptr(n, i));
			r = fn(context, &keys, value_ptr(n, i));
			if (r)
				goto out;
		}
	}

out:
	jd_tm_unlock(info->tm, node);
	return r;
}

int jd_btree_walk(struct jd_btree_info *info, jd_block_t root,
		  int (*fn)(void *context, uint64_t *keys, void *leaf),
		  void *context)
{
	BUG_ON(info->levels > 1);
	return walk_node(info, root, fn, context);
}
