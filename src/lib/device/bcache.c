/*
 * bcache.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define _GNU_SOURCE

#include "bcache.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <libaio.h>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/user.h>

#define SECTOR_SHIFT 9L

/* assumes the list is not empty. */
static inline struct jd_list *_list_pop(struct jd_list *head)
{
	struct jd_list *l;

	l = head->n;
	jd_list_del(l);
	return l;
}

struct control_block {
	struct jd_list list;
	void *context;
	struct iocb cb;
};

struct cb_set {
	struct jd_list free;
	struct jd_list allocated;
	struct control_block *vec;
} control_block_set;

static struct cb_set *_cb_set_create(unsigned nr)
{
	int i;
	struct cb_set *cbs = malloc(sizeof(*cbs));

	if (!cbs)
		return NULL;

	cbs->vec = malloc(nr * sizeof(*cbs->vec));
	if (!cbs->vec) {
		free(cbs);
		return NULL;
	}

	jd_list_init(&cbs->free);
	jd_list_init(&cbs->allocated);

	for (i = 0; i < nr; i++)
		jd_list_add(&cbs->free, &cbs->vec[i].list);

	return cbs;
}

static void _cb_set_destroy(struct cb_set *cbs)
{
	/* 
         * We know this is always called after a wait_all.  So there should
	 * never be in flight IO.
         */
	if (!jd_list_empty(&cbs->allocated)) {
		/* bail out */
		printf("async io still in flight");
		return;
	}

	free(cbs->vec);
	free(cbs);
}

static struct control_block *_cb_alloc(struct cb_set *cbs, void *context)
{
	struct control_block *cb;

	if (jd_list_empty(&cbs->free))
		return NULL;

	cb = jd_list_item(_list_pop(&cbs->free), struct control_block);
	cb->context = context;
	jd_list_add(&cbs->allocated, &cb->list);

	return cb;
}

static void _cb_free(struct cb_set *cbs, struct control_block *cb)
{
	jd_list_del(&cb->list);
	jd_list_add_h(&cbs->free, &cb->list);
}

static struct control_block *_iocb_to_cb(struct iocb *icb)
{
	return jd_list_struct_base(icb, struct control_block, cb);
}

struct async_engine {
	struct io_engine e;
	io_context_t aio_context;
	struct cb_set *cbs;
	unsigned page_mask;
};

static struct async_engine *_to_async(struct io_engine *e)
{
	return container_of(e, struct async_engine, e);
}

static void _async_destroy(struct io_engine *ioe)
{
	int r;
	struct async_engine *e = _to_async(ioe);

	_cb_set_destroy(e->cbs);

	/* io_destroy is really slow */
	r = io_destroy(e->aio_context);
	if (r)
		printf("io_destroy");

	free(e);
}

static bool _async_issue(struct io_engine *ioe, enum dir d, int fd,
			 sector_t sb, sector_t se, void *data, void *context)
{
	int r;
	struct iocb *cb_array[1];
	struct control_block *cb;
	struct async_engine *e = _to_async(ioe);

	if (((uintptr_t) data) & e->page_mask) {
		printf("misaligned data buffer");
		return false;
	}

	cb = _cb_alloc(e->cbs, context);
	if (!cb) {
		printf("couldn't allocate control block");
		return false;
	}

	memset(&cb->cb, 0, sizeof(cb->cb));

	cb->cb.aio_fildes = (int) fd;
	cb->cb.u.c.buf = data;
	cb->cb.u.c.offset = sb << SECTOR_SHIFT;
	cb->cb.u.c.nbytes = (se - sb) << SECTOR_SHIFT;
	cb->cb.aio_lio_opcode = (d == DIR_READ) ? IO_CMD_PREAD : IO_CMD_PWRITE;

	cb_array[0] = &cb->cb;
	do {
		r = io_submit(e->aio_context, 1, cb_array);
	} while (r == -EAGAIN);

	if (r < 0) {
		_cb_free(e->cbs, cb);
		return false;
	}

	return true;
}

#define MAX_IO 1024
#define MAX_EVENT 64

static bool _async_wait(struct io_engine *ioe, io_complete_fn fn)
{
	int i, r;
	struct io_event event[MAX_EVENT];
	struct control_block *cb;
	struct async_engine *e = _to_async(ioe);

	memset(&event, 0, sizeof(event));
	do {
		r = io_getevents(e->aio_context, 1, MAX_EVENT, event, NULL);
	} while (r == -EINTR);

	if (r < 0) {
		printf("io_getevents");
		return false;
	}

	for (i = 0; i < r; i++) {
		struct io_event *ev = event + i;

		cb = _iocb_to_cb((struct iocb *) ev->obj);

		if (ev->res == cb->cb.u.c.nbytes)
			fn((void *) cb->context, 0);

		else if ((int) ev->res < 0)
			fn(cb->context, (int) ev->res);

		/* FIXME: dct added this. a short read is ok?! */
		else if (ev->res >= (1 << SECTOR_SHIFT)) {
			/* minimum acceptable read is 1 sector */
			fn((void *) cb->context, 0);

		} else {
			fn(cb->context, -ENODATA);
		}

		_cb_free(e->cbs, cb);
	}

	return true;
}

static unsigned _async_max_io(struct io_engine *e)
{
	return MAX_IO;
}

struct io_engine *create_async_io_engine(void)
{
	int r;
	struct async_engine *e = malloc(sizeof(*e));

	if (!e)
		return NULL;

	e->e.destroy = _async_destroy;
	e->e.issue = _async_issue;
	e->e.wait = _async_wait;
	e->e.max_io = _async_max_io;

	e->aio_context = 0;
	r = io_setup(MAX_IO, &e->aio_context);
	if (r < 0) {
		printf("io_setup failed");
		free(e);
		return NULL;
	}

	e->cbs = _cb_set_create(MAX_IO);
	if (!e->cbs) {
		printf("couldn't create control block set");
		free(e);
		return NULL;
	}

	e->page_mask = sysconf(_SC_PAGESIZE) - 1;

	return &e->e;
}

struct sync_io {
        struct jd_list list;
	void *context;
};

struct sync_engine {
	struct io_engine e;
	struct jd_list complete;
};

static struct sync_engine *_to_sync(struct io_engine *e)
{
        return container_of(e, struct sync_engine, e);
}

static void _sync_destroy(struct io_engine *ioe)
{
        struct sync_engine *e = _to_sync(ioe);
        free(e);
}

static bool _sync_issue(struct io_engine *ioe, enum dir d, int fd,
                        sector_t sb, sector_t se, void *data, void *context)
{
        int r;
        uint64_t len = (se - sb) * 512, where;
	struct sync_engine *e = _to_sync(ioe);
	struct sync_io *io = malloc(sizeof(*io));
	if (!io) {
		printf("unable to allocate sync_io");
        	return false;
	}

	where = sb * 512;
	r = lseek(fd, where, SEEK_SET);
	if (r < 0) {
        	printf("unable to seek to position %llu", (unsigned long long) where);
        	free(io);
        	return false;
	}

	while (len) {
        	do {
                	if (d == DIR_READ)
                                r = read(fd, data, len);
                        else {
                                printf("core write.\n");
                                r = write(fd, data, len);
                        }

        	} while ((r < 0) && ((r == EINTR) || (r == EAGAIN)));

        	if (r < 0) {
                	printf("io failed %d", r);
                	free(io);
                	return false;
        	}

                len -= r;
	}

	if (len) {
        	printf("short io %u bytes remaining", (unsigned) len);
        	free(io);
        	return false;
	}

	jd_list_add(&e->complete, &io->list);
	io->context = context;

	return true;
}

static bool _sync_wait(struct io_engine *ioe, io_complete_fn fn)
{
        struct sync_io *io, *tmp;
	struct sync_engine *e = _to_sync(ioe);

	jd_list_iterate_items_safe(io, tmp, &e->complete) {
		fn(io->context, 0);
		jd_list_del(&io->list);
		free(io);
	}

	return true;
}

static unsigned _sync_max_io(struct io_engine *e)
{
        return 1;
}

struct io_engine *create_sync_io_engine(void)
{
	struct sync_engine *e = malloc(sizeof(*e));

	if (!e)
        	return NULL;

        e->e.destroy = _sync_destroy;
        e->e.issue = _sync_issue;
        e->e.wait = _sync_wait;
        e->e.max_io = _sync_max_io;

        jd_list_init(&e->complete);
        return &e->e;
}

#define MIN_BLOCKS 16
#define WRITEBACK_LOW_THRESHOLD_PERCENT 33
#define WRITEBACK_HIGH_THRESHOLD_PERCENT 66

static void *_alloc_aligned(size_t len, size_t alignment)
{
	void *result = NULL;
	int r = posix_memalign(&result, alignment, len);
	if (r)
		return NULL;

	return result;
}

static bool _test_flags(struct block *b, unsigned bits)
{
	return (b->flags & bits) != 0;
}

static void _set_flags(struct block *b, unsigned bits)
{
	b->flags |= bits;
}

static void _clear_flags(struct block *b, unsigned bits)
{
	b->flags &= ~bits;
}

enum block_flags {
	BF_IO_PENDING = (1 << 0),
	BF_DIRTY = (1 << 1),
};

struct bcache {
	sector_t block_sectors;
	uint64_t nr_data_blocks;
	uint64_t nr_cache_blocks;
	unsigned max_io;

	struct io_engine *engine;

	void *raw_data;
	struct block *raw_blocks;

	/*
	 * Lists that categorise the blocks.
	 */
	unsigned nr_locked;
	unsigned nr_dirty;
	unsigned nr_io_pending;

	struct jd_list free;
	struct jd_list errored;
	struct jd_list dirty;
	struct jd_list clean;
	struct jd_list io_pending;

	/*
	 * Hash table.
	 */
	unsigned nr_buckets;
	unsigned hash_mask;
	struct jd_list *buckets;

	/*
	 * Statistics
	 */
	unsigned read_hits;
	unsigned read_misses;
	unsigned write_zeroes;
	unsigned write_hits;
	unsigned write_misses;
	unsigned prefetches;
};

/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001ULL

static unsigned _hash(struct bcache *cache, int fd, uint64_t i)
{
	uint64_t h = (i << 10) & fd;
	h *= GOLDEN_RATIO_PRIME_64;
	return h & cache->hash_mask;
}

static struct block *_hash_lookup(struct bcache *cache, int fd, uint64_t i)
{
	struct block *b;
	unsigned h = _hash(cache, fd, i);

	jd_list_iterate_items_gen (b, cache->buckets + h, hash)
		if (b->fd == fd && b->index == i)
			return b;

	return NULL;
}

static void _hash_insert(struct block *b)
{
	unsigned h = _hash(b->cache, b->fd, b->index);
	jd_list_add_h(b->cache->buckets + h, &b->hash);
}

static inline void _hash_remove(struct block *b)
{
	jd_list_del(&b->hash);
}

/*
 * Must return a power of 2.
 */
static unsigned _calc_nr_buckets(unsigned nr_blocks)
{
	unsigned r = 8;
	unsigned n = nr_blocks / 4;

	if (n < 8)
		n = 8;

	while (r < n)
		r <<= 1;

	return r;
}

static bool _hash_table_init(struct bcache *cache, unsigned nr_entries)
{
	unsigned i;

	cache->nr_buckets = _calc_nr_buckets(nr_entries);
	cache->hash_mask = cache->nr_buckets - 1;
	cache->buckets = malloc(cache->nr_buckets * sizeof(*cache->buckets));
	if (!cache->buckets)
		return false;

	for (i = 0; i < cache->nr_buckets; i++)
		jd_list_init(cache->buckets + i);

	return true;
}

static void _hash_table_exit(struct bcache *cache)
{
	free(cache->buckets);
}

static bool _init_free_list(struct bcache *cache, unsigned count, unsigned pgsize)
{
	unsigned i;
	size_t block_size = cache->block_sectors << SECTOR_SHIFT;
	unsigned char *data =
		(unsigned char *) _alloc_aligned(count * block_size, pgsize);

	/* allocate the data for each block.  We page align the data. */
	if (!data)
		return false;

	cache->raw_blocks = malloc(count * sizeof(*cache->raw_blocks));
	if (!cache->raw_blocks) {
		free(data);
		return false;
	}

	cache->raw_data = data;

	for (i = 0; i < count; i++) {
		struct block *b = cache->raw_blocks + i;
		b->cache = cache;
		b->data = data + (block_size * i);
		jd_list_add(&cache->free, &b->list);
	}

	return true;
}

static void _exit_free_list(struct bcache *cache)
{
	free(cache->raw_data);
	free(cache->raw_blocks);
}

static struct block *_alloc_block(struct bcache *cache)
{
	if (jd_list_empty(&cache->free))
		return NULL;

	return jd_list_struct_base(_list_pop(&cache->free), struct block, list);
}

/*
 * Clean/dirty list management.
 * Always use these methods to ensure nr_dirty_ is correct.
 */
static void _unlink_block(struct block *b)
{
	if (_test_flags(b, BF_DIRTY))
		b->cache->nr_dirty--;

	jd_list_del(&b->list);
}

static void _link_block(struct block *b)
{
	struct bcache *cache = b->cache;

	if (_test_flags(b, BF_DIRTY)) {
		jd_list_add(&cache->dirty, &b->list);
		cache->nr_dirty++;
	} else
		jd_list_add(&cache->clean, &b->list);
}

static void _relink(struct block *b)
{
	_unlink_block(b);
	_link_block(b);
}

/*
 * Low level IO handling
 *
 * We cannot have two concurrent writes on the same block.
 * eg, background writeback, put with dirty, flush?
 *
 * To avoid this we introduce some restrictions:
 *
 * i)  A held block can never be written back.
 * ii) You cannot get a block until writeback has completed.
 */

static void _complete_io(void *context, int err)
{
	struct block *b = context;
	struct bcache *cache = b->cache;

	b->error = err;
	_clear_flags(b, BF_IO_PENDING);
	cache->nr_io_pending--;

	/*
	 * b is on the io_pending list, so we don't want to use unlink_block.
	 * Which would incorrectly adjust nr_dirty.
	 */
	jd_list_del(&b->list);

	if (b->error) {
		jd_list_add(&cache->errored, &b->list);

	} else {
		_clear_flags(b, BF_DIRTY);
		_link_block(b);
	}
}

/*
 * |b->list| should be valid (either pointing to itself, on one of the other
 * lists.
 */
static void _issue_low_level(struct block *b, enum dir d)
{
	struct bcache *cache = b->cache;
	sector_t sb = b->index * cache->block_sectors;
	sector_t se = sb + cache->block_sectors;

	if (_test_flags(b, BF_IO_PENDING))
		return;

	b->io_dir = d;
	_set_flags(b, BF_IO_PENDING);
	cache->nr_io_pending++;

	jd_list_move(&cache->io_pending, &b->list);

	if (!cache->engine->issue(cache->engine, d, b->fd, sb, se, b->data, b)) {
		/* FIXME: if io_submit() set an errno, return that instead of EIO? */
		_complete_io(b, -EIO);
		return;
	}
}

static inline void _issue_read(struct block *b)
{
	_issue_low_level(b, DIR_READ);
}

static inline void _issue_write(struct block *b)
{
	_issue_low_level(b, DIR_WRITE);
}

static bool _wait_io(struct bcache *cache)
{
	return cache->engine->wait(cache->engine, _complete_io);
}

/*
 * High level IO handling
 */
static void _wait_all(struct bcache *cache)
{
	while (!jd_list_empty(&cache->io_pending))
		_wait_io(cache);
}

static void _wait_specific(struct block *b)
{
	while (_test_flags(b, BF_IO_PENDING))
		_wait_io(b->cache);
}

static unsigned _writeback(struct bcache *cache, unsigned count)
{
	unsigned actual = 0;
	struct block *b, *tmp;

	jd_list_iterate_items_gen_safe (b, tmp, &cache->dirty, list) {
		if (actual == count)
			break;

		/* We can't writeback anything that's still in use. */
		if (!b->ref_count) {
			_issue_write(b);
			actual++;
		}
	}

	return actual;
}

/*
 * High level allocation
 */
static struct block *_find_unused_clean_block(struct bcache *cache)
{
	struct block *b;

	jd_list_iterate_items (b, &cache->clean) {
		if (!b->ref_count) {
			_unlink_block(b);
			_hash_remove(b);
			return b;
		}
	}

	return NULL;
}

static struct block *_new_block(struct bcache *cache, int fd, block_address i, bool can_wait)
{
	struct block *b;

	b = _alloc_block(cache);
	while (!b && !jd_list_empty(&cache->clean)) {
		b = _find_unused_clean_block(cache);
		if (!b) {
			if (can_wait) {
				if (jd_list_empty(&cache->io_pending))
					_writeback(cache, 16);  /* FIXME: magic number */
				_wait_io(cache);
			} else {
				printf("bcache no new blocks for fd %d index %u",
					  fd, (uint32_t) i);
				return NULL;
			}
		}
	}

	if (b) {
		jd_list_init(&b->list);
		jd_list_init(&b->hash);
		b->flags = 0;
		b->fd = fd;
		b->index = i;
		b->ref_count = 0;
		b->error = 0;

		_hash_insert(b);
	}

#if 0
	if (!b) {
		printf("bcache no new blocks for fd %d index %u "
			  "clean %u free %u dirty %u pending %u nr_data_blocks %u nr_cache_blocks %u",
			  fd, (uint32_t) i,
			  jd_list_size(&cache->clean),
			  jd_list_size(&cache->free),
			  jd_list_size(&cache->dirty),
			  jd_list_size(&cache->io_pending),
			  (uint32_t)cache->nr_data_blocks,
			  (uint32_t)cache->nr_cache_blocks);
	}
#endif

	return b;
}

/*
 * Block reference counting
 */
static void _zero_block(struct block *b)
{
	b->cache->write_zeroes++;
	memset(b->data, 0, b->cache->block_sectors << SECTOR_SHIFT);
	_set_flags(b, BF_DIRTY);
}

static void _hit(struct block *b, unsigned flags)
{
	struct bcache *cache = b->cache;

	if (flags & (GF_ZERO | GF_DIRTY))
		cache->write_hits++;
	else
		cache->read_hits++;

	_relink(b);
}

static void _miss(struct bcache *cache, unsigned flags)
{
	if (flags & (GF_ZERO | GF_DIRTY))
		cache->write_misses++;
	else
		cache->read_misses++;
}

static struct block *_lookup_or_read_block(struct bcache *cache,
				  	   int fd, block_address i,
					   unsigned flags)
{
	struct block *b = _hash_lookup(cache, fd, i);

	if (b) {
		/* 
                 * FIXME: this is insufficient.  We need to also catch a read
		 * lock of a write locked block.  Ref count needs to distinguish.
                 */
		if (b->ref_count && (flags & (GF_DIRTY | GF_ZERO))) {
			printf("concurrent write lock attempted");
			return NULL;
		}

		if (_test_flags(b, BF_IO_PENDING)) {
			_miss(cache, flags);
			_wait_specific(b);

		} else
			_hit(b, flags);

		_unlink_block(b);

		if (flags & GF_ZERO)
			_zero_block(b);

	} else {
		_miss(cache, flags);

		b = _new_block(cache, fd, i, true);
		if (b) {
			if (flags & GF_ZERO)
				_zero_block(b);

			else {
				_issue_read(b);
				_wait_specific(b);

				/* we know the block is clean and unerrored. */
				_unlink_block(b);
			}
		}
	}

	if (b) {
		if (flags & (GF_DIRTY | GF_ZERO))
			_set_flags(b, BF_DIRTY);

		_link_block(b);
		return b;
	}

	return NULL;
}

static void _preemptive_writeback(struct bcache *cache)
{
	/* 
         * FIXME: this ignores those blocks that are in the error state.  Track
	 * nr_clean instead?
         */
	unsigned nr_available = cache->nr_cache_blocks - (cache->nr_dirty - cache->nr_io_pending);
        /*printf("nr_available=%u (WRITEBACK_LOW_THRESHOLD_PERCENT * cache->nr_cache_blocks / 100)=%u\n", \
               nr_available, (WRITEBACK_LOW_THRESHOLD_PERCENT * cache->nr_cache_blocks / 100));
        */
        //if (nr_available < (WRITEBACK_LOW_THRESHOLD_PERCENT * cache->nr_cache_blocks / 100)) {
		_writeback(cache, (WRITEBACK_HIGH_THRESHOLD_PERCENT * cache->nr_cache_blocks / 100) - nr_available);
        //}

}

/*
 * Public interface
 */
struct bcache *bcache_create(sector_t block_sectors, unsigned nr_cache_blocks,
			     struct io_engine *engine)
{
	struct bcache *cache;
	unsigned max_io = engine->max_io(engine);
	long pgsize = sysconf(_SC_PAGESIZE);

	if (!nr_cache_blocks) {
		printf("bcache must have at least one cache block");
		return NULL;
	}

	if (!block_sectors) {
		printf("bcache must have a non zero block size");
		return NULL;
	}

	if (block_sectors & ((pgsize >> SECTOR_SHIFT) - 1)) {
		printf("bcache block size must be a multiple of page size");
		return NULL;
	}

	cache = malloc(sizeof(*cache));
	if (!cache)
		return NULL;

	cache->block_sectors = block_sectors;
	cache->nr_cache_blocks = nr_cache_blocks;
	cache->max_io = nr_cache_blocks < max_io ? nr_cache_blocks : max_io;
	cache->engine = engine;
	cache->nr_locked = 0;
	cache->nr_dirty = 0;
	cache->nr_io_pending = 0;

	jd_list_init(&cache->free);
	jd_list_init(&cache->errored);
	jd_list_init(&cache->dirty);
	jd_list_init(&cache->clean);
	jd_list_init(&cache->io_pending);

	if (!_hash_table_init(cache, nr_cache_blocks)) {
		cache->engine->destroy(cache->engine);
		free(cache);
		return NULL;
	}

	cache->read_hits = 0;
	cache->read_misses = 0;
	cache->write_zeroes = 0;
	cache->write_hits = 0;
	cache->write_misses = 0;
	cache->prefetches = 0;

	if (!_init_free_list(cache, nr_cache_blocks, pgsize)) {
		cache->engine->destroy(cache->engine);
		_hash_table_exit(cache);
		free(cache);
		return NULL;
	}

	return cache;
}

void bcache_destroy(struct bcache *cache)
{
	if (cache->nr_locked)
		printf("some blocks are still locked");

	bcache_flush(cache);
	_wait_all(cache);
	_exit_free_list(cache);
	_hash_table_exit(cache);
	cache->engine->destroy(cache->engine);
	free(cache);
}

sector_t bcache_block_sectors(struct bcache *cache)
{
	return cache->block_sectors;
}

unsigned bcache_nr_cache_blocks(struct bcache *cache)
{
	return cache->nr_cache_blocks;
}

unsigned bcache_max_prefetches(struct bcache *cache)
{
	return cache->max_io;
}

void bcache_prefetch(struct bcache *cache, int fd, block_address i)
{
	struct block *b = _hash_lookup(cache, fd, i);

	if (!b) {
		if (cache->nr_io_pending < cache->max_io) {
			b = _new_block(cache, fd, i, false);
			if (b) {
				cache->prefetches++;
				_issue_read(b);
			}
		}
	}
}

static void _recycle_block(struct bcache *cache, struct block *b)
{
	_unlink_block(b);
	_hash_remove(b);
	jd_list_add(&cache->free, &b->list);
}

bool bcache_get(struct bcache *cache, int fd, block_address i,
	        unsigned flags, struct block **result)
{
	struct block *b;

	b = _lookup_or_read_block(cache, fd, i, flags);
	if (b) {
		if (b->error) {
			if (b->io_dir == DIR_READ) {
				/* 
                                 * Now we know the read failed we can just forget
				 * about this block, since there's no dirty data to
				 * be written back.
                                 */
				_recycle_block(cache, b);
			}
			return false;
		}

		if (!b->ref_count)
			cache->nr_locked++;
		b->ref_count++;

		*result = b;
		return true;
	}

	*result = NULL;

	printf("bcache failed to get block %u fd %d", (uint32_t) i, fd);
	return false;
}

static void _put_ref(struct block *b)
{
	if (!b->ref_count) {
		printf("ref count on bcache block already zero");
		return;
	}

	b->ref_count--;
	if (!b->ref_count)
		b->cache->nr_locked--;
}

void bcache_put(struct block *b)
{
	_put_ref(b);

        if (_test_flags(b, BF_DIRTY))
		_preemptive_writeback(b->cache);
}

bool bcache_flush(struct bcache *cache)
{
	/* 
         * Only dirty data is on the errored list, since bad read blocks get
	 * recycled straight away.  So we put these back on the dirty list, and
	 * try and rewrite everything.
         */
	jd_list_splice(&cache->dirty, &cache->errored);
        printf("7777\n");

	while (!jd_list_empty(&cache->dirty)) {
		struct block *b = jd_list_item(_list_pop(&cache->dirty), struct block);
		if (b->ref_count || _test_flags(b, BF_IO_PENDING)) {
			// The superblock may well be still locked.
			continue;
		}

		_issue_write(b);
	}

	_wait_all(cache);

	return jd_list_empty(&cache->errored);
}

/*
 * You can safely call this with a NULL block.
 */
static bool _invalidate_block(struct bcache *cache, struct block *b)
{
	if (!b)
		return true;

	if (_test_flags(b, BF_IO_PENDING))
		_wait_specific(b);

	if (b->ref_count) {
		printf("bcache_invalidate: block (%d, %llu) still held",
			 b->fd, (unsigned long long) b->index);
		return false;
	}

	if (_test_flags(b, BF_DIRTY)) {
		_issue_write(b);
		_wait_specific(b);

		if (b->error)
        		return false;
	}

	_recycle_block(cache, b);

	return true;
}

bool bcache_invalidate(struct bcache *cache, int fd, block_address i)
{
	return _invalidate_block(cache, _hash_lookup(cache, fd, i));
}

/* 
 * FIXME: switch to a trie, or maybe 1 hash table per fd?  To save iterating
 * through the whole cache.
 */
bool bcache_invalidate_fd(struct bcache *cache, int fd)
{
	struct block *b, *tmp;
	bool r = true;

	/* start writing back any dirty blocks on this fd. */
	jd_list_iterate_items_safe (b, tmp, &cache->dirty)
		if (b->fd == fd)
			_issue_write(b);

	_wait_all(cache);

	/* Everything should be in the clean list now. */
	jd_list_iterate_items_safe (b, tmp, &cache->clean)
		if (b->fd == fd)
			r = _invalidate_block(cache, b) && r;

       return r;
}
